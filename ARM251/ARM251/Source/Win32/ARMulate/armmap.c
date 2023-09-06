/* armmap.c - Memory model that supports an "armsd.map" file */
/* Copyright (C) Advanced RISC Machines Limited, 1995. All rights reserved. */
/* Copyright (C) ARM Limited, 1998. All rights reserved. */

/*
 * RCS $Revision: 1.27 $
 * Checkin $Date: 1998/07/24 09:21:30 $
 * Revising $Author: mwilliam $
 */

#include <string.h>
#include <ctype.h>
#include "armdefs.h"
#include "armcnf.h"
#include "rdi_stat.h"

#include "linklist.h"

/* Possibly inline some functions */
#ifdef DONT_INLINE
#  define INLINE
#else
#  if defined(__GNUC__)
#    define INLINE __inline__
#  elif defined(_MSC_VER) || defined(__CC_NORCROFT)
#    define INLINE __inline
#  else
#    define INLINE
#  endif
#endif

#define ModelName (tag_t)"MapFile"

typedef struct MemDescr {
  RDI_MemDescr desc;
  /* cycle counters */
  /* @@@ N.B. this table RELIES on the bit positions in acc words.
   * for indexing and for it's size.
   */
#if (WIDTH_MASK | acc_Nrw | acc_seq | acc_Nmreq) == 0x7f
  unsigned long access[0x60];   /* number of accesses to this region */
  int counter[0x60];            /* wait states - -ve for "special" */
#else
#  error Code relies on things about ARMul_accs
#endif
  struct MemDescr *next;
} MemDescr;

typedef_LIST(MemDescr);

#define NUMPAGES 64 * 1024
#define PAGESIZE 64 * 1024
#define PAGEBITS 16
#define OFFSETBITS 0xffff
#define OFFSETBITS_WORD 0xfffc

typedef struct {
  ARMword memory[PAGESIZE/4];
} mempage;

typedef struct {
  mempage *page[NUMPAGES];
} memory;

typedef struct {
  unsigned long wait_states;    /* counter for wait states */
  ARMul_acc last_acc;           /* last cycle flag */
  int cnt;                      /* cycle-by-cycle counter */
} bus;

typedef struct {
  unsigned int bigendSig;
  MemDescr *desc;
  ARMul_Cycles cycles;
  unsigned long IS_cycles;      /* counter for IS_cycles */
  bus i, d;
  bool harvard_data_flag;
  memory mem;
  ARMul_State *state;

  /* Three values give the clock speed: 
   * clk - clockspeed, as cycle length in us
   * mult/period - integer representation of the same, for determining
   * the waitstates. The clock period is period*mult ns. This is done to
   * try to keep precision in division.
   */
  double clk;
  unsigned long mult,period;

  unsigned long prop;
} toplevel;

#define MAP_COUNTWAIT     0x0001
#define MAP_AMBABUSCOUNTS 0x0002
#define MAP_SPOTISCYCLES  0x0004
#define MAP_HARVARD       0x0008

/*
 * When spotting I-S cycles, a memory controller can either:
 * - speculatively decode all I cycles. This gives 2 cycles to do the I-S
 *   access.
 * - check for nMREQ in the middle of I cycles. This gives 1.5 cycles to do
 *   the access.
 * - process each cycle in turn. This gives only 1 cycle to do the access.
 * armmap.c can model each of these, "SPECULATIVE", "EARLY" or "LATE"
 */
#define MAP_ISMODE        0x0030
#define MAP_IS_SPEC       0x0010
#define MAP_IS_EARLY      0x0020
#define MAP_IS_LATE       0x0030

static const struct {
  tag_t option;
  unsigned long flag;
} MapOption[] = {
  ARMulCnf_CountWaitStates, MAP_COUNTWAIT,
  ARMulCnf_AMBABusCounts, MAP_AMBABUSCOUNTS,
  ARMulCnf_SpotISCycles, MAP_SPOTISCYCLES,
  NULL, 0, NULL
};

/*
 * Callback for installing the memory map.
 * Adds to the HEAD of the list
 */

