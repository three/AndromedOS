/*
 * terminal.c
 */
#include "terminal.h"
#include "memory.h"

#include <stdint.h>

#define TERM_VIDMEM_START (void *)0xB8000
#define TERM_VIDMEM_END   (void *)0xB8FA0
#define TERM_VIDMEM_SIZE          0x00FA0

#define TERM_MSAFE_BARRIER 16
#define TERM_MSAFE_ENDBARRIER 17
#define TERM_MSAFE_COLORMASK 0x0F00

void terminal_simplewrite(char *str, int pos)
{
    uint16_t * mpos = TERM_VIDMEM_START + pos;
    int spos = 0;
    while ( str[spos] )
    {
        if ( mpos > (uint16_t *)TERM_VIDMEM_END )
            mpos = TERM_VIDMEM_START;
        *mpos = TERM_MSAFE_COLORMASK | str[spos];
        spos++; mpos++;
    }
}

uint16_t *terminal_maskedwrite(char *str, uint16_t mask, uint16_t *pos)
{
    int offset = 0;
    uint16_t * const e = TERM_VIDMEM_END;

    while ( str[offset] )
    {
        while ( pos >= e )
            pos = TERM_VIDMEM_START;
        *pos = mask | str[offset];
        pos++; offset++;
    }

    if ( pos >= e )
        return TERM_VIDMEM_START;
    else
        return pos;
}

void *terminal_findchar(char character)
{
    uint16_t *c = TERM_VIDMEM_START;
    uint16_t *e = TERM_VIDMEM_END;
    while ( c < e )
    {
        if ( (char)(*c&0xFF) == character )
        {
            return c;
        }
        c++;
    }
    return 0;
}

void static *terminal_strcpy( char *from, char *to ) {
    // Private, other modules should use memory_strcpy instead
    while ( *from ) {
        *to = *from;
        to++; from++;
    }
    return from;
}

void terminal_init()
{
    // Clear the Screen
    volatile uint16_t *c = TERM_VIDMEM_START;
    volatile uint16_t *e = TERM_VIDMEM_END;
    while (c < e)
    {
        *c = TERM_MSAFE_COLORMASK | 0;
        c++;
    }

    // Make first character the end barrier
    *(uint16_t *)TERM_VIDMEM_START =
        TERM_MSAFE_COLORMASK | TERM_MSAFE_ENDBARRIER;
}

int terminal_log(char *message)
{
    // Try to find the last barrier
    uint16_t *pos = terminal_findchar( TERM_MSAFE_ENDBARRIER );
    if (!pos) {
        // Tell user there's an error and return 1
        terminal_simplewrite("Error writing message as memsafe!", 0);
        return 1;
    }

    // Replace the endbarrier with a regular barrier
    *pos = TERM_MSAFE_COLORMASK | TERM_MSAFE_BARRIER;
    pos++; // Move up a character

    // Write new message
   pos = terminal_maskedwrite(message, TERM_MSAFE_COLORMASK, pos);

   // Put Endbarrier after
   *pos = TERM_MSAFE_COLORMASK | TERM_MSAFE_ENDBARRIER;

   return 0;
}

