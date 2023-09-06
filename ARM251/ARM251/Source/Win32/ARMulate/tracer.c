/* tracer.c - module to trace memory accesses and instruction execution
 *            in ARMulator.
 * Copyright (c) 1996-1998 Advanced RISC Machines Limited. All Rights Reserved.
 * Copyright (c) 1998 ARM Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.54.2.1 $
 * Checkin $Date: 1998/08/13 10:47:50 $
 * Revising $Author: mwilliam $
 */

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>       /* for strcpy */
#include "armdefs.h"
#include "armcnf.h"
#include "tracer.h"
#include "disass.h"
#ifdef PicAlpha
#include "picdis.h"
#endif

#ifdef SOCKETS
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

static struct {
  tag_t option;
  unsigned long flag;
  char *print;
} TraceOption[] = {
  ARMulCnf_TraceInstructions, TRACE_INSTR,    "Instructions",
  ARMulCnf_TraceMemory,       TRACE_MEM,      "Memory accesses",
  ARMulCnf_TraceIdle,         TRACE_IDLE,     "Idle cycles",
  ARMulCnf_TraceEvents,       TRACE_EVENT,    "Events",
  ARMulCnf_Disassemble,       TRACE_DISASS,   "Disassemble",
  ARMulCnf_StartOn,           TRACE_STARTON,  "Start on",
  ARMulCnf_TraceBus,          TRACE_BUS,      "Trace bus",
  ARMulCnf_TraceNonAcc,       TRACE_NONACC,   "Non-accounted",
  ARMulCnf_TraceWaits,        TRACE_WAITS,    "Waits",
  NULL, 0, NULL
};

/* other option fields */


#define ModelName (tag_t)"Tracer"

/*
 * The trace function generates a stream of packets, which are either
 * logged into a file or sent to a socket.
 */

#ifndef EXTERNAL_DISPATCH       /* if you want something better... */
static char *cb_proc(
    dis_cb_type t, int32 offset, unsigned32 addr, int width, void *arg,
    char *buf)
{
  IGNORE(t); IGNORE(offset); IGNORE(addr); IGNORE(width); IGNORE(arg);
  return buf;
}

static int Tracer_Acc(Trace_State *ts, ARMul_acc acc, unsigned32 addr)
{
  void *f=ts->output.handle;
  Tracer_PrintfProc *tprintf = ts->output.printf;
  Tracer_PutSProc *tputs = ts->output.puts;
  Tracer_PutCProc *tputc = ts->output.putc;
  int width=8;

  tputc('M',f);
  if (acc == acc_Icycle) {
    /* This is used for 2 purposes:
     *   - SDT 7TDMI ARMulator generates these for 'internal cycles'
     *     Not bus idles because needed to decode merged IS cycles
     *     However SDT 7TDMI does not provide correct address.
     *   - AMBA real bus idle cycles
     */
    tputc('I',f); /* has no relevant address or data */
  } else {
    if (acc_MREQ(acc)) {
      /* These casts are needed to remove warnings under MSVC++ */
      tputc((char)(acc_SEQ(acc) ? 'S' : 'N'), f);
    } else {
      if (ts->memfeatures & TRACE_MEMFEATURE_AMBA) {
        tputc((char)(acc_SEQ(acc) ? 'r' /* reserved */ : 'A'), f);
      } else {
        tputc((char)(acc_SEQ(acc) ? 'C' : 'I'),f);
      }
    }
    tputc((char)(acc_READ(acc) ? 'R' : 'W'), f);
    switch (acc_WIDTH(acc)) {
    case BITS_8:  tputc('1',f); width=2; break;
    case BITS_16: tputc('2',f); width=4; break;
    case BITS_32: tputc('4',f); break;
    case BITS_64: tputc('8',f); break;
    }
    tputc((char)(acc_OPC(acc) ? 'O' : '_'), f);
    tputc((char)(acc_LOCK(acc) ? 'L' : '_'), f);
    tputc((char)(acc_SPEC(acc) ? 'S' : '_'), f);
    if (ts->memfeatures & TRACE_MEMFEATURE_AMBA) {
      switch (acc_BURST_TYPE(acc)) {
      case acc_burst_none:  tputc('_',f); break;
      case acc_burst_4word: tputc('4',f); break;
      case acc_burst_8word: tputc('8',f); break;
      default:              tputc('r',f); break; /* reserved */
      }
    }
    tprintf(f, " %08lX", addr);
  }
  return width;
}

