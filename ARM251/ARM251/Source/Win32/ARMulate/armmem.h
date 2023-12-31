/* armmem.h - Interface for memory models to the ARMulator */
/* Copyright (C) 1996-1998 ARM Limited. All rights reserved. */

/*
 * RCS $Revision: 1.27.6.1 $
 * Checkin $Date: 1998/12/09 13:40:07 $
 * Revising $Author: dcleland $
 */

#ifndef armmem_h
#define armmem_h

#include "host.h"               /* void * */
#include "rdi_stat.h"           /* RDI_MemAccessStats */

typedef unsigned int ARMul_acc; /* A flag word, containing.... */

#define acc_Nrw     0x10        /* not read/write */
#define acc_seq     0x20        /* sequential */
#define acc_Nmreq   0x40        /* not memory request */
#define acc_Nopc    0x80        /* not opcode fetch */
#define acc_rlw    0x100        /* read-lock-write */
/* Trans is not supported - use TransChangeUpcall instead */
/* #define acc_Ntrans 0x200      * not trans (only for LDRT etc.) */
#define acc_spec   0x400        /* speculative instruction fetch (ARM810) */
#define acc_new    0x800        /* a new request (StrongARM) */

#define acc_burst0  0x1000      /* AMBA BURST[0:1] pins set on ARM 9xxT */
#define acc_burst1  0x2000        
#define acc_BURST0(w)       ((w) & acc_burst0)
#define acc_BURST1(w)       ((w) & acc_burst1)
#define acc_BURST_TYPE(w)   (w & (acc_burst0 | acc_burst1))
#define acc_burst_none      0
#define acc_burst_4word     acc_burst0
#define acc_burst_8word     acc_burst1
#define acc_burst_reserved  (acc_burst0 | acc_burst1)  

#define acc_WRITE(w)   ((w) & acc_Nrw)
#define acc_SEQ(w)     ((w) & acc_seq)
#define acc_nMREQ(w)   ((w) & acc_Nmreq)
#define acc_nOPC(w)    ((w) & acc_Nopc)
#define acc_LOCK(w)    ((w) & acc_rlw)
/* #define acc_nTRANS(w)  ((w) & acc_Ntrans) */
#define acc_SPEC(w)    ((w) & acc_spec)
#define acc_NEW(w)     ((w) & acc_new)

#define acc_READ(w)    (!acc_WRITE(w))
#define acc_nSEQ(w)    (!acc_SEQ(w))
#define acc_MREQ(w)    (!acc_nMREQ(w))
#define acc_OPC(w)     (!acc_nOPC(w))
#define acc_nLOCK(w)   (!acc_LOCK(w))
/* #define acc_TRANS(w)   (!acc_nTRANS(w)) */
#define acc_nSPEC(w)   (!acc_SPEC(w))
#define acc_nNEW(w)    (!acc_NEW(w))

#define acc_typeN  0
#define acc_typeS  acc_seq
#define acc_typeI  acc_Nmreq
#define acc_typeC  (acc_Nmreq | acc_seq)

#define acc_CYCLE(w)  (w & (acc_Nmreq | acc_seq))

/*
 * The bottom four bits of the access word define the width of the access.
 * The number used is LOG_2(number-of-bits)
 */

#define BITS_8  3
#define BITS_16 4
#define BITS_32 5
#define BITS_64 6
#define WIDTH_MASK 0x0f

#define acc_WIDTH(w) ((w) & WIDTH_MASK)

/*
 * Or it's byte lanes - 1 bit per byte
 */

#define acc_byte_0 0x1
#define acc_byte_1 0x2
#define acc_byte_2 0x4
#define acc_byte_3 0x8
#define BYTELANE_MASK 0xf

#define acc_BYTELANE(w) ((w) & BYTELANE_MASK)


#define acc_BYTE0(acc) ((acc) & acc_byte_0)
#define acc_BYTE1(acc) ((acc) & acc_byte_1)
#define acc_BYTE2(acc) ((acc) & acc_byte_2)
#define acc_BYTE3(acc) ((acc) & acc_byte_3)

#define acc_LoadInstrS    (BITS_32 | acc_typeS)
#define acc_LoadInstrS2   (BITS_64 | acc_typeS)
#define acc_LoadInstrS2Spec   (BITS_64 | acc_typeS | acc_spec)
#define acc_LoadInstrN    (BITS_32 | acc_typeN)
#define acc_LoadInstrN2   (BITS_64 | acc_typeN)
#define acc_LoadInstrN2Spec   (BITS_64 | acc_typeN | acc_spec)
#define acc_LoadInstr16S  (BITS_16 | acc_typeS)
#define acc_LoadInstr16N  (BITS_16 | acc_typeN)
#define acc_LoadWordS     (BITS_32 | acc_typeS | acc_Nopc)
#define acc_LoadWordS2    (BITS_64 | acc_typeS | acc_Nopc)
#define acc_LoadWordN     (BITS_32 | acc_typeN | acc_Nopc)
#define acc_LoadWordN2    (BITS_64 | acc_typeN | acc_Nopc)
#define acc_LoadByte      (BITS_8  | acc_typeN | acc_Nopc)
#define acc_LoadHalfWord  (BITS_16 | acc_typeN | acc_Nopc)

