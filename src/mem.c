/**
 * this is a REALLY REALLY bad malloc implementation.
 * don't use this anywhere that matters lol.
 * 
 * this works (i think?) by just storing the pointer to the _end symbol, and allocating memory
 * after that. as this symbol is at the end of the kernel, it should not interfer with anything.
 * however, this also does literally no error checking, so something bad may still happen.
 * ¯\_(ツ)_/¯
 */
#include "types.h"
#include "console.h"

extern unsigned char _end[];
static uint8 *_memPtr = 1 + ((uint8 *)_end);

/**
 * really bad malloc. 
 * try to not use this too much :P
 * 
 * @param len how much memory to allocate
 * @returns pointer to the start of the allocated memory
 */
static inline void *malloc(uint32 len)
{
    // get current pointer
    void *ptr = _memPtr;

    // increment by len
    _memPtr += len;

    // return pointer for use
    return ptr;
}
