/*
 * shell.c
 *
 * Setups interface for user to interact
 */
#include "shell.h"

#include "terminal.h"
#include "log.h"

#define SHELL_COLOR_BORDER  ((shell_color)0xF000)
#define SHELL_COLOR_LOG     ((shell_color)0xF800)
#define SHELL_COLOR_WARN    ((shell_color)0xE800)
#define SHELL_COLOR_ERROR   ((shell_color)0x4800)

//
// Shell Boundries
//

static terminal_box shell1box = {
    .x1 = 1,
    .y1 = 1,
    .x2 = 38,
    .y2 = 23,
    .cx = 1,
    .cy = 1,
    .color = 0,
    .hascursor = 1,
};
static terminal_box shell2box = {
    .x1 = 41,
    .y1 = 1,
    .x2 = 78,
    .y2 = 23,
    .cx = 41,
    .cy = 1,
    .color = 0,
    .hascursor = 0,
};

//
// Static Functions
//

static void drawborder()
{
    int x, y;
    // Draw border around the edges
    for (y=0;y<25;y+=24)
        for (x=1;x<79;x++)
            terminal_setchar(x,y,0xCD|SHELL_COLOR_BORDER);
    for (x=0;x<80;x+=79)
        for (y=1;y<24;y++)
            terminal_setchar(x,y,0xBA|SHELL_COLOR_BORDER);
    terminal_setchar(79, 0,0xBB|SHELL_COLOR_BORDER);
    terminal_setchar(79,24,0xBC|SHELL_COLOR_BORDER);
    terminal_setchar(0, 24,0xC8|SHELL_COLOR_BORDER);
    terminal_setchar(0,  0,0xC9|SHELL_COLOR_BORDER);

    // Draw line down the middle
    x = 40;
    for (y=1;y<24;y++)
        terminal_setchar(x,y,0xBA|SHELL_COLOR_BORDER);
    terminal_setchar(x,0, 0xCB|SHELL_COLOR_BORDER);
    terminal_setchar(x,24,0xCA|SHELL_COLOR_BORDER);
}

static terminal_box *getshellbox(int id)
{
    if ( id == 1 )
        return &shell1box;
    if ( id == 2 )
        return &shell2box;
    kthrow("Could not find box!");
    return 0;
}

//
// Logging Functions
//

void shell_log(char *text)
{
    shell2box.color = SHELL_COLOR_LOG;
    terminal_boxwrite(&shell2box, text);
    terminal_boxwritenewline(&shell2box);
}
void shell_warn(char *text)
{
    shell2box.color = SHELL_COLOR_WARN;
    terminal_boxwrite(&shell2box, text);
    terminal_boxwritenewline(&shell2box);
}
void shell_error(char *text)
{
    shell2box.color = SHELL_COLOR_ERROR;
    terminal_boxwrite(&shell2box, text);
    terminal_boxwritenewline(&shell2box);
}
void shell_tell(char *text)
{
    shell1box.color = SHELL_COLOR_LOG;
    terminal_boxwrite(&shell1box, text);
}

void shell_logbytes(uint8_t *bytes, int length, int shell)
{
    terminal_box *box = getshellbox(shell);
    for (int i=0;i<length;i++) {
        uint8_t byte = bytes[i];
        terminal_boxwrite(box,"  0x");

        uint8_t low  = 0x0F&byte;
        uint8_t high = (0xF0&byte)>>4;
        low+=48;high+=48;
        low  = (low  > 57)?(low  + 7):low;
        high = (high > 57)?(high + 7):high;

        terminal_boxwritechar(box, high);
        terminal_boxwritechar(box, low);

        if ( (i%6) == 5 || i+1>=length )
            terminal_boxwritenewline(box);
    }
}

//
// Prompt
//

void shell_start()
{
    shell1box.color = SHELL_COLOR_LOG;
    terminal_boxwrite(&shell1box, "\n> ");
    terminal_boxread(&shell1box);
}

//
// Initialization
//

void shell_init()
{
    drawborder();

    klog_set(&shell_log);
    kwarn_set(&shell_warn);
    kerror_set(&shell_error);
    ktell_set(&shell_tell);
}
