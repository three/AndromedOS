/*
 * general.h
 */

#include <stdint.h>

typedef long size_t;

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

size_t strlen(char *str);