static void Tracer_Dispatch(Trace_State *ts,Trace_Packet *packet)
{
  void *f=ts->output.handle;
  if (ts->prop & TRACE_TEXT) {
    Tracer_PrintfProc *tprintf = ts->output.printf;
    Tracer_PutSProc *tputs = ts->output.puts;
    Tracer_PutCProc *tputc = ts->output.putc;
    switch (packet->type) {
    case Trace_Instr: {
      char buffer[256];
      ARMword instr=packet->u.instr.instr,pc=packet->u.instr.pc;
      unsigned executed=packet->u.instr.executed;

      tprintf(f,"I%c %08lX ", executed ? 'T' : 'S', pc);
      if (packet->u.instr.thumb)
        tprintf(f, "%04lx", instr);
      else
        tprintf(f, "%08lx", instr);

      if (ts->prop & TRACE_DISASS) {
        if (packet->u.instr.thumb) {
          /* Complications for Thumb 2-instruction BL...
           * ARM disass API (current instr, next instr) is not ideal
           * for disassembling instructions as they are executed, where
           * want to specify current instr & previous instr.
           */
          if (instr>>11==0x1E) {
            strcpy(buffer, "(1st instr of BL pair)");
          } else if (instr>>11==0x1F) {  /* second instruction */
            /* if prev_instr is invalid, disass_16 shows "???" */
            disass_16(ts->prev_instr,instr,pc-2,buffer,NULL,cb_proc);
          } else {
            disass_16(instr,0,pc,buffer,NULL,cb_proc);
          }
          ts->prev_instr=instr;
        } else {
          disass(instr,pc,buffer,NULL,cb_proc);
        }
        if (!executed) {
          char *p;
          for (p=buffer; *p; p++)
            if (isupper(*p)) *p=tolower(*p);
        }
        tputc(' ', f);
        tputs(buffer, f);
      }
      tputc('\n', f);
    }
      break;

    case Trace_MemAccess: {
      ARMul_acc acc=packet->u.mem_access.acc;
      unsigned32 addr=packet->u.mem_access.addr;
      int rv=packet->u.mem_access.rv;
      int width;

      /* all mem_access packets sent are to be displayed */
      width=Tracer_Acc(ts, acc, addr);
      if (acc_CYCLE(acc) != acc_typeI /* includes acc_Icycle */) {
        if (rv != 0) {
          tprintf(f," %0*.*lX", width, width, packet->u.mem_access.word1);
          if (acc_WIDTH(acc)==BITS_64)
            tprintf(f," %08lX", packet->u.mem_access.word2);
        }
      }
      if (rv < 0) {
        tputs(" (abort)", f);
      } else if (rv == 0) {
        tputs(" (wait)", f);
      }
      if (acc_ACCOUNT(acc)) {
#ifdef PIPELINED
        if (ts->memfeatures & TRACE_MEMFEATURE_PIPELINED) {
          /* display pipeline prediction errors */
          Trace_Pipeline *predict=&packet->u.mem_access.predict;
          if (predict->calls != -1) { /* not start-up */
            if (predict->calls != 1) {
              tprintf(f," pipe calls %d", predict->calls);
            } else if (acc != predict->acc ||
                       /* don't check addr on bus idle cycles
                          TBD: fix armulator */
                       (acc != acc_Icycle && addr != predict->addr)) {
              tprintf(f," pipe err ");
              Tracer_Acc(ts, predict->acc, predict->addr);
            }
          }
        }
#endif
      } else {
        /* display non-accounted accesses */
        if (acc_MREQ(acc))
          tprintf(f, acc_READ(acc) ? " (peek)" : " (poke)");
      }
      tputc('\n', f);
    }
      break;

    case Trace_Event:
#ifdef PicAlpha
      if (packet->u.event.type >= PiccoloEvent_Instr &&
          packet->u.event.type <= PiccoloEvent_Stalled) {
        ARMword pc = packet->u.event.addr;
        ARMword instr = packet->u.event.addr2;
        char buffer[256];
        disass_pic(instr, pc, buffer);
        switch (packet->u.event.type) {
        case PiccoloEvent_Instr:
          tprintf(f, "PI %08lX %08lX %s\n", pc, instr, buffer);
          break;

        case PiccoloEvent_Skip:
          tprintf(f, "PM %08lX %08lX %s (multi-cycle)\n", pc, instr, buffer);
          break;

        case PiccoloEvent_Stalled:
          tprintf(f, "PS %08lX %08lX %s (stalled)\n", pc, instr, buffer);
          break;
        }

      } else if (packet->u.event.type == PiccoloEvent_ROBLoad) {
        ARMword reg = packet->u.event.addr2 & 0xf;
        tprintf(f, "PR %08lX -> %c%c\n", packet->u.event.addr,
                "AXYZ"[reg/4], (char)('0'+(reg%4)));

      } else if (packet->u.event.type == PiccoloEvent_FIFOOut) {
        tprintf(f, "PF ARM <- %08lx [%d]\n", packet->u.event.addr,
                (unsigned)packet->u.event.addr2);
      } else
#endif
      tprintf(f,"E %08lX %08lX %x\n",
              packet->u.event.addr,packet->u.event.addr2,
              packet->u.event.type);
      break;
    }
  } else {
    Tracer_WriteProc *twrite = ts->output.write;
    twrite(packet,sizeof(*packet),1,f);
  }
}