static int InstallMemDescr(void *handle, RDI_MemDescr *md)
{
  toplevel *top=(toplevel *)handle;
  MemDescr *list;
  long cnt = 1, seq = 1, counter;
  int i, limit;
  ARMul_State *state=top->state;
  unsigned long mult=top->mult,period=top->period;

  list=linklist_new(MemDescr);
  if (list==NULL) return 1;

  if (top->desc==NULL) {
    ARMul_ConsolePrint(state,"Memory map:\n");
  }

  list->next=top->desc;
  top->desc=list;

  list->desc=*md;

  list->desc.limit+=list->desc.start-1;
  list->desc.width+=BITS_8;     /* 0->8 bits, 1->16, 2->32 */

  ARMul_ConsolePrint(state,"\
  %08x..%08x, %.2d-Bit, %c%c%c, wait states:",
                    list->desc.start,list->desc.limit,
                    1<<list->desc.width,
                    (list->desc.access & 4) ? '*' : ' ',
                    (list->desc.access & 2) ? 'w' : '-',
                    (list->desc.access & 1) ? 'r' : '-');


  /*
   * Cycle counts are kept in an array so that the mem_access function only
   * has to do an array lookup to get the cycle count. The work is done
   * here in setting up the array.
   * When spotting I-S cycles, the mem_access function uses an otherwise
   * unused entry (that for an idle cycle) to get the number of wait states.
   */

  limit = (top->prop & MAP_SPOTISCYCLES) ? 0x60 : 0x40;

  /* Many times around this loop correspond to illegal values of
   * acc_WIDTH.
   * Two values -- cnt and seq -- are used to say how many cycles are
   * needed for this type of access (N/S/I-S, Read or Write), cnt being
   * the number of cycles for the access, seq being for any sequential
   * cycles needed when accessing a value wider than the bus.
   * These values are invariant across the different bus widths, so are
   * set up the first time around the loop for a particular access type,
   * when the bus width is the illegal value '0'
   */

  for (i = 0; i < limit; i++) {
    counter=1;
    switch (acc_WIDTH(i)) {
    case BITS_32:
      counter=((list->desc.width==BITS_8) ? (cnt + seq * 3) :
               (list->desc.width==BITS_16) ? (cnt + seq) : cnt);
      if (acc_nSEQ(i) && acc_MREQ(i) && counter &&
          (top->prop & MAP_AMBABUSCOUNTS)) {
        counter++;              /* AMBA decode cycle for N */
      }
      break;

    case BITS_16:
      counter=((list->desc.width==BITS_8) ? (cnt + seq) : cnt);
      if (!acc_SEQ(i) && counter && top->prop & MAP_AMBABUSCOUNTS)
        counter++;              /* AMBA decode cycle for N */
      if (acc_READ(i) && acc_SEQ(i) && (list->desc.access & 4) &&
          counter > 1) {
        /* latched read possibly */
        counter = -counter;
      }
      break;

    default:
    case BITS_8:
      counter=cnt;
      if (!acc_SEQ(i) && counter && top->prop & MAP_AMBABUSCOUNTS)
         counter++;             /* AMBA decode cycle for N */
      break;

    case 0:
      /* First time round for this access type - get the base figures. */
      if (acc_nMREQ(i)) {
        /* Time for I-S cycles, if spotting them */
        if (acc_READ(i)) {
          if ((list->desc.access & 1) != 0) {   /* read okay */
            /* number of ticks needed for a sequential access */
            seq = list->desc.Sread_ns * mult;
            /* now divide by 'period' to get number of cycles, and add one
             * more cycle if there's any remainder */
            seq = (seq / period) + ((seq % period) != 0 || (seq == 0));

            /* cnt is set to the number of ticks needed for a non-seq
             * access. */
            cnt = list->desc.Nread_ns * mult;

            /* The number of wait-states needed for an I-S access depends
             * on the mode being used, which determines how many cycles there
             * are to start with.
             */
            switch (top->prop & MAP_ISMODE) {
            case MAP_IS_SPEC:
              /* For speculative decode, there are 2 cycles to start with */
              /* divide by the period to get the number of cycles, add one
               * more if there's any remainder, then take off the one free
               * we have from the speculative decode */
              cnt = (cnt / period) + ((cnt % period) != 0) - 1;
              break;

            case MAP_IS_EARLY:
              /* For the early decode, there are 1.5 cycles to start with */
              /* divide by the period to get the number of cycles, and add
               * one if the remainder is greater than half-a-cycle. */
              cnt = (cnt / period) + ((cnt % period) >= (period / 2));
              break;

            case MAP_IS_LATE:   /* one + wait cycle */
              /* For the late decode, there's only 1 cycle to start with */
              /* now divide by 'period' to get number of cycles, and add one
               * more cycle if there's any remainder */
              cnt = (cnt / period) + ((cnt % period) != 0);
              break;
            }
            /* Make sure we have at least 1 cycle! */
            if (cnt <= 0) cnt = 1;
          } else {
            seq = cnt = 0;      /* zero cycles signals 'abort' */
          }
        } else {                /* write */
          if ((list->desc.access & 1) != 0) {   /* read okay */
            /* see above */
            seq = list->desc.Swrite_ns * mult;
            seq = (seq / period) + ((seq % period) != 0 || (seq == 0));

            cnt = list->desc.Nwrite_ns * mult;
            switch (top->prop & MAP_ISMODE) {
            case MAP_IS_SPEC:   /* two cycles */
              cnt = (cnt / period) + ((cnt % period) != 0) - 1;
              break;
            case MAP_IS_EARLY:  /* 1.5 cycles */
              cnt = (cnt / period) + ((cnt % period) >= (period / 2));
              break;
            case MAP_IS_LATE:   /* one cycle */
              cnt = (cnt / period) + ((cnt % period) != 0);
              break;
            }
            if (cnt <= 0) cnt = 1;
          } else {
            seq = cnt = 0;
          }
        }
      } else if (acc_READ(i)) { /* read */
        if (list->desc.access & 1) { /* read access okay */
          /* see above */
          seq = list->desc.Sread_ns * mult;
          seq = (seq / period) + ((seq % period) != 0 || (seq == 0));
          if (acc_SEQ(i)) {
            cnt = seq;
          } else {
            /* see above */
            cnt = list->desc.Nread_ns * mult;
            cnt = (cnt / period) + ((cnt % period) != 0 || (cnt == 0));
          }
        } else {
          seq = cnt = 0;
        }
      } else {                  /* write */
        if (list->desc.access & 2) { /* write access okay */
          /* see above */
          seq = list->desc.Swrite_ns * mult;
          seq = (seq / period) + ((seq % period) != 0 || (seq == 0));
          if (acc_SEQ(i)) {
            cnt = seq;
          } else {
            /* see above */
            cnt = list->desc.Nwrite_ns * mult;
            cnt = (cnt / period) + ((cnt % period) != 0 || (cnt == 0));
          }
        } else {
          seq = cnt = 0;
        }
      }

      /* report the wait states */
      ARMul_ConsolePrint(state, " %c", acc_READ(i) ? 'R' : 'W');
      if (acc_nMREQ(i)) {
        ARMul_ConsolePrint(state, "IS");
      } else {
        ARMul_ConsolePrint(state, "%c", acc_SEQ(i) ? 'S' : 'N');
      }

      if (cnt != 0) {
        ARMul_ConsolePrint(state, "=%d", cnt - 1);
        if ((acc_nSEQ(i) || acc_nMREQ(i)) && seq != cnt) {
          ARMul_ConsolePrint(state, "/%d", seq - 1);
        }
      } else
        ARMul_ConsolePrint(state, "=Abt");
    }
    list->access[i]=0;
    /*
     * normally "counter" will be the number of cycles the access takes, so
     *   "counter-1" is the number of wait states.
     * if this would abort, "counter==0", so wait states becomes -ve - i.e. -1.
     * if a 16-bit sequential read to latched memory, then the value is -ve
     *   number of cycles. "counter-1" could only ==-1 iff counter=-0, i.e. abort,
     *   so these -ve numbers don't overlap with the "abort" signal. The real
     *   number of wait states is "(-counter)-1" i.e. "-(-counter-1)-2"
     */
    list->counter[i]=counter-1; /* number of wait states, or special */
  }

  ARMul_ConsolePrint(state,"\n");

  return 0;
}

