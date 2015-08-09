# Memory

AndromedOS is a relatively simple Operating System that does not require a
complex way to allocate and manage memory. The goals of this memory allocation
system is to be as simple as possible while still meeting the needs of the
project.

## Static Memory Layout

The contents of the memory up to the kernel is determined in advanced for what
will be in it. This includes some basic information on how memory will be
allocated dynamically. The memory allocated by `kmalloc` will be in higher
memory, outside this area.

    0x000000    Interrupt Descriptor Table
    0x010000    Dynamic Memory Linked List Start
    0x030000    Memory Map
    0x040000    Kernel Stack
    0x080000    Unusable
    0x100000    Kernel
    0x130000    Dynamically Allocated Memory (limited by memory map)
    0x80000000  End of useable memory by kernel (even if memory map says OK)

## Dynamic Memory Allocation

The `kernel/memory.c` file handles dynamically allocated memory. The dynamic
memory allocation is primarily composed of 2 function: `kmalloc` and `kfree`
which mirror `malloc` and `free` from the C standard library.

### Blocks

Available memory is stored using a linked list of blocks. Blocks are regions of
memory usable by the operating system (ie they appear as a type 1 usable region
by the memory map). Blocks start with a linked list entry giving the addresses
of the next and previous block, along with the length of the current block. When
there is no longer space in blocks suitable for the desired application, a new
block is created based on the next contiguous memory area given by the memory
map.

In order to make blocks simple to work with they should have the following
properties:

  * Blocks should never overlap
  * A Block should never point to a block earlier in memory as its next block
  * A Block should never point to a block later in memory as its previous block
  * Non-first blocks should be as large as possible without including unable
    memory
  * The first block should be of length 0 and stored in a known location (See
    *Static Memory Layout* for specific location).

In order to accomplish these properties, each new block should be created the
same way: by finding the next contiguous region of memory not currently
allocated by a block using the memory map. This may mean multiple blocks must be
created on a single allocation because the first created wasn't suited for the
current allocation.

Blocks are split into boxes, depending on how the memory should be allocated.
See *Boxes*.

### Boxes

The allocator stores memory in 3 different ways:

  * 4-byte buckets
  * 256-byte buckets
  * large allocation

Depending on the type of allocation, different types of boxes will be used.
4-byte and 256-byte bucket boxes allocate memory in 4 or 256 byte regions of
memory called buckets, with which are allocated stored using a bitmap before the
region of buckets. Large allocations, if present, must always be allocated and
represent a portion of memory larger than 256 bytes.

### Bucket Boxes

This system is designed with the assumption that few memory allocations are
going to be over 256 bytes, and most will be 4 bytes (the size of pointers and
integers). For space and ease of use buckets are used to handle allocations less
than 256 bytes.

Essentially, the bucket box starts with a bitmap representing each bucket in the
box's allocation status (allocated or not allocated), followed be an array of
buckets. This system is much more space efficient and less time consuming than
storing the position and size of each allocation.

### Large Boxes

### Deciding what Box type to use

### Overview of Allocation Process

**Setting up**

  1. Fill first linked list entry block
  2. Organize Memory Map so it is non-overlapping and consecutive

**Allocating**

  1. Verify Size
  2. Walk linked list until block is found. For each block, check if suitable
     by...
     1. If using buckets, is the a bucket box matching the correct size? If
        so...
        1. Find first available bucket using allocation bitmap.
        2. Set bitmap as alloacted
        3. Return address
    2. If using buckets, if no bucket box matching correct size...
        1. If there is room to create one...
            1. Reserve space for bucket box
            2. Zero bitmap, except first bit
            3. Return first bucket
    3. If a large allocation, if there is room...
        1. Reserve a large allocation space
        3. Return address of allocation
  3. If no suitable block found...
    1. Use memory map to find next continuous chunk of memory
    2. Create linked list entry at start of block
    3. Set next/prev addresses and size correctly
    4. Start process over

**Freeing**

### Notes on Alignment

In the standard C library `malloc(size)` is guaranteed to give a pointer aligned
on the nearest 2^n higher than size. When calling `kmalloc` in AndromedOS, due
to the bucket system, any allocation less than or equal to 256 bytes will follow
this rule. Allocations above 256 bytes are guaranteed to be aligned on 256 bytes
(to prevent the awkward situation of a larger allocation being guaranteed a lower
alignment), but not more.

In the future, additional functions may be necessary to retrieve properly
aligned pointers.

# Additional Note on this Document

This document represents what the implementation **should** look like, **not**
what it does look like. At the time this is being written, the implementation of
the system has not been started. Changes may be made to the implementation while
the implementation is being made, based on the challenges faced writing it. This
document is only to act as a guideline to organize my thoughts and remind my
future self what I was thinking.