/*
 * Veneers onto DebugPrint
 */
static int DebugPutS(const char *s, void *handle)
{
  RDI_HostosInterface *hostif = (RDI_HostosInterface *)handle;
  ARMul_DebugPrint_i(hostif, "%s", s);
  return 0;
}

static int DebugPutC(char c, void *handle)
{
  RDI_HostosInterface *hostif = (RDI_HostosInterface *)handle;
  ARMul_DebugPrint_i(hostif, "%c", c);
  return 0;
}
   
/*
 * Open the trace stream.
 * Under Unix, possibly try doing a socket instead of a plain
 * file. Returns 1 on failure.
 */

static int Banner(Trace_State *ts)
{    
  time_t now=time(NULL);
  void *f=ts->output.handle;
  Tracer_PrintfProc *tprintf = ts->output.printf;
  Tracer_PutSProc *tputs = ts->output.puts;
  Tracer_PutCProc *tputc = ts->output.putc;

  tprintf(f, "Date: %s", ctime(&now));
  tprintf(f, "Source: Armul\n");
  tputs("Options: ", f);
  if (ts->prop & TRACE_INSTR)  tputs("Trace Instructions  ", f);
  if (ts->prop & TRACE_DISASS) tputs("(Disassemble)  ", f);
  if (ts->prop & TRACE_MEM) {
    char nextch = '(';
    tputs("Trace Memory Cycles  ", f);
    if (ts->prop & TRACE_IDLE) {
      tputc(nextch, f);
      tputs("Idles", f);
      nextch = ',';
    }
    if (ts->prop & TRACE_NONACC) {
      tputc(nextch, f);
      tputs("Non-accounted", f);
      nextch = ',';
    }
    if (ts->prop & TRACE_WAITS) {
      tputc(nextch, f);
      tputs("Waits", f);
      nextch = ',';
    }
    if (nextch != '(')
      tputs(")  ", f);
  }
  if (ts->prop & TRACE_EVENT)  tputs("Trace Events  ", f);
  tputc('\n', f);  
  if (ts->prop & TRACE_RANGE)
    tprintf(f, "Range: 0x%08lX -> 0x%08lX\n",
            ts->range_lo, ts->range_hi);
  if (ts->sample_base)
    tprintf(f, "Sample: %ld\n", ts->sample_base);
  tputc('\n', f);

  return 0;
}

