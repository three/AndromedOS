/*
 * shell.h
 */

#include <stdint.h>
#include "terminal.h"

typedef uint16_t shell_color;

void shell_init();

void shell_log(char *text);
void shell_logbytes(uint8_t *bytes, int length);
void shell_init();