#define acc_StoreWordS    (BITS_32 | acc_typeS | acc_Nrw | acc_Nopc)
#define acc_StoreWordN    (BITS_32 | acc_typeN | acc_Nrw | acc_Nopc)
#define acc_StoreByte     (BITS_8  | acc_typeN | acc_Nrw | acc_Nopc)
#define acc_StoreHalfWord (BITS_16 | acc_typeN | acc_Nrw | acc_Nopc)

#define acc_NoFetch (acc_typeI)

#define acc_Icycle (acc_typeI | acc_Nopc)
#define acc_Ccycle (acc_typeC | acc_Nopc)

#define acc_NotAccount    (1u<<31)
#define acc_nACCOUNT(x)   ((x) & acc_NotAccount)
#define acc_ACCOUNT(x)    (!acc_nACCOUNT(x))

#define acc_DontAccount(X)  ((X) | acc_NotAccount)

#define acc_ReadWord      acc_DontAccount(acc_LoadWordN)
#define acc_ReadHalfWord  acc_DontAccount(acc_LoadHalfWord)
#define acc_ReadByte      acc_DontAccount(acc_LoadByte)

#define acc_WriteWord     acc_DontAccount(acc_StoreWordN)
#define acc_WriteHalfWord acc_DontAccount(acc_StoreHalfWord)
#define acc_WriteByte     acc_DontAccount(acc_StoreByte)

/*
 * Definitions of things in the memory interface
 */

typedef struct armul_meminterface ARMul_MemInterface;

/*
 * There are many different revisions of the memory interface,
 * relating to different processor types
 */
typedef enum {
  /*
   * Basic memory interface - one access per cycle
   */
  /* Basic:
   *   N/S/I/C cycles
   *   instr fetch seq/non-seq
   *   word read/write seq/non-seq
   *   byte read/write non-seq
   *   word/byte locked read/write non-seq */
  ARMul_MemType_Basic,          /* only does word/byte accesses */
  /* 16Bit: as Basic, but adds:
   *   half-word read/write non-seq */
  ARMul_MemType_16Bit,          /* can also do 16-bit data accesses */
  /* Thumb: as 16Bit, but adds:
   *   half-word instr fetch seq/non-seq */
  ARMul_MemType_Thumb,          /* can also do 16-bit instr accesses */

  /*
   * Cached memory interface - never performs 'N' cycles
   */
  /* Cached processors only ever do "cycle" and "not a cycle" */
  ARMul_MemType_BasicCached,
  ARMul_MemType_16BitCached,
  ARMul_MemType_ThumbCached,

  /*
   * Pipelined versions for CoVerification
   */
  ARMul_MemType_PipelinedAMBA,

  /*
   * The ARM8 core can request two sequential accesses per cycle
   */
  /* ARM8: as 16Bit, but adds:
   *   instr fetch, double bandwidth, seq/non-seq
   *   word read, double bandwidth, seq/non-seq */
  ARMul_MemType_ARM8,           /* double bandwidth */

  /*
   * StrongARM makes one or two seperate requests per cycle, the last
   * one of which will have nOPC low.
   */
  ARMul_MemType_StrongARM,      /* StrongARM interface */

  /*
   * The StrongARM's external memory interface supports byte-lanes.
   * In such a model the bottom four bits of "acc" are a mask, rather
   * than a width specifier, and the bottom two bits of the address
   * are assumed to be zero, and the word written/read will have the
   * data to be written in the specified bytes of the word.
   * However the access function is the same.
   */
  ARMul_MemType_ByteLanes,

  /*
   * ARM9
   */
  ARMul_MemType_ARM9,

#ifdef NOT_YET_DEFINED
  /* Asynchronous memory interface - for future expansion */
  ARMul_MemType_Async,
  ARMul_MemType_Async16,
#endif

  ARMul_MemType_Last
} ARMul_MemType;

/* All revisions share an init function type. */

typedef ARMul_Error armul_MemInit(ARMul_State *state,
                                  ARMul_MemInterface *interf,
                                  ARMul_MemType variant,
                                  toolconf your_config);

/*
 * These are the functions that can be defined in a given memory interface
 * revision
 */

/*
 * Function to return a "time elapsed" counter in microseconds.
 * If NULL, then Unix "clock()" is used.
 */

typedef unsigned long armul_ReadClock(void *handle);

/*
 * Function to return a cycle count. Returns NULL if not available,
 * or may be assigned to be NULL.
 * On some processors cycle types may overlap (e.g. F cycles
 * and I cycles overlap on cached processors, or additional wait-state
 * cycles on a standard memory model). Hence there is a "total"
 * field, which should be filled in as the total number of cycles on the
 * model's primary clock.
 */

typedef struct {
  unsigned long Total;
  unsigned long NumNcycles,NumScycles,NumIcycles,NumCcycles,NumFcycles;
  unsigned long CoreCycles;
} ARMul_Cycles;

