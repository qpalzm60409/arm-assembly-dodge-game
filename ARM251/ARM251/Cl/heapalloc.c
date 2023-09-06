/*
 * heapalloc.c
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.9.4.1 $
 * Checkin $Date: 1998/09/22 09:20:19 $
 * Revising $Author: wdijkstr $
 */

#include <stdlib.h>

#include "heap.h"
#include "rt.h"
#include "interns.h"

#ifdef EMBEDDED_CLIB
#  define heapDescPtr __rt_heapdescriptor()
extern Heap_Descriptor *__rt_heapdescriptor(void);
extern Heap_Descriptor *__rt_embeddedalloc_init(void *base, size_t size);
#else
#  define heapDescPtr &heap
extern Heap_Descriptor heap;
#endif


#if defined alloc_c || defined SHARED_C_LIBRARY

#ifdef EMBEDDED_CLIB

extern Heap_Descriptor *__rt_embeddedalloc_init(void *base, size_t size)
{
    if (size <= sizeof(Heap_Descriptor))
        return NULL;
    else
    {   Heap_Descriptor *hd = (Heap_Descriptor *)base;
        Heap_Initialise(hd, NULL, NULL, NULL, NULL);
        Heap_InitMemory(hd, hd+1, size - sizeof(Heap_Descriptor));
        return hd;
    }
}

#else
Heap_Descriptor heap;
/* <_terminate_user_alloc>
 * User allocation has finished
 */
extern void _terminate_user_alloc(void)
{
}

/* <_init_user_alloc>
 * User allocation is starting
 */
extern void _init_user_alloc(void)
{
}

/* <Heap_Full>
 * Called when the heap gets full - extend heap heap now to service request
 */
static int Heap_Full(void *param, size_t size)
{
    void *base;

    param = param;

    /* Alloc from runtime, request amount in words, word size of gotten block returned */
    size = __rt_alloc(((size+3)&~3)/4, &base) * 4;

    if (size != 0)
    {
        Heap_ProvideMemory(heapDescPtr, base, size);
        return 1;
    }
    return 0;
}

/* <Heap_Broken>
 * Called when the heap gets broken
 */
static void Heap_Broken(void *param)
{
    param = param;
    _sysdie("Heap broken (overwrite detected or self-inconsistent)", "");
}

/* <_init_alloc>
 * Initialise the malloc/free system
 */
extern void _init_alloc(void)
{
    Heap_Initialise(heapDescPtr, Heap_Full, heapDescPtr, Heap_Broken, heapDescPtr);
    __rt_register_allocs(&malloc, &free);
}

#endif  /* EMBEDDED_CLIB */

/* <malloc>
 * Allocate the given number of bytes - return 0 if fails
 */
extern void *malloc(size_t size)
{
    return size != 0 ? Heap_Alloc(heapDescPtr, size)
                     : NULL;
}

#endif

#if defined free_c || defined SHARED_C_LIBRARY

/* <free>
 * Free the given block of bytes
 */
extern void free(void *p)
{
    if (p != NULL)
    {
        Heap_Free(heapDescPtr, p);
    }
}

#endif /* free_c */


#if defined realloc_c || defined SHARED_C_LIBRARY

void *realloc(void *p, size_t size)
{
    return p == NULL ? Heap_Alloc(heapDescPtr, size)
                     : Heap_Realloc(heapDescPtr, p, size);
}
#endif


#if defined calloc_c || defined SHARED_C_LIBRARY

#include <string.h>

/* <calloc>
 * Alloc a zero-filled block count*size bytes big
 */
extern void *calloc(size_t count, size_t size)
{
    void *r;
    /*
     * This code computes the 64 bit product of count * size to 
     * verify that the product really is in range.
     */
    unsigned m1 = (count >> 16) * (size & 0xffff);
    unsigned m2 = (count & 0xffff) * (size >> 16);
    unsigned len = (count & 0xffff) * (size & 0xffff);

    if ((m1 | m2) >> 16)
        return NULL;
    m1 += m2;
    if (m1 + (len >> 16) >= 0x10000)
        return NULL;
    len += m1 << 16;
    
    r = malloc(len);
    if (r != NULL)
        memset(r, 0, len);
    return r;
}
#endif

#if defined stats_c || defined SHARED_C_LIBRARY

extern void __heapstats(int (*dprint)(char const *format, ...))
{
    Heap_Stats(dprint, heapDescPtr);
}

#endif

/* End of file heapalloc.c */
