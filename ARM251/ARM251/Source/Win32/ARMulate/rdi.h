/*
 * ARM RDI : rdi.h
 * Copyright (C) 1994-1998 ARM Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.3.6.1 $
 * Checkin $Date: 1998/09/30 14:01:41 $
 * Revising $Author: mwilliam $
 */

#ifndef rdi_h
#define rdi_h

/*
 * Currently supports RDI 1.0 and RDI 1.5
 * Future versions will support RDI 2.0 (RDI_VERSION == 200)
 */

#include "armtypes.h"
#include "rdi_stat.h"

#ifdef PICCOLO
#ifndef RDI_VERSION
#define RDI_VERSION 150
#endif
#endif

#ifndef RDI_VERSION
#define RDI_VERSION 150
#endif

#include "rdi_err.h"            /* RDIError codes */
#include "rdi_info.h"           /* RDIInfo codes */

/*
 * Other RDI values
 */

/*
 * Bits of OpenAgentProc/OpenProc type parameter
 */
#define RDIOpen_BootLevel       0
#define RDIOpen_CommsReset      1
#define RDIOpen_ByteSexMask     0xc
#define RDIOpen_ByteSexShift    2

/* Byte sex */
#define RDISex_Little           0
#define RDISex_Big              1
#define RDISex_DontCare         2

/* The different types of break- and watchpoints */
#define RDIPoint_EQ             0
#define RDIPoint_GT             1
#define RDIPoint_GE             2
#define RDIPoint_LT             3
#define RDIPoint_LE             4
#define RDIPoint_IN             5
#define RDIPoint_OUT            6
#define RDIPoint_MASK           7

#define RDIPoint_16Bit          16  /* 16-bit breakpoint                */
#define RDIPoint_Conditional    32

/* ORRed with point type in extended RDP break and watch messages       */
#define RDIPoint_Inquiry        64
#define RDIPoint_Handle         128 /* messages                         */

#define RDIWatch_ByteRead       1 /* types of data accesses to watch for*/
#define RDIWatch_HalfRead       2
#define RDIWatch_WordRead       4
#define RDIWatch_ByteWrite      8
#define RDIWatch_HalfWrite      16
#define RDIWatch_WordWrite      32


/* mask values for registers (CPURead and CPUWrite) */
/* ARM */
#define RDIReg_R15              (1L << 15)
#define RDIReg_PC               (1L << 16)
#define RDIReg_CPSR             (1L << 17)
#define RDIReg_SPSR             (1L << 18)
#define RDINumCPURegs           19
/* Magic value for the "current mode". Other mode numbers are in armtypes.h */
#define RDIMode_Curr            255

/* Piccolo */
#define RDIMode_Bank0           0
#define RDIMode_Bank1           1
#define RDIMode_Bank2           2
#define RDIMode_Bank3           3
#define RDIMode_Bank4           4
#define RDIMode_Bank5           5
#define RDIMode_Bank6           6

/* Coprocessors */
#define RDINumCPRegs            10 /* current maximum                   */


/* profile map type */
typedef struct {
  ARMword len;
  ARMword map[1];
} RDI_ProfileMap;

/* types for opaque handles - of a defined size */
/* break and watchpoint handles */
typedef unsigned32 RDI_PointHandle;
#define RDI_NoPointHandle        ((RDI_PointHandle)-1L)
/* thread handles */
typedef unsigned32 RDI_ThreadHandle;
#define RDI_NoHandle             ((RDI_ThreadHandle)-1L)

/* other RDI types. HostosInterface is defined in rdi_hif.h */
/* rdi_stat.h typedef's this to just RDI_HostosInterface */
struct RDI_HostosInterface;

/* RDI_DbgState is to be defined by the Debug Controller */
typedef struct RDI_OpaqueDbgStateStr RDI_DbgState;

/* Types used by RDI_AddConfig */
typedef enum {
    RDI_ConfigCPU,
    RDI_ConfigSystem
} RDI_ConfigAspect;

typedef enum {
    RDI_MatchAny,
    RDI_MatchExactly,
    RDI_MatchNoEarlier
} RDI_ConfigMatchType;

/* A type of a function to fill a buffer, used in RDI_LoadAgent */
typedef struct RDI_GetBufferArgStr RDI_GetBufferArg;
typedef char *RDI_GetBufferProc(RDI_GetBufferArg *getbarg, unsigned32 *sizep);

/* A function that can be registered with the Debug Target to allow it
 * to report errors with the underlying transport service back to the
 * Debug Controller. This gives the Debug Controller a chance to
 * intervene before an RDI error is reported. (For example, to
 * completely reset and start again, under user control.)  See
 * rdi_info.h - RDIInfo_AddTimeoutFunc/RDIInfo_RemTimeoutFunc.  The
 * values for errtyp and the return value are (currently)
 * Target-specific.
 */
typedef int RDI_OnErrorProc(const void *device, int errtyp, void *data);

/* Type for list of names returned from RDI_CPUNames and RDI_DriverNames */
typedef struct {
    int itemmax;
    char const * const *names;
} RDI_NameList;

/* The actual RDI implementations are in seperate headers */
#if RDI_VERSION == 100
#  include "rdi100.h"
#else
#  include "rdi150.h"
#endif

#endif /* rdi_h */

/* EOF rdi.h */