static unsigned Tracer_Open(Trace_State *ts)
{
  const char *option;
  unsigned verbose;

  verbose=ts->prop & TRACE_VERBOSE;

  if (ToolConf_DLookupBool(ts->config, ARMulCnf_RDILog, FALSE)) {
    /* output to the rdi_log window */
    /* talk directly, rather than via DebugPrint, as it should be faster,
     * and DebugPrint checks rdi_log bit 1
     */
    ts->output.handle = (void *)ARMul_HostIf(ts->state);
    ts->output.printf = (Tracer_PrintfProc *)ARMul_DebugPrint_i;
    ts->output.puts = DebugPutS;
    ts->output.putc = DebugPutC;
    ts->prop|=TRACE_TEXT;

    return Banner(ts);
  }

  option=ToolConf_Lookup(ts->config, ARMulCnf_File);
  if (option!=NULL) {
    FILE *f = fopen(option,"w");
    if (f == NULL) {
      fprintf(stderr,"Could not open trace file '%s' - abandoning trace.\n",
              option);
      return 1;
    }
    ts->output.handle = f;
    ts->output.printf = (Tracer_PrintfProc *)fprintf;
    ts->output.puts = (Tracer_PutSProc *)fputs;
    ts->output.putc = (Tracer_PutCProc *)fputc;
    ts->output.close = (Tracer_CloseProc *)fclose;
    ts->output.flush = (Tracer_FlushProc *)fflush;

    ts->prop|=TRACE_TEXT;

    return Banner(ts);
  }

  option=ToolConf_Lookup(ts->config, ARMulCnf_BinFile);
  if (option!=NULL) {
    FILE *f = fopen(option,"wb");
    if (f == NULL) {
      fprintf(stderr,"Could not open trace file '%s' - abandoning trace.\n",
              option);
      return 1;
    }
    ts->output.handle = f;
    ts->output.write = (Tracer_WriteProc *)fwrite;
    ts->output.close = (Tracer_CloseProc *)fclose;
    ts->output.flush = (Tracer_FlushProc *)fflush;
    return 0;
  }

#ifdef SOCKETS
  option=ToolConf_Lookup(ts->config, ARMulCnf_Port);
  if (option) {
    long port;
      
    port=strtol(option,NULL,0);

    option=ToolConf_Lookup(ts->config, ARMulCnf_Host);

    if (option) {
      struct hostent *host;
      struct sockaddr_in con;
      int sock;
      
      host=gethostbyname(option);

      if (host==NULL) {
        ARMul_ConsolePrint(ts->state,"Could not resolve host '%s'\n",
                           option);
        return 1;
      }
        
      if (verbose)
        ARMul_ConsolePrint(ts->state,"Tracing to %s:%d\n",option,port);

      sock=socket(AF_INET,SOCK_STREAM,0);
      if (sock==-1) {
        ARMul_ConsolePrint(ts->state,"Could not open trace port\n");
        return 1;
      }
        
      memset(&con,'\0',sizeof(con));
      con.sin_family=AF_INET;
      memcpy(&con.sin_addr,host->h_addr,sizeof(con.sin_addr));
      con.sin_port=htons(port & 0xffff);
        
      if (connect(sock,(struct sockaddr *)&con, sizeof(con))!=0) {
        close(sock);
        ARMul_ConsolePrint(ts->state,"Failed to open socket\n");
        return 1;
      }

      ts->output.handle=(void *)fdopen(sock,"wb");
      if (ts->output.handle!=NULL) return 0;
      ts->output.write = (Tracer_WriteProc *)fwrite;
      ts->output.close = (Tracer_CloseProc *)fclose;
      ts->output.flush = (Tracer_FlushProc *)fflush;

      ARMul_ConsolePrint(ts->state,"Failed to fdopen socket.\n");
      return 1;
    }

    ARMul_ConsolePrint(ts->state,"PORT configured with no HOST.\n");
    return 1;
  }
#endif

#ifdef PIPE
  option=ToolConf_Lookup(ts->config, ARMulCnf_Pipe);
  if (option) {
    ts->prop|=TRACE_PIPE;
    ts->output.handle=(void *)popen(option,"w");
    if (ts->output.handle == NULL) {
      ARMul_ConsolePrint(state,
                         "Could not open pipe to '%s' - abandoning trace.\n",
                         option);
      return 0;
    }
    ts->output.write = (Tracer_WriteProc *)fwrite;
    ts->output.close = (Tracer_CloseProc *)pclose;
    ts->output.flush = (Tracer_FlushProc *)fflush;
    return 1;
  }
#endif
  
  fprintf(stderr,"No trace file configured - abandoning trace.\n");
  return 1;
}

static void Tracer_Close(Trace_State *ts)
{
  if (ts->output.close)
    ts->output.close(ts->output.handle);
}

static void Tracer_Flush(Trace_State *ts)
{
  if (ts->output.flush)
    ts->output.flush(ts->output.handle);
}

#endif                          /* EXTERNAL_DISPATCH */

/*
 * The function is called from the ARMulator when rdi_log & 16 is true.
 * It is used to generate an executed instruction stream trace.
 */
static void trace_event(void *handle, unsigned int event,
                        ARMword addr1, ARMword addr2)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;

  /* Mask events */
  if ((event & ts->event_mask)!=ts->event_set) return;

  if ((ts->prop & TRACE_RANGE) && (addr1<ts->range_lo ||
                                   (addr1>=ts->range_hi && ts->range_hi!=0)))
    /* range test fails */
    return;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return;
    ts->sample=ts->sample_base-1;
  }

  packet.type=Trace_Event;

  packet.u.event.addr=addr1;
  packet.u.event.addr2=addr2;
  packet.u.event.type=event;

  Tracer_Dispatch(ts,&packet);
}

/*
 * this function is called for every instruction
 */

static void trace(void *handle, ARMword pc, ARMword instr)
{
  Trace_State *ts=(Trace_State *)handle;
  int temp;
  Trace_Packet packet;

  if ((ts->prop & TRACE_RANGE) && (pc<ts->range_lo ||
                                   (pc>=ts->range_hi && ts->range_hi!=0)))
    /* range test fails */
    return;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return;
    ts->sample=ts->sample_base-1;
  }

  temp=TOPBITS(28);
  packet.type=Trace_Instr;

  packet.u.instr.pc=pc;
  packet.u.instr.instr=instr;
  switch (ts->thumb) {
  case TRACE_THUMB:
    if (instr >= 0x10000) goto check;
    packet.u.instr.thumb=TRUE;
    if ((instr >> 7) == 0x8e)   /* BX */
      ts->thumb = TRACE_CHECK;
    break;
  case TRACE_ARM:
    if (instr < 0x10000) goto check;
    if ((instr & 0x0c00f000) == 0x0000f000)
      /* instruction is a candidate for in instruction set changing instruction */
      ts->thumb = TRACE_CHECK;
    packet.u.instr.thumb=FALSE;
    break;
  case TRACE_CHECK: check: 
    if (ARMul_GetCPSR(ts->state) & ARM_PSR_T) {
      ts->thumb = TRACE_THUMB;
      packet.u.instr.thumb=TRUE;
    } else {
      ts->thumb = TRACE_ARM;
      packet.u.instr.thumb=FALSE;
    }
    break;
  }
  packet.u.instr.executed = ARMul_CondCheckInstr(ts->state,instr);

  Tracer_Dispatch(ts,&packet);
}


