
//! This code taken from Portable Formats Specification, Version 1.1

#include "elfdefs.h"

uint32_t elf_hash(const unsigned char *name)
{
    uint32_t h = 0, g;
    while(*name)
    {
        h =(h << 4) + *name++;
        if((g = h & 0xf0000000))
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}
