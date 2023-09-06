/* tracer.h -- data structure for tracer module
 * Copyright (c) 1996 Advanced RISC Machines Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.11 $
 * Checkin $Date: 1998/06/26 12:24:04 $
 * Revising $Author: plg $
 */

#ifndef tracer_h
#define tracer_h

#include "host.h"
#include "armdefs.h"

#ifdef PIPELINED
typedef struct {
  unsigned32 addr;
  ARMul_acc acc;
  int calls;                    /* number of calls to NextCycle */ 
} Trace_Pipeline;
#endif
      
#define TRACE_INSTR      0x00000001
#define TRACE_MEM        0x00000002
#define TRACE_IDLE       0x00000004
#define TRACE_RANGE      0x00000008
#define TRACE_PIPE       0x00000010     /* use pclose to close */
#define TRACE_TEXT       0x00000020     /* stream is plain text */
#define TRACE_EVENT      0x00000040
#define TRACE_DISASS     0x00000080     /* disassemble instruction stream */
#define TRACE_BUS        0x00000100     /* trace at the bus/core */
#define TRACE_NONACC     0x00000200     /* trace non-accounted accesses */
#define TRACE_WAITS      0x00000400     /* show wait states returned */
#define TRACE_STARTON    0x80000000     /* tracing always on */
#define TRACE_VERBOSE    0x40000000

/* Functions for standard "dispatch" */
typedef int Tracer_PrintfProc(void *handle, const char *format, ...);
typedef int Tracer_PutSProc(const char *string, void *handle);
typedef int Tracer_PutCProc(char c, void *handle);
typedef int Tracer_WriteProc(void *packet, int size, int n, void *handle);
typedef int Tracer_CloseProc(void *handle);
typedef int Tracer_FlushProc(void *handle);

typedef struct {
  ARMul_State *state;
  unsigned int not_tracing;     /* zero when RDILog_Trace is set */
  unsigned int trace_opened;    /* set to one once Tracer_Open called */
  void *config;
  struct {
    void *handle;               /* usually a FILE * */
    Tracer_PrintfProc *printf;  /* fprintf */
    Tracer_PutSProc *puts;      /* fputs */
    Tracer_PutCProc *putc;      /* fputc */
    Tracer_WriteProc *write;    /* fwrite */
    Tracer_CloseProc *close;    /* fclose */
    Tracer_FlushProc *flush;    /* fflush */
  } output;
  unsigned32 prop;
  enum { TRACE_ARM, TRACE_THUMB, TRACE_CHECK } thumb; /* whether we think we are thumb at the moment */
  ARMul_MemInterface real;
  ARMword range_lo,range_hi;
  unsigned long sample,sample_base;
  unsigned int event_mask,event_set; /* masks for events */
  void *hourglass,*trace_event; /* handles for ARMulator callbacks */
#ifdef PIPELINED
  Trace_Pipeline current, advance;
#endif
  unsigned32 memfeatures;       /* features when displaying mem cycles */
  unsigned32 prev_instr;        /* Thumb 2-instr BL disassembly */
} Trace_State;

#define TRACE_MEMFEATURE_AMBA          0x1    /* use AMBA mnemonics */
#define TRACE_MEMFEATURE_PIPELINED     0x2    /* check next cycle info */

typedef struct {
  enum {
    Trace_Instr,                /* instruction execution */
    Trace_MemAccess,            /* memory cycles, includes idles */
    Trace_Event                 /* other misc events */
    } type;
  union {
    struct {
      unsigned32 instr;
      unsigned32 pc;
      unsigned8 executed;       /* 1 if executed, 0 otherwise */
      unsigned8 thumb;          /* 1 if a Thumb instruction, 0 otherwise */
    } instr;
    struct {
      ARMul_acc acc;
      unsigned32 addr;
      unsigned32 word1,word2;
      int rv;                   /* return value from mem_access call */
#ifdef PIPELINED
      Trace_Pipeline predict;   /* for checking against actual cycle */
#endif
    } mem_access;
    struct {
      unsigned32 addr,addr2;
      unsigned int type;        /* see note in armdefs.h */
    } event;
  } u;
} Trace_Packet;

/* If compiled with EXTERNAL_DISPATCH defined, the following functions
 * must be defined by some external module
 */
#if defined(EXTERNAL_DISPATCH)

extern unsigned Tracer_Open(Trace_State *ts);
/* Called when the tracer stream is openned */
extern void Tracer_Dispatch(Trace_State *ts,Trace_Packet *packet);
/* Called for each object to be traced */
extern void Tracer_Close(Trace_State *ts);
/* Called on exit, when the trace file is closed */
extern void Tracer_Flush(Trace_State *ts);
/* Called when tracing is disabled - like fflush() */

#endif

#endif
