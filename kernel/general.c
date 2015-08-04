/*
 * general.c
 */
#include "general.h"

#include <stdint.h>

uint16_t general_base16ify(uint8_t dbyte)
{
    // Converts dbyte into two bytes of ASCII base16
    uint8_t c1, c2;
    c1 = (dbyte & 0x0F);
    c2 = (dbyte & 0xF0) >> 4;
    c1 += 48; c2 += 48;

    if (c1 > 57) {
        c1 += 7;
    }
    if (c2 > 57) {
        c2 += 7;
    }

    return (c1 << 8)|c2;
}

// Standard Functions
//  (behavior should match that of standard library)

typedef long size_t;

size_t strlen(char *str)
{
    char *end = str;
    while ( *end )
        end++;
    return end-str;
}