/*
 * Callback for telling about memory stats
 */
static const RDI_MemAccessStats *GetAccessStats(void *handle,
                                                RDI_MemAccessStats *stats,
                                                ARMword s_handle)
{
  MemDescr *desc;
  toplevel *top=(toplevel *)handle;

  desc=top->desc;

  while (desc) {
    if (desc->desc.handle==s_handle) {
      int i;

      stats->Nreads=stats->Nwrites=stats->Sreads=stats->Swrites=0;
      stats->ns=stats->s=0;

      for (i=0;i<0x40;i++) {
        unsigned long count;
        double ns;
        count=desc->access[i];
        if (acc_READ(i)) {
          if (acc_SEQ(i))
            stats->Sreads+=count;
          else
            stats->Nreads+=count;
        } else {
          if (acc_SEQ(i))
            stats->Swrites+=count;
          else
            stats->Nwrites+=count;
        }

        if (desc->counter[i]>=0) {
          ns=((double)(count*(desc->counter[i]+1))*top->clk*1000.0);
        } else {
          ns=((double)(count)*top->clk*1000.0);
        }

        while (ns>1e9) {
          ns-=1e9;
          stats->s++;
        }

        stats->ns+=(unsigned long)ns;

        /* I don't think this can go round more than once, but there's no harm
         * being safe - this isn't time critical code */
        while (stats->ns>(unsigned long)1e9) {
          stats->ns-=(unsigned long)1e9;
          stats->s++;
        }
      }

      return stats;
    }
    desc=desc->next;
  }

  return NULL;                  /* not found */
}

