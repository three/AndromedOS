/*
 * link.ld
 */

OUTPUT_FORMAT(binary)
ENTRY(kentry)
SECTIONS
{
    . = 0x100000;
    .text : { kernel/entry.o(.text) *(.text) }
    .data : { *(.data) }
}
