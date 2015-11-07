/*
 * main.c
 */

#include "terminal.h"
#include "memory.h"
#include "shell.h"

void kprint(char * str);
void clearscreen(void);

volatile unsigned short * const vid = (void *)0xB8000;
volatile unsigned short * const vidmax = (void *)0xB87D0;

void kmain(void)
{
    terminal_init();
    memory_init();
    shell_init();
}