/*
 * Function to deal with RDI calls related to memory maps
 */

static int RDI_info(void *handle, unsigned type, ARMword *arg1, ARMword *arg2)
{
  switch (type) {
  case RDIMemory_Access:
    if (GetAccessStats(handle,(RDI_MemAccessStats *)arg1,*arg2))
      return RDIError_NoError;
    else
      return RDIError_NoSuchHandle;
    
  case RDIMemory_Map: {
    int n=(int)*arg2;
    RDI_MemDescr *p=(RDI_MemDescr *)arg1;
    while (--n >= 0)
      if (InstallMemDescr(handle,p)!=0)
        return RDIError_Error;
  }
    return RDIError_NoError;

  case RDIInfo_Memory_Stats:
    return RDIError_NoError;

  case RDICycles: {
    toplevel *top=(toplevel *)handle;
    ARMul_State *state = top->state;
    ARMword total, idle;
    ARMul_AddCounterValue(state, arg1, arg2, NO, &top->cycles.NumScycles);
    ARMul_AddCounterValue(state, arg1, arg2, NO, &top->cycles.NumNcycles);
    total = (top->cycles.NumScycles + top->cycles.NumNcycles +
             top->cycles.NumIcycles + top->cycles.NumCcycles);
    if (!(top->prop & MAP_HARVARD) && (top->prop & MAP_AMBABUSCOUNTS)) {
      idle = top->cycles.NumIcycles + top->cycles.NumCcycles;
      ARMul_AddCounterValue(state, arg1, arg2, NO, &idle);
    } else {
      idle = top->cycles.NumIcycles;
      ARMul_AddCounterValue(state, arg1, arg2, NO, &top->cycles.NumIcycles);
      ARMul_AddCounterValue(state, arg1, arg2, NO, &top->cycles.NumCcycles);
    }
    if (top->prop & MAP_COUNTWAIT) {
      total += top->d.wait_states;
      ARMul_AddCounterValue(state, arg1, arg2, NO, &top->d.wait_states);
      if (top->prop & MAP_HARVARD) {
        total += top->i.wait_states;
        ARMul_AddCounterValue(state, arg1, arg2, NO, &top->i.wait_states);
      }
    }
    ARMul_AddCounterValue(state, arg1, arg2, NO, &total);
    if (top->prop & MAP_SPOTISCYCLES) {
      idle -= top->IS_cycles;
      ARMul_AddCounterValue(state, arg1, arg2, NO, &idle);
    }
  }
    break;

  case RDIRequestCyclesDesc: {
    toplevel *top=(toplevel *)handle;
    ARMul_State *state = top->state;
    if (top->prop & MAP_HARVARD) {
      ARMul_AddCounterDesc(state, arg1, arg2, "ID_Cycles");
      ARMul_AddCounterDesc(state, arg1, arg2, "IBus_Cycles");
      ARMul_AddCounterDesc(state, arg1, arg2, "Idle_Cycles");
      ARMul_AddCounterDesc(state, arg1, arg2, "DBus_Cycles");
    } else if (top->prop & MAP_AMBABUSCOUNTS) {
      ARMul_AddCounterDesc(state, arg1, arg2, "S_Cycles");
      ARMul_AddCounterDesc(state, arg1, arg2, "N_Cycles");
      ARMul_AddCounterDesc(state, arg1, arg2, "A_Cycles");
    } else {
      ARMul_AddCounterDesc(state, arg1, arg2, "S_Cycles");
      ARMul_AddCounterDesc(state, arg1, arg2, "N_Cycles");
      ARMul_AddCounterDesc(state, arg1, arg2, "I_Cycles");
      ARMul_AddCounterDesc(state, arg1, arg2, "C_Cycles");
    }
    if (top->prop & MAP_COUNTWAIT) {
      if (top->prop & MAP_HARVARD) {
        ARMul_AddCounterDesc(state, arg1, arg2, "D_Wait_States");
        ARMul_AddCounterDesc(state, arg1, arg2, "I_Wait_States");
      } else {
        ARMul_AddCounterDesc(state, arg1, arg2, "Wait_States");
      }
    }
    ARMul_AddCounterDesc(state, arg1, arg2, "Total");
    if (top->prop & MAP_SPOTISCYCLES) {
      ARMul_AddCounterDesc(state, arg1, arg2, "True_Idle_Cycles");
    }
  }
  break;

  default:
    /* check for capability messages */
    if (type & RDIInfo_CapabilityRequest)
      switch (type & ~RDIInfo_CapabilityRequest) {
      case RDIMemory_Access:
      case RDIMemory_Map:
      case RDIInfo_Memory_Stats:
        return RDIError_NoError;

      default:
        break;                  /* fall through */
      }
    break;
  }
  return RDIError_UnimplementedMessage;
}

