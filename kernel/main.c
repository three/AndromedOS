/*
 * main.c
 */

#include "terminal.h"
#include "memory.h"
#include "shell.h"
#include "log.h"

void kmain(void)
{
    terminal_init();
    shell_init();
    memory_init();

    ktell("AndromedOS loaded successfully!");
    shell_start();
}
