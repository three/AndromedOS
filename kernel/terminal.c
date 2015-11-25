/*
 * terminal.c
 *
 * Handles IO
 */
#include "terminal.h"
#include "memory.h"
#include "general.h"

#include <stdint.h>

#define TERM_VIDMEM_START ((terminal_char *)0xB8000)
#define TERM_VIDMEM_END   ((terminal_char *)0xB8FA0)
#define TERM_VIDMEM_SIZE                    0x00FA0
#define TERM_VIDMEM_WIDTH   80
#define TERM_VIDMEM_HEIGHT  25

//
// Helper Functions
//

static int getpos(int x, int y)
{
    if ( x>=TERM_VIDMEM_WIDTH || x<0 ||y>=TERM_VIDMEM_HEIGHT || y<0 )
        return -1;
    return TERM_VIDMEM_WIDTH*y + x;
}

//
// Basic terminal API
//

void terminal_init()
{
    for (int p=0;p<TERM_VIDMEM_SIZE;p++)
    {
        TERM_VIDMEM_START[p] = 0xFF00;
    }
}

void terminal_setchar(int x, int y, terminal_char tchar)
{
    int pos = getpos(x,y);
    if (pos<0)
        return;
    TERM_VIDMEM_START[pos] = tchar;
}

uint16_t terminal_getchar(int x, int y)
{
    terminal_char c;
    int pos = getpos(x,y);
    if (pos<0)
        return 0;
    c = TERM_VIDMEM_START[pos];
    return c;
}

void terminal_movecursor(int x, int y)
{
    int pos = getpos(x,y);
    uint8_t low = (uint8_t)(pos&0xFF);
    uint8_t high = (uint8_t)( (pos>>8)&0xFF );
    outb(0x3D4,0x0F);
    outb(0x3D5,low);
    outb(0x3D4,0x0E);
    outb(0x3D5,high);
}

//
// Boxwrite Functions
//  Writes to terminal in a box with properties and cursor position stored in
//  a terminal_box struct
//

//
// Writing Functions
//  When passing strings to these functions the following characters are treated
//  special:
//   0x11 - color will be changed to byte following this
//   0x12 - next 2 bytes will be placed in memory as is
//          (WARNING: this includes 0x0000. If used improperly string may not
//                    terminate!)
//

void terminal_boxwritenewline(terminal_box *box)
{
    box->cx = box->x1;
    box->cy++;
    if ( box->cy > box->y2 )
        box->cy = box->y1;
}

void terminal_boxwriteback(terminal_box *box)
{
    terminal_char c;
    if ( box->cx == box->x1 ) {
        box->cx = box->x2;
        box->cy--;
        if ( box->cy < box->y1 )
            box->cy = box->y2;
    } else {
        box->cx--;
    }

    c = terminal_getchar(box->cx, box->cy);
    c &= 0xFF00;
    c |= (' '<<8);
    terminal_setchar(box->cx, box->cy, c);

    if ( box->hascursor )
        terminal_movecursor(box->cx, box->cy);
}

void terminal_boxwritefullchar(terminal_box *box, terminal_char tchar)
{
    // Check to make sure we're in bounds
    if ( box->cx < box->x1 || box->cx > box->x2 ||
         box->cy < box->y1 || box->cy > box->y2 ) {
        // Out of bounds. Mark in red where problem is.
        terminal_setchar(box->cx,box->cy, 0x4420);
        return;
    }
    // Write the character and incriment pointer
    terminal_setchar(box->cx,box->cy,tchar);
    box->cx++;
    // Carry over if we overrun the bounds
    if ( box->cx > box->x2 )
        terminal_boxwritenewline(box);
    // If necessary, move the cursor
    if (box->hascursor)
        terminal_movecursor(box->cx,box->cy);
}

void terminal_boxwritechar(terminal_box *box, char c)
{
    switch (c) {
        case '\n':
            terminal_boxwritenewline(box);
            break;
        default:
            terminal_boxwritefullchar(box,(terminal_char)(box->color|c));
            break;
    }
}

void terminal_boxwrite(terminal_box *box, char *text)
{
    // The following characters are handled as special:
    //  0x11 - color will be changed to byte following this
    //  0x12 - next byte will be written in exactly as char
    //  0x13 - next 2 bytes will be written in exactly as terminal_char
    //  WARNING: If bytes used following these are 0, string will not be escaped
    while ( *text )
    {
        if ( *text == 0x11 ) {
            text++;
            box->color = (uint16_t)(*text<<8);
            text++;
        } else if ( *text == 0x12 ) {
            text++;
            terminal_boxwritechar(box, *text);
            text++;
        } else if ( *text == 0x13 ) {
            uint8_t low  = *(++text);
            uint8_t high = *(++text);
            terminal_char full = (terminal_char)( (high<<8) & low );
            terminal_boxwritefullchar(box,full);
            text++;
        } else {
            terminal_boxwritechar(box, *text);
            text++;
        }
    }
}

//
// Reading Functions
//  Blocking functions that reads from PS2 Keyboard
//

// Keymap for US QWERTY keyboard
//  Converts output from PS2 device into keys that are more easily translateable
//  into ASCII and ignores keys we don't care about.
//  High bit: 1-up,0-down
//  Lower 7 bits:
//    |+00|+01|+02|+03|+04|+05|+06|+07|+08|+09|+10|+11|+12|+13|+14|+15|
//  ---------------------------------------------------------------
//  00|NUL|LSH|RSH|LCL|RCL|LAT|RAT| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
//  10| 9 | A | B | C | D | E | F | G | H | I | J | K | L | M | N | O |
//  20| P | Q | R | S | T | U | V | W | X | Y | Z |F1 |F2 |F3 |F4 |F5 |
//  30|F6 |F7 |F8 |F9 |F10|F11|F12|ENT|UP |DOW|LEF|RIG|ESC|BAC|SPA|(,)|
//  40|(-)|(.)|(/)|(;)|(=)|(\)|([)|(])|(')|(`)|   |   |   |   |   |   |