static void TracerExit(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;

  if (ts->trace_opened) {
    Tracer_Close(ts);
    ts->trace_opened=FALSE;
  }

  free(ts);
}

/*
 * These are the veneer memory functions that intercept core memory
 * activity and report it to the trace stream.
 * NB: duplicate any changes in all equivalent functions:
 *  - MemAccess
 *  - MemAccess_PipelinedAMBA
 *  - MemAccess2
 */
static int MemAccess(void *handle,ARMword addr,ARMword *word,
                     ARMul_acc acc)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;
  int rv;

  rv=ts->real.x.basic.access(ts->real.handle,addr,word,acc); 

  if (ts->not_tracing) return rv;

  if ((ts->prop & TRACE_RANGE) && (addr<ts->range_lo ||
                                   (addr>=ts->range_hi && ts->range_hi!=0)))
    /* range test fails */
    return rv;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return rv;
    ts->sample=ts->sample_base-1;
  }

  if ((ts->prop & TRACE_MEM) &&
      ((ts->prop & TRACE_WAITS) || (rv != 0)) &&
      ((ts->prop & TRACE_NONACC) || acc_ACCOUNT(acc)) &&
      ((ts->prop & TRACE_IDLE) || acc != acc_Icycle)) {
    packet.type=Trace_MemAccess;
    packet.u.mem_access.acc=acc;
    packet.u.mem_access.addr=addr;
    packet.u.mem_access.word1=(acc_MREQ(acc) ? *word : 0);
    packet.u.mem_access.rv=rv;
    Tracer_Dispatch(ts,&packet);
  }
  return rv;
}

#ifdef PIPELINED
static int MemAccess_PipelinedAMBA(void *handle,ARMword addr,ARMword *word,
                     ARMul_acc acc)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;
  int rv;

  rv=ts->real.x.pipeamba.access(ts->real.handle,addr,word,acc); 

  if (ts->not_tracing) return rv;

  if ((ts->prop & TRACE_RANGE) && (addr<ts->range_lo ||
                                   (addr>=ts->range_hi && ts->range_hi!=0)))
    /* range test fails */
    return rv;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return rv;
    ts->sample=ts->sample_base-1;
  }

  if ((ts->prop & TRACE_MEM) &&
      ((ts->prop & TRACE_WAITS) || (rv != 0)) &&
      ((ts->prop & TRACE_NONACC) || acc_ACCOUNT(acc)) &&
      ((ts->prop & TRACE_IDLE) || acc != acc_Icycle)) {
    packet.type=Trace_MemAccess;
    packet.u.mem_access.acc=acc;
    packet.u.mem_access.addr=addr;
    packet.u.mem_access.predict=ts->current;  /* PIPELINED only */
    packet.u.mem_access.word1=*word;
    packet.u.mem_access.rv=rv;
    Tracer_Dispatch(ts,&packet);
  }

  /* Check the predicted NextCycle info...
   * First, the NextCycle data is temporarily kept in ts->advance
   * (we also count the number of NextCycle calls).
   * Then the prediction information is delayed by one cycle
   * so that it can be compared with the actual cycle that happens.
   * NextCycle info must be supplied for I-cycles and memory accesses.
   */
  if (acc_ACCOUNT(acc) && (rv != 0)) {
    ts->current=ts->advance;  /* delay next cycle information by 1 cycle */
    ts->advance.calls=0;  /* reset counter */
  }

  return rv;
}
#endif

