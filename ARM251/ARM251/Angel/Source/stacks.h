/* -*-C-*-
 *
 * $Revision: 1.2.2.1 $
 *   $Author: rivimey $
 *     $Date: 1998/10/19 12:22:03 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 * stacks.h - holds definitions of offsets of the various stacks
 * within the Angel Stack Area
 */

#ifndef angel_stacks_h
#define angel_stacks_h

#include "devconf.h"

typedef struct {
  unsigned *heapbase;
  unsigned *heaplimit;
  unsigned *stacktop;
  unsigned *stacklimit;
} AngelHeapStackDesc;

/* Note: not const, because profiling *may* alter some of the values */
extern AngelHeapStackDesc angel_heapstackdesc;

/* We define a global variable which holds the Top of Memory
 * In fact this is the top of contiguous memory.  It can be
 * overwriten at run time by the memory sizer.
 */
extern unsigned Angel_TopOfMemory;

/* This is a global variable which holds the base (low) address of
 * all the Angel stacks.  It can be modified at runtime should the
 * stacks live at the top of memory, so that when the memory
 * sizer detects how much memory is present, the stacks move
 */
extern unsigned Angel_StackBase;

/*
 * This is used to define an initial SVC stack for Applications. It is not
 * used by Angel at all. See the code in debugos.c:InitialiseApplication for
 * details.
 */
#define Angel_ApplSVCStackSize  0x80

/* All offsets are from Angel_StackBase */

#define Angel_ApplSVCStackLimitOffset 0
#define Angel_ApplSVCStackOffset    (Angel_ApplSVCStackLimitOffset + Angel_ApplSVCStackSize)

#define Angel_SVCStackLimitOffset   Angel_ApplSVCStackOffset
#define Angel_SVCStackOffset        (Angel_SVCStackLimitOffset + Angel_SVCStackSize)

#define Angel_FIQStackLimitOffset   Angel_SVCStackOffset
#define Angel_FIQStackOffset        (Angel_FIQStackLimitOffset + Angel_FIQStackSize)

#define Angel_IRQStackLimitOffset   Angel_FIQStackOffset
#define Angel_IRQStackOffset        (Angel_IRQStackLimitOffset + Angel_IRQStackSize)

#define Angel_ABTStackLimitOffset   Angel_IRQStackOffset
#define Angel_ABTStackOffset        (Angel_ABTStackLimitOffset + Angel_ABTStackSize)

#define Angel_AngelStackLimitOffset Angel_ABTStackOffset
#define Angel_AngelStackOffset      (Angel_AngelStackLimitOffset + Angel_AngelStackSize)

#define Angel_UNDStackLimitOffset   Angel_AngelStackOffset
#define Angel_UNDStackOffset        (Angel_UNDStackLimitOffset + Angel_UNDStackSize)

#if DEBUG == 1

#define Angel_FatalStackLimitOffset Angel_UNDStackOffset
#define Angel_FatalStackOffset      (Angel_FatalStackLimitOffset + Angel_FatalStackSize)

#define Angel_CombinedAngelStackSize Angel_FatalStackOffset

#else

#define Angel_CombinedAngelStackSize Angel_UNDStackOffset

#endif

/*
 *  Function: angel_RelocateWRTTopOfMemory
 * 
 *   Purpose: Called after a memory sizer has determined where the top
 *            of memory really is.  It updates dynamically changable
 *            holders of memory areas - eg. Angel stacks
 *
 *    Params: Top of Memory
 *
 *   Returns: 0 if top of memory unchanged
 *            1 if top of memory changed by this call
 *
 *   Special: This fn should only be provided in systems which
 *            can have pluggable DRAM systems or similar.
 */
extern unsigned angel_RelocateWRTTopOfMemory(unsigned int memorytop);


/*
 *  Function: angel_FindTopOfMemory
 * 
 *   Purpose: Called to detect where the top of low memory is
 *
 *    Params: None
 *
 *   Returns: The address of the top of contiguous low memory.
 *
 *   Special: This fn should only be provided in systems which
 *            can have pluggable DRAM systems or similar.
 */
extern unsigned angel_FindTopOfMemory(void);

#endif

/* EOF stacks.h */