/*
 * Other ARMulator callbacks
 */

static void ConfigChange(void *handle, ARMword old, ARMword new)
{
  toplevel *top=(toplevel *)handle;  

  IGNORE(old);

  top->bigendSig=((new & MMU_B) != 0);
}


/* ********************************************************************
Function Name: Interrupt
Parameters: void * handle  - the 'top' pointer but with no type info.
            unsigned int which - the interrupt type. see armdefs.h
                for codes corresponding to interrupt types.
Return: void
Description: This function is called by armul_InterruptUpcall iff
        the armmap memory model is being used. See armdefs.h for the
        latest event codes - bit0=FIQ, bit1=IRQ, bit2 = Reset.
        On a reset this function resets ALL memory access statistics.
        This function is NOT an interrupt handler. 
 **********************************************************************
*/

static void Interrupt(void *handle,unsigned int which)
{
  MemDescr *desc,*next;
  int i;

  toplevel *top=(toplevel *)handle;
  if (which & ARMul_InterruptUpcallReset) 
      {
      top->cycles.NumNcycles=0;
      top->cycles.NumScycles=0;
      top->cycles.NumIcycles=0;
      top->cycles.NumCcycles=0;
      top->cycles.NumFcycles=0;
      top->d.wait_states = 0;
      top->d.cnt = 0;
      top->i.wait_states = 0;
      top->i.cnt = 0;
      top->IS_cycles = 0;

     /* Clear the memstats ( memory_statistics ) data fields .
        This happens on startup, load, reload. 
     */
     for (desc=top->desc; desc; desc=next) 
         {
         next=desc->next;
         for (i=0; i < 0x60; i++)
             {
             desc->access[i] = 0;
             } /* end for i=0 */
         } /* end for desc=top */

  } /* end if (which &... */

}



/*
 * Initialise the memory interface
 */

static ARMul_Error MemInit(ARMul_State *state,ARMul_MemInterface *interf,
                           ARMul_MemType type,toolconf config);

ARMul_MemStub ARMul_MapFile = {
  MemInit,
  ModelName
  };

/*
 * Predeclare the memory access functions so that the initialise function
 * can fill them in
 */
static int MemAccess(void *,ARMword,ARMword *,ARMul_acc);
static int MemAccessSA(void *,ARMword,ARMword *,ARMul_acc);
static void MemExit(void *);
static unsigned long ReadClock(void *handle);
static const ARMul_Cycles *ReadCycles(void *handle);
static unsigned long GetCycleLength(void *handle);
static unsigned int DataCacheBusy(void *);

