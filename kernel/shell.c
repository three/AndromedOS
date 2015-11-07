/*
 * shell.c
 *
 * Setups interface for user to interact
 */
#include "shell.h"

#include "terminal.h"

#define SHELL_COLOR_BORDER  ((shell_color)0xF000)
#define SHELL_COLOR_LOG     ((shell_color)0xF800)
#define SHELL_COLOR_WARN    ((shell_color)0xE800)
#define SHELL_COLOR_ERROR   ((shell_color)0x4800)

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

static void startprompt()
{
    shell1box.color = SHELL_COLOR_LOG;
    terminal_boxwrite(&shell1box, "\n> ");
}

void shell_log(char *text)
{
    shell2box.color = SHELL_COLOR_LOG;
    terminal_boxwrite(&shell2box, text);
    terminal_boxwritechar(&shell2box, '\n');
}

void shell_logbytes(uint8_t *bytes, int length)
{
    for (int i=0;i<length;i++) {
        uint8_t byte = bytes[i];
        terminal_boxwrite(&shell2box,"  0x");

        uint8_t low  = 0x0F&byte;
        uint8_t high = (0xF0&byte)>>4;
        low+=48;high+=48;
        low  = (low  > 57)?(low  + 7):low;
        high = (high > 57)?(high + 7):high;

        terminal_boxwritechar(&shell2box, high);
        terminal_boxwritechar(&shell2box, low);

        if ( (i%6) == 5 || i+1>=length )
            terminal_boxwritenewline(&shell2box);
    }
}

void shell_init()
{
    drawborder();
    startprompt();

    shell_log("AndromedOS Started Successfully!");
    shell_log("Here is what the integer 0xABCDEF01 looks like in memory:");
    unsigned int t1 = 0xABCDEF01;
    shell_logbytes((unsigned char *)&t1, sizeof(t1));
    shell_log("Here are special terminal-specific characters not in ASCII:");
    shell_log("   \x12\x10\x12\x11\x12\x12");
    shell_log(" \x11\xA3 Many \x11\xC5 Different \x11\xE7 Colors! ");

    while (1) {
        terminal_boxread(&shell1box);
    }
}
