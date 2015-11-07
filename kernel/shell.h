/*
 * shell.h
 */
#pragma once

#include <stdint.h>
#include "terminal.h"

typedef uint16_t shell_color;

void shell_log(char *text);
void shell_warn(char *text);
void shell_error(char *text);
void shell_tell(char *text);
void shell_logbytes(uint8_t *bytes, int length, int shell);

void shell_start();

void shell_init();