static ARMul_Error MemInit(ARMul_State *state,
                           ARMul_MemInterface *interf,
                           ARMul_MemType type,
                           toolconf config)
{
  memory *mem;
  unsigned page, i;
  unsigned long clk;
  ARMword clk_speed = 0;
  const char *option;
  toplevel *top;
  long nano_mult;
  unsigned long prop = 0;
  
  /* Fill in my functions */
  switch (type) {
  case ARMul_MemType_Basic: case ARMul_MemType_BasicCached:
  case ARMul_MemType_16Bit: case ARMul_MemType_16BitCached:
  case ARMul_MemType_Thumb: case ARMul_MemType_ThumbCached:
    interf->x.basic.access=MemAccess;
    interf->x.basic.get_cycle_length=GetCycleLength;
    break;

  case ARMul_MemType_StrongARM:
  case ARMul_MemType_ARM9:
    interf->x.strongarm.access = MemAccessSA;
    interf->x.strongarm.get_cycle_length = GetCycleLength;
    interf->x.strongarm.data_cache_busy = DataCacheBusy;
    ARMul_PrettyPrint(state,", double bus");
    prop = MAP_HARVARD;
    break;

  default: {
    /* Cannot support .map files for this type of memory system,
     * so we default to whatever we can. */
    extern ARMul_MemStub *ARMul_Memories[];
    if (ARMul_Memories[0] && ARMul_Memories[0]->init &&
        ARMul_Memories[0]->init!=MemInit) {
      ARMul_ConsolePrint(state,"Warning: '.map' file will be ignored\n");
      return ARMul_Memories[0]->init(state,interf,type,config);
    } else {
      return ARMul_RaiseError(state,ARMulErr_MemTypeUnhandled,
                              ModelName);
    }
  }
  }

  interf->read_clock=ReadClock;
  interf->read_cycles=ReadCycles;

  top=(toplevel *)malloc(sizeof(toplevel));
  if (top == NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);
  mem=&top->mem;

  top->state=state;

  ARMul_PrettyPrint(state,", Memory map");

  top->desc=NULL;

  option=ToolConf_Lookup(config,ARMulCnf_MCLK);
  if (option != NULL) clk_speed = ToolConf_Power(option,FALSE);
  if (option == NULL || clk_speed == 0) {
    top->mult = 1;
    top->period = 1000;
    top->clk = 1.0;
  } else {
    clk=clk_speed/10000; clk=clk ? clk : 1;
    clk=100000000/clk;            /* in picoseconds */

    /* shrink this to four sig. figures */
    nano_mult=1000;               /* nanoseconds->picoseconds */
    while ((clk>=10000 || (clk/10)*10==clk) && nano_mult>10) {
      clk/=10;                    /* range reduce */
      nano_mult/=10;
    }
    top->mult=nano_mult; top->period=clk;

    top->clk=1000000.0/clk_speed; /* in microseconds */
  }

  /* only report the speed if "CPU Speed" has been set in the config */
  if (ToolConf_Lookup(config, Dbg_Cnf_CPUSpeed) != NULL) {
    char *fac;
    double clk=ARMul_SIRange(clk_speed,&fac,FALSE);
    ARMul_PrettyPrint(state,", %.1f%sHz",clk,fac);
  }

  for (i = 0; MapOption[i].option != NULL; i++) {
    const char *option=ToolConf_Lookup(config, MapOption[i].option);
    if (option)
      prop = ToolConf_AddFlag(option, prop, MapOption[i].flag, TRUE);
  }

  if (prop & MAP_SPOTISCYCLES) {
    char const *option = ToolConf_Lookup(config, ARMulCnf_ISTiming);
    if (option != NULL && ToolConf_Cmp(option, "SPECULATIVE"))
      prop |= MAP_IS_SPEC;
    else if (option != NULL && ToolConf_Cmp(option, "EARLY"))
      prop |= MAP_IS_EARLY;
    else
      prop |= MAP_IS_LATE;
  }

  top->prop = prop;

  for (page=0; page<NUMPAGES; page++) {
    mem->page[page]=NULL;
  }

  top->cycles.NumNcycles=0;
  top->cycles.NumScycles=0;
  top->cycles.NumIcycles=0;
  top->cycles.NumCcycles=0;
  top->cycles.NumFcycles=0;

  top->d.wait_states=0;
  top->d.cnt=0;

  top->i.wait_states=0;
  top->i.cnt=0;

  top->harvard_data_flag = FALSE;

  ARMul_PrettyPrint(state, ", 4GB");

{
  unsigned long memsize=0;
  option=ToolConf_Lookup(config,Dbg_Cnf_MemorySize);
  if (option) memsize=ToolConf_Power(option,TRUE);
  else memsize=0x80000000;
  ARMul_SetMemSize(state,memsize);
}

  ARMul_InstallUnkRDIInfoHandler(state,RDI_info,top);
  ARMul_InstallInterruptHandler(state,Interrupt,top);
  ARMul_InstallConfigChangeHandler(state,ConfigChange,top);
  ARMul_InstallExitHandler(state,MemExit,top);

  interf->handle=top;

  return ARMulErr_NoError;
}

/*
 * Remove the memory interface
 */

static void MemExit(void *handle)
{
  ARMword page;
  toplevel *top=(toplevel *)handle;
  memory *mem=&top->mem;
  MemDescr *desc,*next;

  /* free all truly allocated pages */
  for (page=0; page<NUMPAGES; page++) {
    mempage *pageptr= mem->page[page];
    if (pageptr) {
      free((char *)pageptr);
    }
  }

  for (desc=top->desc; desc; desc=next) {
    next=desc->next;
    linklist_free(MemDescr,desc);
  }

  /* free top-level structure */
  free(top);
}

static mempage *NewPage(void)
{
  unsigned int i;
  mempage *page=(mempage *)malloc(sizeof(mempage));
  /*
   * We fill the new page with data that, if the ARM tried to execute
   * it, it will cause an undefined instruction trap (whether ARM or
   * Thumb)
   */
  for (i=0;i<PAGESIZE/4;) {
    page->memory[i++]=0xe7ff0010; /* an ARM undefined instruction */
    /* The ARM undefined insruction has been chosen such that the
     * first halfword is an innocuous Thumb instruction (B 0x2)
     */
    page->memory[i++]=0xe800e800; /* a Thumb undefined instruction */
  }
  return page;
}