static int MemAccess2(void *handle,ARMword addr,
                      ARMword *word1,ARMword *word2,
                      ARMul_acc acc)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;
  int rv;

  rv=ts->real.x.arm8.access2(ts->real.handle,addr,word1,word2,acc);

  if (ts->not_tracing) return rv;

  if (!acc_ACCOUNT(acc)) return rv;

  if ((ts->prop & TRACE_RANGE) && (addr<ts->range_lo || /* range test */
                                   (addr>=ts->range_hi && ts->range_hi!=0)))
    return rv;

  if (ts->sample_base) {
    if (ts->sample--)           /* not this one */
      return rv;
    ts->sample=ts->sample_base-1;
  }
  if ((ts->prop & TRACE_MEM) &&
      ((ts->prop & TRACE_WAITS) || (rv != 0)) &&
      ((ts->prop & TRACE_NONACC) || acc_ACCOUNT(acc)) &&
      ((ts->prop & TRACE_IDLE) || acc != acc_Icycle)) {
    packet.type=Trace_MemAccess;
    packet.u.mem_access.acc=acc;
    packet.u.mem_access.addr=addr;
    packet.u.mem_access.word1=*word1;
    packet.u.mem_access.word2=*word2;
    packet.u.mem_access.rv=rv;
    Tracer_Dispatch(ts,&packet);
  }

  return rv;
}

/* This function only called by CoVerification version of ARM9 core */
static void BurstCount(void *handle, unsigned Count, unsigned IsWrite)
{
  Trace_State *ts=(Trace_State *)handle;
  
#if 0
  ts->real.x.basic.burst_count(ts->real.handle, Count, IsWrite);
#endif
}

#ifdef PIPELINED
static void NextCycle_PipelinedAMBA(void *handle,ARMword addr,ARMul_acc acc)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;

  ts->real.x.pipeamba.next(ts->real.handle,addr,acc); 

/* NextCycle info before non-accounted accesses should
 *   - ideally not happen
 *   - be marked as non-accounted if it does happen
 * This enables the NextCycle info from the previous real
 * access to relate to the next real access regardless of
 * intervening non-accounted accesses.
 */
  if (acc_ACCOUNT(acc)) {
    /* store next cycle information */
    ts->advance.addr=addr;
    ts->advance.acc=acc;
    ts->advance.calls++;  /* count the number of NextCycle calls */
  }
}

static unsigned long DeltaCycles_PipelinedAMBA(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  Trace_Packet packet;

  return (ts->real.x.pipeamba.delta_cycles(ts->real.handle));

}
#endif


/* Dummy veneer functions */
static unsigned int DataCacheBusy(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  return ts->real.x.strongarm.data_cache_busy(ts->real.handle);
}
static void CoreException(void *handle,ARMword address,ARMword penc)
{
  Trace_State *ts=(Trace_State *)handle;
  ts->real.x.arm8.core_exception(ts->real.handle,address,penc);
}
static unsigned long GetCycleLength(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  return ts->real.x.basic.get_cycle_length(ts->real.handle);
}
static unsigned long ReadClock(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  return ts->real.read_clock(ts->real.handle);
}
static const ARMul_Cycles *ReadCycles(void *handle)
{
  Trace_State *ts=(Trace_State *)handle;
  return ts->real.read_cycles(ts->real.handle);
}

/* Function called when RDI_LOG changes, so we can control logging
 * memory accesses */

static void trace_on(Trace_State *ts)
{
  unsigned32 prop=ts->prop;
  ARMul_State *state=ts->state;
  
  ts->not_tracing=FALSE;
  
  if (!ts->trace_opened) {
    /* open the tracing file */
    if (Tracer_Open(ts)) return;
    ts->trace_opened=TRUE;
  } 

#ifdef PIPELINED
  ts->current.calls=-1; /* rogue value    */
  ts->advance.calls=0;  /* 0 calls so far */
#endif

  /* install instruction and event tracing functions */
  if (prop & TRACE_INSTR)
    ts->hourglass=ARMul_InstallHourglass(state,trace,ts);
  if (prop & TRACE_EVENT)
    ts->trace_event=ARMul_InstallEventUpcall(state,trace_event,ts);
}

static void trace_off(Trace_State *ts)
{
  unsigned32 prop=ts->prop;
  ARMul_State *state=ts->state;
  
  ts->not_tracing=TRUE;
  /* remove instruction and event tracing functions */
  if (prop & TRACE_INSTR) ARMul_RemoveHourglass(state,ts->hourglass);
  if (prop & TRACE_EVENT) ARMul_RemoveEventUpcall(state,ts->trace_event);
  ts->hourglass=ts->trace_event=NULL;
  Tracer_Flush(ts);
}

static int RDI_info(void *handle,unsigned type,ARMword *arg1,ARMword *arg2)
{
  IGNORE(arg2);
  if (type==RDIInfo_SetLog) {
    Trace_State *ts=(Trace_State *)handle;
    int new=(int)*arg1;

    if (new & RDILog_Trace) {
      if (ts->not_tracing)      /* tracing enable */
        trace_on(ts);
    } else {
      if (!ts->not_tracing)     /* tracing disable */
        trace_off(ts);
    }
  }

  return RDIError_UnimplementedMessage;
}

