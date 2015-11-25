/*
 * terminal.h
 */
#pragma once

#include <stdint.h>

typedef uint8_t terminal_color;
typedef volatile uint16_t terminal_char;

typedef struct {
    int x1; // Top left corner
    int y1;
    int x2; // Bottom right corner
    int y2;
    int cx; // Current cursor position
    int cy; //  Will be altered by write functions
    uint16_t color;
    char hascursor;
} terminal_box;

void terminal_init();

void terminal_setchar(int x, int y, terminal_char c);
uint16_t terminal_getchar(int x, int y);
void terminal_movecursor(int x, int y);

void terminal_boxwritenewline(terminal_box *box);
void terminal_boxwritefullchar(terminal_box *box, terminal_char tchar);
void terminal_boxwriteback(terminal_box *box);
void terminal_boxwritechar(terminal_box *box, char c);
void terminal_boxwrite(terminal_box *box, char *text);

char terminal_boxread(terminal_box *box);