/*
 * Generic memory interface. Just alter this for a memtype memory system
 */

static INLINE int FindMemoryRegion(
    toplevel *top, ARMword address, ARMword *data, ARMul_acc acc,
    bus *bus)
{
  /* first cycle of the access */
  MemDescr *desc;
  unsigned long prop = top->prop;
      
  for (desc = top->desc; desc; desc = desc->next) {
    if (address >= desc->desc.start &&
        address <= desc->desc.limit) {

      desc->access[acc & (WIDTH_MASK | acc_seq | acc_Nrw)]++;

      /* If we're looking for I-S cycles, then use the N cycle
       * counter if we're on an I-S boundary
       */
      if ((prop & MAP_SPOTISCYCLES) &&
          acc_SEQ(acc) &&
          acc_nMREQ(bus->last_acc)) {
        /* Use the 'this width, idle, read/write' counter */
        bus->cnt = desc->counter[(acc & (WIDTH_MASK | acc_Nrw)) | acc_Nmreq];
        if (prop & MAP_AMBABUSCOUNTS) {
          /* This count includes an extra cycle for AMBA decode. We
           * don't need this for I-S cycles, so we need to remove it.
           */
          if (bus->cnt > 0) bus->cnt--;
        }
        top->IS_cycles++;
      } else
        bus->cnt = desc->counter[acc & (WIDTH_MASK | acc_seq | acc_Nrw)];

      if (bus->cnt) {
        if (bus->cnt < 0) {
          if (bus->cnt == -1) {/* abort */
            bus->cnt = 0;
            bus->last_acc = acc;
            return -1;
          }
          /*
           * otherwise, this is a 16-bit SEQ read
           * - takes 1 cycle if an even address,
           * - takes -cnt-2 cycles otherwise.
           */
          if (address & 1) bus->cnt = -bus->cnt-2;
          else break;       /* i.e. fall through - 1 cycle */
        }
        bus->wait_states++;
        bus->last_acc = acc;
        return 0;
      }
      break;                /* fall through */
    }
  }
  if ((prop & MAP_AMBABUSCOUNTS) &&
      acc_MREQ(acc) && acc_nSEQ(acc)) {
    /* needs an address decode cycle */
    bus->cnt = 1;
    bus->wait_states++;
    bus->last_acc = acc;
    return 0;
  }

  return 1;
}

static int MemLookup(
    toplevel *top, ARMword address, ARMword *data, ARMul_acc acc)
{
  unsigned int pageno;
  mempage *page;
  ARMword *ptr;
  ARMword offset;

  pageno = address >> PAGEBITS;
  page=top->mem.page[pageno];
  if (page == NULL) {
    top->mem.page[pageno] = page = NewPage();
  }
  offset = address & OFFSETBITS_WORD;
  ptr=(ARMword *)((char *)(page->memory)+offset);
  
  if (acc==acc_LoadInstrS) {
    *data=*ptr;
    return 1;
  } else if (acc_MREQ(acc)) {
    if (acc_READ(acc)) {
      switch (acc & WIDTH_MASK) {
      case BITS_8:              /* read byte */
        if (HostEndian!=top->bigendSig) address^=3;
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
        if (HostEndian!=top->bigendSig) address^=2;
        *data = *((unsigned16 *)(((char *)ptr)+(address & 2)));
#else
        unsigned32 datum;
        datum=*ptr;
        if (HostEndian!=state->bigendSig) address^=2;
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
        if (HostEndian!=top->bigendSig) address^=3;
        ((unsigned8 *)ptr)[address & 3] = (unsigned8)(*data);
        break;
          
      case BITS_16:             /* write half-word */
        if (HostEndian!=top->bigendSig) address^=2;
        *((unsigned16 *)(((char *)ptr)+(address & 2))) = (unsigned16)(*data);
#if 0   /* An alternative implementation for machines with no native 16-bit
         * type. */
        if (address & 2)
          *ptr=(*ptr & ~0xffffL) | (*data & 0xffffL);
        else 
          *ptr=(*ptr & 0xffffL) | (*data << 16);
#endif
        break;
          
      case BITS_32:             /* write word */
        *ptr=*data;
        break;
        
      default:
        return -1;
      }
    }
  }
    
  return 1;
}