static char keymap[] = {
    /* 0x00 - 0x07 */ 0x00, 0xBC, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D,
    /* 0x08 - 0x0F */ 0x8E, 0x8F, 0x90, 0x87, 0xC0, 0xC4, 0xBD, 0x00,
    /* 0x10 - 0x17 */ 0xA1, 0xA7, 0x95, 0xA2, 0xA4, 0xA9, 0xA5, 0x99,
    /* 0x18 - 0x1F */ 0x9F, 0xA0, 0xC6, 0xC7, 0xB7, 0x83, 0x91, 0xA3,
    /* 0x20 - 0x27 */ 0x94, 0x96, 0x97, 0x98, 0x9A, 0x9B, 0x9C, 0xC3,
    /* 0x28 - 0x2F */ 0xC8, 0xC9, 0x81, 0xC5, 0xAA, 0xA8, 0x93, 0xA6,
    /* 0x30 - 0x37 */ 0x92, 0x9E, 0x9D, 0xBF, 0xC1, 0xC2, 0x82, 0x00,
    /* 0x38 - 0x3F */ 0x85, 0xBE, 0x00, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    /* 0x40 - 0x47 */ 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0x00, 0x00, 0x00,
    /* 0x48 - 0x4F */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x50 - 0x57 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB5,
    /* 0x58 - 0x5F */ 0xB6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x60 - 0x67 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x68 - 0x6F */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x70 - 0x77 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x78 - 0x7F */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x80 - 0x87 */ 0x00, 0x3C, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
    /* 0x88 - 0x8F */ 0x0E, 0x0F, 0x10, 0x07, 0x40, 0x44, 0x3D, 0x00,
    /* 0x90 - 0x97 */ 0x21, 0x27, 0x15, 0x22, 0x24, 0x29, 0x25, 0x19,
    /* 0x98 - 0x9F */ 0x1F, 0x20, 0x46, 0x47, 0x37, 0x03, 0x11, 0x23,
    /* 0xA0 - 0xA7 */ 0x14, 0x16, 0x17, 0x18, 0x1A, 0x1B, 0x1C, 0x43,
    /* 0xA8 - 0xAF */ 0x48, 0x49, 0x01, 0x45, 0x2A, 0x28, 0x13, 0x26,
    /* 0xB0 - 0xB7 */ 0x12, 0x1E, 0x1D, 0x3F, 0x41, 0x42, 0x02, 0x00,
    /* 0xB8 - 0xBF */ 0x05, 0x3E, 0x00, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    /* 0xC0 - 0xC7 */ 0x30, 0x31, 0x32, 0x33, 0x34, 0x00, 0x00, 0x00,
    /* 0xC8 - 0xCF */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xD0 - 0xD7 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35,
    /* 0xD8 - 0xDF */ 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xE0 - 0xE7 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xE8 - 0xEF */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xF0 - 0xF7 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0xF8 - 0xFF */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static char numrow[] = { ')', '!',  '@', '#', '$', '%', '^', '&', '*', '(' };
static char syms[] = {
    ',', '<',  '-', '_', '.', '>', '/', '?',  ';', ':', '=',
    '+', '\\', '|', '[', '{', ']', '}', '\'', '"', '`', '~',
};

// Bitmap for storing which key is pressed
static char pressed[16] = { 0 };

static char findASCII(char c)
{
    // Is either LSH or RSH pressed?
    char shift = pressed[0]&0x06;

    // Number Row
    if ( c>=0x07 && c<=0x10 ) {
        if ( shift ) {
            return numrow[ c-7 ];
        }
        else {
            return c+('0'-7);
        }
    }

    // Letters
    if ( c>=0x11 && c<=0x2A ) {
        if ( shift ) {
            return c+('A'-17);
        }
        else {
            return c+('a'-17);
        }
    }

    // Symbols
    if ( c>=0x3F && c<=0x49 ) {
        if ( shift ) {
            return syms[(c-0x3F)*2+1];
        } else {
            return syms[(c-0x3F)*2];
        }
    }

    // Spacebar
    if ( c == 0x3E )
        return ' ';
    // Backspace
    if ( c == 0x3D )
        return '\b';

    // Key doesn't have a corresponding ASCII character
    return 0;
}

static char waitforkey()
{
    // Returns when a key is pressed or released
    uint8_t res;
    uint8_t pbit, ofbyte, ofbit;
    uint8_t mbit, mbyte;
    while (1) {
        res = keymap[inb(0x60)];
        if ( !(res&0x7F) )
            continue;

        pbit   = (res >> 7);
        ofbyte = (res >> 3)&0x0F;
        ofbit  = (res)     &0x07;

        mbyte = pressed[ofbyte];
        mbit  = (mbyte >> ofbit)&0x01;

        if ( mbit == pbit )
            continue;
        else {
            mbyte ^= (0x01 << ofbit);
            pressed[ofbyte] = mbyte;
        }

        return res;
    }
    return 0;
}

char terminal_boxread(terminal_box *box)
{
    while (1) {
        char k = waitforkey();
        char a = findASCII( k&0x7F );
        if ( a && k&0x80 ) {
            if ( a == '\b' )
                terminal_boxwriteback(box);
            else
                terminal_boxwritechar(box,a);
        }
    }
}
