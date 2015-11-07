/*
 * log.c
 *
 * Handles logging to prevent circular dependencies.
 */

#include "log.h"

#include <stdint.h>

log_logger klog_p;
log_logger kwarn_p;
log_logger kerror_p;

log_logger ktell_p;

void klog(char *m)
{
    (*klog_p)(m);
}

void kwarn(char *m)
{
    (*kwarn_p)(m);
}

void kerror(char *m)
{
    (*kerror_p)(m);
}

void ktell(char *m)
{
    (*ktell_p)(m);
}

void kthrow(char *m)
{
    volatile uint16_t *c = (void *)0xB8000;
    while ( c < (uint16_t *)0xB8FA0 )
        *(c++) = (uint16_t)( *(m++)|0x4F00 );
    asm volatile("1:cli;jmp 1b" : : : "memory");
}

void klog_set(log_logger to)
{
    klog_p = to;
}

void kwarn_set(log_logger to)
{
    kwarn_p = to;
}

void kerror_set(log_logger to)
{
    kerror_p = to;
}

void ktell_set(log_logger to)
{
    ktell_p = to;
}