typedef const ARMul_Cycles *armul_ReadCycles(void *handle);

/*
 * Function to return the number of core cycles in an individual memory
 * access.
 * On cached processors where the number of core cycles in a given memory
 * cycle can vary this function is used to keep track of the number
 * of core cycles required before the next external memory access.
 */
typedef unsigned long armul_ReadDeltaCycles(void *handle);


/*
 * For synchronous memory interfaces, this function returns the duration
 * of a cycle, in tenths of a nanosecond. All calls to "MemAccess" are
 * assumed to take this long by the caller. To insert wait states, return
 * '0' from MemAccess, or use an asynchronous memory interface (not yet
 * supported)
 */
typedef unsigned long armul_GetCycleLength(void *handle);

/*
 * The MemAccess functions return the number of datums read, or -1 for an abort
 *  (N.B. 0 signifies "wait", so a successful idle cycle returns 1)
 *  - MemAccess2 can return up to two words
 *  - MemAccAsync fills in a counter for the time til the next memory cycle
 */
typedef int armul_MemAccess(void *handle,
                            ARMword address,
                            ARMword *data,
                            ARMul_acc access_type);

typedef void armul_MemBurstCount(void *handle, unsigned Count, unsigned IsWrite);

/* This call provides information on the next cycle type, which enables
 * the ARMulator to be used in logic simulation environments where
 * pipelined address are required.
 * LOCK & nTRANS are provided via callbacks - these APIs are
 * called a cycle in advance when a pipelined ARMulator is used.
 */
typedef void armul_NextCycle(void *handle,
                             ARMword address,
                             ARMul_acc access_type);

typedef int armul_MemAccess2(void *,ARMword,ARMword *,ARMword *,
                             ARMul_acc);
typedef int armul_MemAccAsync(void *,ARMword,ARMword *,ARMul_acc acc,
                              unsigned long *counter);

/*
 * When the core takes an exception - e.g. an access to the vectors in a
 * 26-bit mode, on a processor which supports exporting such exceptions
 * (i.e. an ARM8) this function is called if supplied. penc is the priority
 * encoding for the exception in question.
 */
typedef void armul_CoreException(void *handle,ARMword address,
                                 ARMword penc);

/*
 * StrongARM has a tightly coupled data-cache, where the pipeline needs
 * to know if the data cache is busy. This function provides that
 * functionality.
 */

typedef unsigned int armul_DataCacheBusy(void *handle);


struct armul_meminterface {
  void *handle;
  armul_ReadClock *read_clock;
  armul_ReadCycles *read_cycles;
  union {
    struct {
      armul_MemAccess *access;
      armul_GetCycleLength *get_cycle_length;
    } basic;
    struct {
      armul_NextCycle *next;
      armul_MemAccess *access;
      armul_GetCycleLength *get_cycle_length;
      armul_ReadDeltaCycles *delta_cycles;
    } pipeamba;
    struct {
      armul_MemAccAsync *access;
    } async;
    struct {
      armul_MemAccess *access;
      armul_GetCycleLength *get_cycle_length;
      armul_CoreException *core_exception;
      armul_MemAccess2 *access2;
    } arm8;
    struct {
      armul_MemAccess *access;
      armul_GetCycleLength *get_cycle_length;
      armul_CoreException *core_exception;
      armul_DataCacheBusy *data_cache_busy;
    } strongarm;
    struct {
      armul_MemAccess *access;
      armul_GetCycleLength *get_cycle_length;
      armul_CoreException *core_exception;
      armul_DataCacheBusy *data_cache_busy;
      armul_MemBurstCount *burst_count;             /* only for PIPELINED */
    } arm9;
  } x;
};

/*
 * Memory models are presented to the ARMulator through a stub:
 */

typedef struct {
  armul_MemInit *init;          /* memory model initialiser */
  tag_t name;                   /* memory model name */
} ARMul_MemStub;

/*
 * Given a toolconf, lookup and return the memory interface configured in
 * it, or NULL if we don't know about it. Fills in the config associated
 * with that memory too.
 */
extern armul_MemInit *ARMul_FindMemoryInterface(ARMul_State *state,
                                                tag_t name,
                                                toolconf *wrconf);

/*
 * Place the named memory model in the memory hierarchy, above the current
 * default, pointing it at the current default. Should be called before the
 * memory system is initialised, or from a top layer.
 */
extern ARMul_Error ARMul_InstallMemoryInterface(
    ARMul_State *state, unsigned at_core, tag_t name);


/*
 * A memory model may choose to implement MemAccessStats. To do so, a
 * model should use an "unknown RDI_info" callback, and intercept the
 * RDIMemory_Access, RDIMemory_Map and RDIInfo_Memory_Stats messages.
 * "armmap.c" is an example.
 */

/*
 * To implement watchpoints, call this function on every memory access,
 * and intercept RDIInfo_Points, filling in which types of watchpoint you
 * can handle. "watchpnt.c" is an example. N.B. "access" is NOT an ARMul_acc
 * type.
 */
extern void ARMul_CheckWatch(ARMul_State *state, ARMword addr, int access);

#endif

/* EOF armmem.h */