static ARMword Properties(ARMul_State *state, toolconf config)
{
  unsigned int i;
  ARMword prop;
  unsigned verbose;

  verbose=ToolConf_DLookupBool(config, ARMulCnf_Verbose, FALSE);

  prop = verbose ? TRACE_VERBOSE : 0;

  for (i=0;TraceOption[i].option!=NULL;i++) {
    const char *option=ToolConf_Lookup(config,TraceOption[i].option);
    if (option) {
      if (verbose)
        ARMul_ConsolePrint(state,"%s%s",
                           prop ? ", " : "Tracing:                 ",
                           TraceOption[i].print);      
      prop=ToolConf_AddFlag(option,prop,TraceOption[i].flag,TRUE);
    }
  }

  return prop;
}

static ARMul_Error CommonInit(
    ARMul_State *state, Trace_State *ts, toolconf config)
{
  const char *option;
  unsigned verbose = ts->prop & TRACE_VERBOSE;

  ARMul_PrettyPrint(state,", Tracer");

  ts->state=state;
  ts->not_tracing=TRUE;

  ts->output.printf = NULL;
  ts->output.putc = NULL;
  ts->output.puts = NULL;
  ts->output.write = NULL;
  ts->output.close = NULL;
  ts->output.flush = NULL;
  ts->output.handle = NULL;

  ts->trace_opened=FALSE;
  ts->config=config;

  ts->thumb = TRACE_CHECK;

  if (ts->config==NULL) {
    free(ts);
    return ARMul_RaiseError(state,ARMulErr_NoConfigFor,ModelName);
  }

  if (ts->prop & TRACE_EVENT) {
    const char *option=ToolConf_Lookup(ts->config, ARMulCnf_EventMask);
    if (option) {
      char *p;
      ts->event_mask = strtoul(option, &p, 0);
      ts->event_set = p ? strtoul(p+1, NULL, 0) : ts->event_mask;
      if (verbose)
        ARMul_ConsolePrint(state," Mask 0x%08x-0x%08x",
                           ts->event_mask,ts->event_set);
    } else {
      option=ToolConf_Lookup(config,ARMulCnf_Event);
      if (option) {
        ts->event_mask=(unsigned int)~0;
        ts->event_set=strtoul(option,NULL,0);
        if (verbose)
          ARMul_ConsolePrint(state," 0x%08x",ts->event_set);
      } else
        ts->event_mask = ts->event_set = 0;
    }
  }

  if (ts->prop & TRACE_STARTON) trace_on(ts);

  option=ToolConf_Lookup(ts->config,ARMulCnf_Range);
  if (option) {
    if (sscanf(option,"%li,%li",&ts->range_lo,&ts->range_hi)==2) {
      ts->prop|=TRACE_RANGE;
      if (verbose) ARMul_ConsolePrint(state," Range %08x->%08x",
                                      ts->range_lo,ts->range_hi);
    } else
      ARMul_ConsolePrint(state,"TRACER: Did not understand range '%s'\n",
                         option);
  }

  option=ToolConf_Lookup(ts->config,ARMulCnf_Sample);
  if (option) {
    ts->sample_base=strtoul(option,NULL,0);
    ts->sample=0;
    if (verbose) ARMul_ConsolePrint(state," Sample rate %d",ts->sample_base);
  } else {
    ts->sample_base=0;
  }

  if (ts->prop!=0 && verbose) ARMul_ConsolePrint(state,"\n");

  /* Install rdi_log handler */
  ARMul_InstallUnkRDIInfoHandler(state,RDI_info,ts);
  ARMul_InstallExitHandler(state,TracerExit,ts);

  return ARMulErr_NoError;
}

