/* -*-C-*-
 *
 * $Revision: 1.4.2.2 $
 *   $Author: rivimey $
 *     $Date: 1998/10/19 12:22:03 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: stack and memory variables and dynamic relocation of stacks
 */

#include "stacks.h"

unsigned Angel_TopOfMemory = Angel_DefaultTopOfMemory;

/*
 * Angel StackBase is the lowest address used by Angel for it's internal
 * stacks. In the standard configuration, the SVC stack is here, and the USR
 * stack (the AngelStack) is at the top of the region.
 *
 * The total size of the stacks required by Angel is set in stacks.h as
 * Angel_CombinedAngelStackSize, and Angel_StackBaseOffsetFromTopMemory is
 * defined in the port devconf.h as the negation of this ammount.
 */

#if Angel_StacksAreRelativeToTopOfMemory
unsigned Angel_StackBase = Angel_DefaultTopOfMemory + Angel_StackBaseOffsetFromTopMemory;
#else
unsigned Angel_StackBase = Angel_FixedStackBase;
#endif



/*
 * This structure is what is returned when the SWI GETHEAPDESC
 * is made; it returns information about the layout of memory
 * to a C library subsystem expecting the program ... heap .. stack
 * layout.
 *
 * The values set here are also used to initialise the application
 * svc and usr mode stack pointers when an InitialiseApplication call
 * is made, as the application must have usable stacks on startup.
 * see debugos.c:Angel_InitialiseApplication for more details.
 *
 *       ---------- <- top of memory (FFFFFFFF)
 *       |        |
 *       ~        ~
 *       ~        ~
 *       |--------| 
 *       |        |
 *       |        | <- ( Angel stacks, if Angel stacks *are* at top of mem )
 *       |        |
 *       |--------| <- stacktop (== memtop if Angel stacks *not* at top of mem)
 *       |        |
 *       |        |
 *       |--------| <- \ 
 *       |--------| <- / heaplimit = stacklimit
 *       |        |
 *       |        |
 *       |        |
 *       |        |
 *       |--------| <- heapbase
 *       ~        ~
 *       ~        ~
 *       |        |
 *       ---------- <- bottom of memory (0)
 *
 */

#if MINIMAL_ANGEL == 0
AngelHeapStackDesc angel_heapstackdesc =
{
    /* heapbase; */
#ifdef Angel_ApplHeap
    (unsigned *)Angel_ApplHeap,
#else
    0,
#endif

    /* heaplimit; */
#ifdef Angel_ApplHeapLimit
    (unsigned *)Angel_ApplHeapLimit,
#else
    /* Top of heap = bottom of stack */
    (unsigned *)(Angel_DefaultTopOfMemory + Angel_ApplStackLimitOffset),
#endif

    /* stacktop; */
    (unsigned *)(Angel_DefaultTopOfMemory + Angel_ApplStackOffset),
    /* stacklimit; */
    (unsigned *)(Angel_DefaultTopOfMemory + Angel_ApplStackLimitOffset)
};
#endif /* MINMAL_ANGEL */



#if MEMORY_SIZE_MAY_CHANGE_DYNAMICALLY

/*
 *        Function:  angel_RelocateWRTTopOfMemory
 *
 *         Purpose:  Given a new top-of-memory value (probaly calculated by
 *                   angel_FindTopOfMemory), modify the global variables which
 *                   relate to this value. This may include where the Angel
 *                   and the application stacks are located and the location
 *                   and size of the application heap.
 *
 *       Arguments:  memorytop - the new top of memory address (the last byte
 *                               usable).
 *
 *         Returns:  0 if memtop was already ok, 1 otherwise
 *
 *   Pre-conditions: Angel_TopOfMemory must have been set up in advance.
 *
 *  Post-conditions: The application heap descriptor and Angel Stack Base
 *                   are initialised as required.
 */

unsigned 
angel_RelocateWRTTopOfMemory(unsigned int memorytop)
{
    if (memorytop == Angel_TopOfMemory)
        return 0;

    Angel_TopOfMemory = memorytop;
#if Angel_StacksAreRelativeToTopOfMemory
    Angel_StackBase = memorytop + Angel_StackBaseOffsetFromTopMemory;
#endif

#if MINIMAL_ANGEL == 0
#ifndef Angel_ApplHeapLimit
    /* Top of heap = bottom of stack */
    angel_heapstackdesc.heaplimit = (unsigned *)(memorytop + Angel_ApplStackLimitOffset);
#endif

    angel_heapstackdesc.stacklimit = (unsigned *)(memorytop + Angel_ApplStackLimitOffset);
    angel_heapstackdesc.stacktop = (unsigned *)(memorytop + Angel_ApplStackOffset);
#endif /* MINMAL_ANGEL */

    return 1;
}

/*
 *        Function:  angel_FindTopOfMemory
 *
 *         Purpose:  Determine how much additional memory has been added to a (PID)
 *                   board.
 * 
 *                   This code is intended for the PID cards only, but should be fairly
 *                   easily modified for other boards - by changing the values from
 *                   those above
 * 
 *                   The code writes a special value to the start of the area to check
 *                   which can be used to detect wraparound; another value is used to
 *                   check if a memory 'page' is present. If a page is present, the
 *                   next page is checked until the the top of possible DRAM pages is
 *                   reached.
 * 
 *                   All values written to memory are restored.
 *
 *                   NOTE: If this code is used, you must define:
 *
 *                      FINDTOP_FIRST_ADDRESS_TO_CHECK -- where to start checking
 *                      FINDTOP_BLOCK_STEP             -- how often to check
 *                      FINDTOP_DRAM_WRAP_ADDR         -- if memory aliased, where
 *                                                        aliases might occur
 *                      FINDTOP_DRAM_MAX               -- the absolue highest address
 *                                                        which might have memory.
 *
 *                    to values appropriate for the memory layout on your card. See the
 *                    file pid/devconf.h for an example.
 * 
 *       Arguments:  None.
 *
 *         Returns:  The new value of memtop.
 *
 *   Pre-conditions: None
 *
 *  Post-conditions: None.
 */

unsigned 
angel_FindTopOfMemory(void)
{
    unsigned memtop = FINDTOP_FIRST_ADDRESS_TO_CHECK;
    unsigned volatile *baseplace = (unsigned *)(FINDTOP_DRAM_WRAP_ADDR);
    unsigned baseval;

    /* Write a unique pattern into the bottom word so that we can detect
     * memory wrapping.
     */
    baseval = *baseplace;
    *baseplace = 0xDEADBEEF;

    /* Start from the DRAM Base and see if memory exists further up
     * If it does then we have memory up to there, so repeat ...
     * If not then we are at the top of memory already.  */
    do
    {
        unsigned tmpdata, origdata;
        unsigned volatile *testaddr = (unsigned *)(memtop + FINDTOP_BLOCK_STEP - 4);
        int failed = 0;

        origdata = *testaddr;
        *testaddr = 0x12345678;

        /* See if memory has wrapped */
        if (*baseplace != 0xDEADBEEF)
        {
            failed = 1;
        }

        tmpdata = *testaddr;
        *testaddr = origdata;

        if (tmpdata != 0x12345678)
        {
            failed = 1;
        }

        if (!failed)
            memtop += FINDTOP_BLOCK_STEP;
        else
            break;
    }
    while (memtop < FINDTOP_DRAM_MAX);

    *baseplace = baseval;
    return memtop;
}

#endif

/* EOF stacks.c */