static int MemAccess(
    void *handle, ARMword address, ARMword *data, ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;

  if (acc_ACCOUNT(acc)) {
    if (acc_MREQ(acc)) {
      if (top->d.cnt == 0 || acc != top->d.last_acc) {
        int rv = FindMemoryRegion(top, address, data, acc, &top->d);
        if (rv != 1) {
          return rv;
        }
        /* else fall through */
      } else {
        /* not the first cycle */
        if (--top->d.cnt) {
          top->d.wait_states++;
          top->d.last_acc = acc;
          return 0;
        }
        /* else fall through */
      }

      if (acc_SEQ(acc)) {
        top->cycles.NumScycles++;
      } else {
        top->cycles.NumNcycles++;
      }
    } else {
      if (acc_SEQ(acc)) {
        top->cycles.NumCcycles++;
      } else {
        top->cycles.NumIcycles++;
      }
    }
    top->d.cnt = 0;
    top->d.last_acc = acc;
  }

  return MemLookup(top, address, data, acc);
}

static int MemAccessSA(
    void *handle, ARMword address, ARMword *data, ARMul_acc acc)
{
  toplevel *top=(toplevel *)handle;

  if (acc_ACCOUNT(acc)) {
    bus *bus;
    if (acc_OPC(acc)) {
      bus = &top->i;
    } else {
      bus = &top->d;
    }
    if (acc_MREQ(acc)) {
      if (bus->cnt == 0 || acc != bus->last_acc) {
        int rv = FindMemoryRegion(top, address, data, acc, bus);
        if (rv != 1) {
          return rv;
        }
        /* else fall through */
      } else {
        /* not the first cycle */
        if (--bus->cnt) {
          bus->wait_states++;
          bus->last_acc = acc;
          return 0;
        }
        /* else fall through */
      }

      /*
       * On Harvard architectures there are four types of cycle -
       * we'll reuse the four cycle counters for these:
       *
       *  Instruction fetched, No data fetched      N
       *  Instruction fetched, data fetched         S
       *  No instruction fetched, No data fetched   I
       *  No instruction fetched, data fetched      C
       */

      if (acc_OPC(acc)) {
        /* End of cycle - account for access */
        /* This access is either acc_LoadInstrN or acc_NoFetch */
        if (top->harvard_data_flag) {
          /* data fetched */
          top->harvard_data_flag = FALSE;
          top->cycles.NumScycles++; /* instr/data */
        } else {
          /* no data fetched */
          top->cycles.NumNcycles++;/* instr/no data */
        }
      } else {
        /* don't count on data accesses */
        /* data fetched */
        top->harvard_data_flag = TRUE;
      }
    } else {
      if (acc_OPC(acc)) {
        /* End of cycle - account for access */
        /* This access is either acc_LoadInstrN or acc_NoFetch */
        if (top->harvard_data_flag) {
          /* data fetched */
          top->harvard_data_flag = FALSE;
          top->cycles.NumCcycles++; /* no instr/data */
        } else {
          /* no data fetched */
          top->cycles.NumIcycles++;/* no instr/no data */
        }
      }
    }
    bus->cnt = 0;
    bus->last_acc = acc;
  }

  return MemLookup(top, address, data, acc);
}

static const ARMul_Cycles *ReadCycles(void *handle)
{
  static ARMul_Cycles cycles;
  toplevel *top=(toplevel *)handle;

  if (top->prop & (MAP_COUNTWAIT | MAP_AMBABUSCOUNTS)) {
    cycles.NumNcycles = top->cycles.NumNcycles;
    cycles.NumScycles = top->cycles.NumScycles;
    cycles.NumIcycles = top->cycles.NumIcycles + top->cycles.NumCcycles;
    cycles.NumCcycles = top->d.wait_states + top->i.wait_states;
    cycles.Total = 0;
  } else {
    cycles = top->cycles;
    cycles.Total = top->d.wait_states + top->i.wait_states;
  }
  
  cycles.Total += (cycles.NumNcycles + cycles.NumScycles +
                   cycles.NumIcycles + cycles.NumCcycles);

  return &cycles;
}

static unsigned long ReadClock(void *handle)
{
  const ARMul_Cycles *cycles = ReadCycles(handle);
  /* returns a us count */
  toplevel *top = (toplevel *)handle;
  double t = (double)(cycles->Total) * top->clk;
  return (unsigned long)t;
}

static unsigned long GetCycleLength(void *handle)
{
  /* Returns the cycle length in tenths of a nanosecond */
  toplevel *top=(toplevel *)handle;
  return (unsigned long)(top->clk * 10000.0);
}

static unsigned int DataCacheBusy(void *handle)
{
  IGNORE(handle);
  return FALSE;
}


/* EOF armmap.c */