static ARMul_Error MemoryInit(ARMul_State *state,ARMul_MemInterface *interf,
                              ARMul_MemType type,toolconf config)
{
  /*
   * Tracer installed as a memory model - can trace memory accesses
   * if so configured.
   */
  Trace_State *ts;
  armul_MemInit *stub=NULL;
  toolconf child;
  ARMul_Error err;
  tag_t tag;

  tag = (tag_t)ToolConf_Lookup(config,ARMulCnf_Memory);

  if (tag)
    stub=ARMul_FindMemoryInterface(state,tag,&child);
  if (tag==NULL || stub==NULL || stub==MemoryInit)
    return ARMul_RaiseError(state,ARMulErr_NoMemoryChild,ModelName);

  ts=(Trace_State *)malloc(sizeof(Trace_State));
  if (ts==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  ts->prop=Properties(state,config);
  err=CommonInit(state,ts,config);
  if (err!=ARMulErr_NoError) return err;

  if (child==NULL) child=ts->config;
  else {                        /* pass on clock speed */
    const char *option = ToolConf_FlatLookup(config, ARMulCnf_MCLK);
    if (option) ToolConf_UpdateTagged(child, ARMulCnf_MCLK, option);
  }
  
  if (ts->prop & TRACE_MEM) {
    err=stub(state,&ts->real,type,child);
    if (err!=ARMulErr_NoError) {
      free(ts);
      return err;
    }
    interf->handle=(void *)ts;
    interf->read_clock=ReadClock;
    interf->read_cycles=ReadCycles;
    switch (type) {
    case ARMul_MemType_Basic: case ARMul_MemType_BasicCached:
    case ARMul_MemType_16Bit: case ARMul_MemType_16BitCached:
    case ARMul_MemType_Thumb: case ARMul_MemType_ThumbCached:
      interf->x.basic.access=MemAccess;
      interf->x.basic.get_cycle_length=GetCycleLength;
      break;
#ifdef PIPELINED
    case ARMul_MemType_PipelinedAMBA:
      interf->x.pipeamba.next=NextCycle_PipelinedAMBA;
      interf->x.pipeamba.access=MemAccess_PipelinedAMBA;
      interf->x.pipeamba.get_cycle_length=GetCycleLength;

      /* Only pass back the tracer DeltaCycles function if */
      /* the child memory interface supports it.           */ 
      if (ts->real.x.pipeamba.delta_cycles) {
        interf->x.pipeamba.delta_cycles=DeltaCycles_PipelinedAMBA;
      } else {
        interf->x.pipeamba.delta_cycles=NULL;
      }
      break;
#endif
    case ARMul_MemType_ARM8:
      interf->x.arm8.access=MemAccess;
      interf->x.arm8.get_cycle_length=GetCycleLength;
      interf->x.arm8.access2=MemAccess2;
      interf->x.arm8.core_exception=CoreException;
      break;
    case ARMul_MemType_StrongARM:
      interf->x.strongarm.access=MemAccess;
      interf->x.strongarm.get_cycle_length=GetCycleLength;
      interf->x.strongarm.core_exception=CoreException;
      interf->x.strongarm.data_cache_busy=DataCacheBusy;
      break;
    case ARMul_MemType_ARM9:
      interf->x.arm9.access=MemAccess;
      interf->x.arm9.get_cycle_length=GetCycleLength;
      interf->x.arm9.data_cache_busy=DataCacheBusy;
      interf->x.arm9.burst_count=BurstCount;
      break;
    default:
      *interf=ts->real;         /* copy real memory interface across */
      ARMul_ConsolePrint(state,"\
TRACER: Cannot trace this type of memory interface.\n");
      break;
    }
    ts->memfeatures = (type == ARMul_MemType_PipelinedAMBA) ?
      TRACE_MEMFEATURE_AMBA | TRACE_MEMFEATURE_PIPELINED : 0;
    return ARMulErr_NoError;
  } else {
    return stub(state,interf,type,child);
  }
}

const ARMul_MemStub ARMul_TracerMem = {
  MemoryInit,
  ModelName
  };

/* function that copies the configuration across */
static int CopyConfig(
    toolconf from, tag_t tag, const char *value, toolconf child, void *arg)
{
  toolconf to = (toolconf)arg;
  IGNORE(from);
  IGNORE(child);                /* ignore this - never used */
  ToolConf_UpdateTagged(to, tag, value);
  return 0;
}

static ARMul_Error ModelInit(ARMul_State *state,toolconf config)
{
  /*
   * Tracer installed as a basic model - can only trace instructions and
   * events.
   */
  Trace_State *ts;
  ARMword prop;

  prop = Properties(state, config);
  if (prop & TRACE_MEM) {
    toolconf child = NULL;
    ARMul_Error err = ARMul_InstallMemoryInterface(
                          state, !(prop & TRACE_BUS), ModelName);
    if (err != ARMulErr_NoError) return err;

    /* now copy our config across... */
    (void)ARMul_FindMemoryInterface(state, ModelName, &child);
    if (child == NULL) return ARMul_RaiseError(state, ARMulErr_NoMemoryType);

    ToolConf_EnumerateTags(config, CopyConfig, child);

    return ARMulErr_NoError;
  }
  
  ts=(Trace_State *)malloc(sizeof(Trace_State));
  if (ts==NULL) return ARMul_RaiseError(state,ARMulErr_OutOfMemory);

  ts->prop = prop;
  ts->prev_instr = 0;  /* Thumb 2-instr BL disassembly */
  return CommonInit(state,ts,config);
}

const ARMul_ModelStub ARMul_TracerModel = {
  ModelInit,
  ModelName
  };

/* EOF tracer.c */
