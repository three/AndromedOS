/*
 * general.c
 */
#include "general.h"

#include <stdint.h>

void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port) );
}

uint8_t inb(uint16_t port)
{
    uint8_t r;
    asm volatile("inb %1, %0" : "=a"(r) : "Nd"(port) );
    return r;
}

// Standard Functions
//  (behavior should match that of standard library)

size_t strlen(char *str)
{
    char *end = str;
    while ( *end )
        end++;
    return end-str;
}
