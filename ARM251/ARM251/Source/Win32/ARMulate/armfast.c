/* armfast.c - Fast ARMulator memory interface.
 * Copyright (C) Advanced RISC Machines Limited, 1995-1998. All rights reserved.
 * Copyright (C) ARM Limited, 1998. All rights reserved.
 *
 * RCS $Revision: 1.4 $
 * Checkin $Date: 1998/08/03 11:03:18 $
 * Revising $Author: dbryan $
 */

#include <string.h>             /* for memset */
#include "armdefs.h"
#include "armcnf.h"
#include "rdi.h"

#define ModelName (tag_t)"Fast"

#define MEMORYSIZE 2048 * 1024
#define OFFSETBITS (MEMORYSIZE-1)
#define OFFSETBITS_WORD (OFFSETBITS & ~3)

typedef struct {
  unsigned bigendMask;
  ARMword memory[MEMORYSIZE/4];
  ARMul_Cycles cycles;

} toplevel;

/*
 * ARMulator callbacks
 */

static void ConfigChange(void *handle, ARMword old, ARMword new)
{
  toplevel *top=(toplevel *)handle;
  IGNORE(old);
  top->bigendMask = (((new & MMU_B) != 0) != HostEndian) ? 3 : 0;
}

/*
 * Initialise the memory interface
 */

static ARMul_Error MemInit(ARMul_State *state,ARMul_MemInterface *interf,
                           ARMul_MemType type,toolconf config);

ARMul_MemStub ARMul_Fast = {
  MemInit,
  ModelName
  };

/*
 * Predeclare the memory access function so that the initialise function
 * can fill it in
 */
static int MemAccess(void *,ARMword,ARMword *,ARMul_acc);
static unsigned long ReadClock(void *handle);
static const ARMul_Cycles *ReadCycles(void *handle);

static ARMul_Error MemInit(ARMul_State *state,
                           ARMul_MemInterface *interf,
                           ARMul_MemType type,
                           toolconf config)
{
  toplevel *top;

  IGNORE(config);

  interf->read_clock=ReadClock;
  interf->read_cycles=ReadCycles;
  interf->x.basic.get_cycle_length=NULL;

  /* Fill in my functions */
  switch (type) {
  case ARMul_MemType_Basic:
  case ARMul_MemType_16Bit:
  case ARMul_MemType_Thumb:
  case ARMul_MemType_BasicCached:
  case ARMul_MemType_16BitCached:
  case ARMul_MemType_ThumbCached:
    interf->x.basic.access=MemAccess;
    break;
  default:
    return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,ModelName);
  }

  top=(toplevel *)malloc(sizeof(toplevel));
  if (top == NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  memset(&top->memory, 0, sizeof(top->memory));

  ARMul_PrettyPrint(state, ", %dMB", MEMORYSIZE/1024/1024);
  ARMul_SetMemSize(state,MEMORYSIZE);

  ARMul_InstallConfigChangeHandler(state,ConfigChange,top);
  ARMul_InstallExitHandler(state,free,top);

  interf->handle=top;

  return ARMulErr_NoError;
}

/*
 * Generic memory interface.
 */

/*
 * This is the most basic memory access function - an ARM6/ARM7 interface. 
 */
static int MemAccess(void *handle,
                     ARMword address,
                     ARMword *data,
                     ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;
  ARMword *ptr;
  ARMword offset;

  offset = address & OFFSETBITS_WORD;
  ptr=(ARMword *)((char *)(top->memory)+offset);

  if (acc==acc_LoadInstrS) {
    *data=*ptr;
    return 1;
  } else if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc & WIDTH_MASK) {
      case BITS_8:              /* read byte */
      address ^= top->bigendMask;
        *data = ((unsigned8 *)ptr)[address & 3];
        break;
        
      case BITS_16: {           /* read half-word */
        /* extract half-word */
#ifndef HOST_HAS_NO_16BIT_TYPE
        /*
         * unsigned16 is always a 16-bit type, but if there is no native
         * 16-bit type (e.g. ARM!) then we can do something a bit more
         * cunning.
         */
        address ^= top->bigendMask;
        *data = *((unsigned16 *)(((char *)ptr)+(address & 2)));
#else
        unsigned32 datum;
        datum=*ptr;
        address ^= top->bigendMask;
        if (address & 2) datum<<=16;
        *data = (datum>>16);
#endif
      }
        break;
        
      case BITS_32:             /* read word */
        *data=*ptr;
        break;

      default:
        return -1;
      }
    } else {
      switch (acc & WIDTH_MASK) {
        /* extract byte */
      case BITS_8:              /* write_byte */
        address ^= top->bigendMask;
        ((unsigned8 *)ptr)[address & 3]=(unsigned8)(*data);
        break;
        
      case BITS_16:             /* write half-word */
        address ^= top->bigendMask;
        *((unsigned16 *)(((char *)ptr)+(address & 2))) = (unsigned16)(*data);
        break;

      case BITS_32:             /* write word */
        *ptr=*data;
        break;

      default:
        return -1;
      }
    }                           /* internal cycle */
  }

  return 1;
}




static unsigned long ReadClock(void *handle)
{
  /* returns a us count */ 
  double t=0;
  return (unsigned long)t;
}

static const ARMul_Cycles *ReadCycles(void *handle)
{
  toplevel *top=(toplevel *)handle;
   return &(top->cycles);
}





/* EOF armfast.c */
