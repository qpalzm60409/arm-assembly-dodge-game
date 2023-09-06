/*
 * ARM debugger toolbox : dbg_rdi.h
 * Copyright (C) 1992 Advanced Risc Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.70.12.1 $
 * Checkin $Date: 1998/08/24 15:44:57 $
 * Revising $Author: bclarke $
 */

#ifndef dbg_rdi__h
#define dbg_rdi__h

#include "armtypes.h"
#include "dbg_stat.h"
#ifdef PICCOLO
#ifndef RDI_VERSION
#define RDI_VERSION 150
#endif
#endif

#if RDI_VERSION==150
#include "toolconf.h"
#endif

#ifndef RDI_VERSION
#define RDI_VERSION 100
#endif


/*
 * RDI 1.50:  An introduction
 * --------------------------
 *
 * This file, which originally contained the definition for RDI 1, has been
 * extended (through conditional compilation via the RDI_VERSION macro) to
 * additionally include the definition of RDI 1.50.
 *
 * To use RDI 1, then RDI_VERSION should either be undefined, or be defined thus
 *     #define RDI_VERSION 100
 *
 * To use RDI 1.50, then RDI_VERSION should be defined this
 *     #define RDI_VERSION 150
 *
 * CHANGES
 * -------
 * Most RDI functions have had one additional parameter added
 *  RDI_ModuleHandle mh
 * This qualifies the functions for use in a debuggee with:
 *  multi ARM processors
 *  coprocssors that can run their own instructions
 *
 * BACKWARD COMPATIBILITY
 * ----------------------
 * The new framework can use existing debuggees.
 * To this effect error codes, structures, typedefs and flag bit positions
 * must remain as they are. The new RDI provides additional functionality
 * with minimal change.
 *
 * THREADS
 * -------
 * Threads will be dealt with in a later OS dependent interface.
 * ThreadHandles are included, for future use, in break and watch points.
 * Note: There is no method for obtaining ThreadHandles in the RDI.
 *
 */

/***************************************************************************\
*                              Error Codes                                  *
\***************************************************************************/

/* THESE ARE DUPLICATED IN adp.h */

#define RDIError_NoError                0

#define RDIError_Reset                  1
#define RDIError_UndefinedInstruction   2
#define RDIError_SoftwareInterrupt      3
#define RDIError_PrefetchAbort          4
#define RDIError_DataAbort              5
#define RDIError_AddressException       6
#define RDIError_IRQ                    7
#define RDIError_FIQ                    8
#define RDIError_Error                  9
#define RDIError_BranchThrough0         10

#define RDIError_NotInitialised         128
#define RDIError_UnableToInitialise     129
#define RDIError_WrongByteSex           130
#define RDIError_UnableToTerminate      131
#define RDIError_BadInstruction         132
#define RDIError_IllegalInstruction     133
#define RDIError_BadCPUStateSetting     134
#define RDIError_UnknownCoPro           135
#define RDIError_UnknownCoProState      136
#define RDIError_BadCoProState          137
#define RDIError_BadPointType           138
#define RDIError_UnimplementedType      139
#define RDIError_BadPointSize           140
#define RDIError_UnimplementedSize      141
#define RDIError_NoMorePoints           142
#define RDIError_BreakpointReached      143
#define RDIError_WatchpointAccessed     144
#define RDIError_NoSuchPoint            145
#define RDIError_ProgramFinishedInStep  146
#define RDIError_UserInterrupt          147
#define RDIError_CantSetPoint           148
#define RDIError_IncompatibleRDILevels  149

#define RDIError_CantLoadConfig         150
#define RDIError_BadConfigData          151
#define RDIError_NoSuchConfig           152
#define RDIError_BufferFull             153
#define RDIError_OutOfStore             154
#define RDIError_NotInDownload          155
#define RDIError_PointInUse             156
#define RDIError_BadImageFormat         157
#define RDIError_TargetRunning          158
#define RDIError_DeviceWouldNotOpen     159
#define RDIError_NoSuchHandle           160
#define RDIError_ConflictingPoint       161

#define RDIError_TargetBroken           162
#define RDIError_TargetStopped          163

#define RDIError_LinkTimeout            200  /* data link timeout error */
#define RDIError_OpenTimeout            201  /* open timeout (c.f link timeout, on an open */
#define RDIError_LinkDataError          202  /* data error (read/write) on link */
#define RDIError_Interrupted            203  /* (e.g. boot) interrupted */

#define RDIError_LittleEndian           240
#define RDIError_BigEndian              241
#define RDIError_SoftInitialiseError    242

/* New error: ReadOnly, when state can't be written */
#define RDIError_ReadOnly               252
#define RDIError_InsufficientPrivilege  253
#define RDIError_UnimplementedMessage   254
#define RDIError_UndefinedMessage       255

/* Range of numbers that are reserved for RDI implementations */
#define RDIError_TargetErrorBase        256
#define RDIError_TargetErrorTop       (Dbg_Error_Base-1)

/***************************************************************************\
*                          RDP Message Numbers                              *
\***************************************************************************/

/*********                host -> agent                         *********/

#define RDP_Start               (unsigned char)0x0
/* byte type, word memorysize {, byte speed (if (type & 2)) }           */
/* returns byte status                                                  */

#define RDP_End                 (unsigned char)0x1
/* no argument                                                          */
/* returns byte status                                                  */

#define RDP_Read                (unsigned char)0x2
/* word address, word nbytes                                            */
/* returns bytes data, byte status {, word count }                      */
/* The count value is returned only if status != 0, and is the number   */
/* of bytes successfully read                                           */

#define RDP_Write               (unsigned char)0x3
/* word address, word nbytes, bytes data                                */
/* returns byte status {, word count }                                  */
/* The count value is returned only if status != 0, and is the number   */
/* of bytes successfully written                                        */

#define RDP_CPUread             (unsigned char)0x4
/* byte mode, word mask                                                 */
/* returns words data, byte status                                      */

#define RDP_CPUwrite            (unsigned char)0x5
/* byte mode, word mask, words data                                     */
/* returns byte status                                                  */

#define RDP_CPread              (unsigned char)0x6
/* byte cpnum, word mask                                                */
/* returns words data, byte status                                      */

#define RDP_CPwrite             (unsigned char)0x7
/* byte cpnum, word mask, words data                                    */
/* returns byte status                                                  */

#define RDP_SetBreak            (unsigned char)0xa
/* word address, byte type {, word bound }                              */
/* if !(type & RDIPoint_Inquiry)                                        */
/*    returns {word pointhandle, } byte status                          */
/* if (type & RDIPoint_Inquiry)                                         */
/*    returns word address {, word bound }, byte status                 */
/* pointhandle is present if type & RDIPoint_Handle                     */
/* bound arguments and replies are present if                           */
/*   (type & 7) == RDIPoint_IN, RDIPoint_OUT or RDIPoint_MASK           */

#define RDP_ClearBreak          (unsigned char)0xb
/* word pointhandle                                                     */
/* returns byte status                                                  */

#define RDP_SetWatch            (unsigned char)0xc
/* word address, byte type, byte datatype {, word bound }               */
/* if !(type & RDIPoint_Inquiry)                                        */
/*    returns {word pointhandle, } byte status                          */
/* if (type & RDIPoint_Inquiry)                                         */
/*    returns word address {, word bound }, byte status                 */
/* pointhandle is present if type & RDIPoint_Handle                     */
/* bound arguments and replies are present if                           */
/*   (type & 7) == RDIPoint_IN or RDIPoint_OUT                          */

#define RDP_ClearWatch          (unsigned char)0xd
/* word pointhandle                                                     */
/* returns byte status                                                  */

#define RDP_Execute             (unsigned char)0x10
/* byte type                                                            */
/* returns {word pointhandle, } byte status                             */
/* pointhandle is returned if (type & RDIPoint_Handle); type also has:  */
#  define RDIExecute_Async 1

#define RDP_Step                (unsigned char)0x11
/* byte type, word stepcount                                            */
/* returns {word pointhandle, } byte status                             */
/* (type as for RDP_Execute)                                            */

#define RDP_Info                (unsigned char)0x12
/* argument and return type different for each operation: see below     */

#define RDP_OSOpReply           (unsigned char)0x13

#define RDP_AddConfig           (unsigned char)0x14
/* word nbytes                                                          */
/* returns byte status                                                  */

#define RDP_LoadConfigData      (unsigned char)0x15
/* word nbytes, nbytes * bytes data                                     */
/* returns byte status                                                  */

#define RDP_SelectConfig        (unsigned char)0x16
/* byte aspect, byte namelen, byte matchtype, word version,             */
/*      namelen * bytes name                                            */
/* returns word version selected, byte status                           */

#define RDP_LoadAgent           (unsigned char)0x17
/* word loadaddress, word size                                          */
/* followed by a number of messages:                                    */
/*   byte = RDP_LoadConfigData, word size, size * bytes data            */
/*   returns byte status                                                */

#define RDP_Interrupt           (unsigned char)0x18

#define RDP_CCToHostReply       (unsigned char)0x19
#define RDP_CCFromHostReply     (unsigned char)0x1a

/*********                agent -> host                         *********/

#define RDP_Stopped             (unsigned char)0x20
/* reply to step or execute with RDIExecute_Async                       */

#define RDP_OSOp                (unsigned char)0x21
#define RDP_CCToHost            (unsigned char)0x22
#define RDP_CCFromHost          (unsigned char)0x23

#define RDP_Fatal               (unsigned char)0x5e
#define RDP_Return              (unsigned char)0x5f
#define RDP_Reset               (unsigned char)0x7f

/***************************************************************************\
*                            Other RDI values                               *
\***************************************************************************/

#if RDI_VERSION == 150
/* opaque handle types */
typedef struct RDI_AgentState *RDI_AgentHandle;
typedef struct RDI_ModuleState *RDI_ModuleHandle;
#endif

#define RDISex_Little           0 /* the byte sex of the debuggee       */
#define RDISex_Big              1
#define RDISex_DontCare         2

#define RDIPoint_EQ             0 /* the different types of break/watchpoints */
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

#define RDIReg_R15              (1L << 15) /* mask values for CPU       */
#define RDIReg_PC               (1L << 16)
#define RDIReg_CPSR             (1L << 17)
#define RDIReg_SPSR             (1L << 18)
#define RDINumCPURegs           19

#define RDINumCPRegs            10 /* current maximum                   */

#define RDIMode_Curr            255

/*
 * Piccolo register banks
 */
#define RDIMode_Bank0           0
#define RDIMode_Bank1           1
#define RDIMode_Bank2           2
#define RDIMode_Bank3           3
#define RDIMode_Bank4           4
#define RDIMode_Bank5           5
#define RDIMode_Bank6           6

/* RDI_Info subcodes */
/* rdp in parameters are all preceded by                                */
/*   in byte = RDP_Info, word = info subcode                            */
/*     out parameters are all preceded by                               */
/*   out byte = RDP_Return                                              */

#define RDIInfo_Target          0
/* rdi: out ARMword *targetflags, out ARMword *processor id             */
/* rdp: in none, out word targetflags, word processorid, byte status    */
/* the following bits are defined in targetflags                        */
#  define RDITarget_LogSpeed                  0x0f
#  define RDITarget_HW                        0x10    /* else emulator  */
#  define RDITarget_AgentMaxLevel             0xe0
#  define RDITarget_AgentLevelShift       5
#  define RDITarget_DebuggerMinLevel         0x700
#  define RDITarget_DebuggerLevelShift    8
#  define RDITarget_CanReloadAgent           0x800
#  define RDITarget_CanInquireLoadSize      0x1000
#  define RDITarget_UnderstandsRDPInterrupt 0x2000
#  define RDITarget_CanProfile              0x4000
#  define RDITarget_Code16                  0x8000
#  define RDITarget_HasCommsChannel        0x10000

#define RDIInfo_Points          1
/* rdi: out ARMword *pointcapabilities                                  */
/* rdp: in none, out word pointcapabilities, byte status                */
/* the following bits are defined in pointcapabilities                  */
#  define RDIPointCapability_Comparison   1
#  define RDIPointCapability_Range        2
/* 4 to 128 are RDIWatch_xx{Read,Write} left-shifted by two */
#  define RDIPointCapability_Mask         0x100
#  define RDIPointCapability_ThreadBreak  0x200
#  define RDIPointCapability_ThreadWatch  0x400
#  define RDIPointCapability_CondBreak    0x800
#  define RDIPointCapability_Status       0x1000 /* status enquiries available */

#define RDIInfo_Step            2
/* rdi: out ARMword *stepcapabilities                                   */
/* rdp: in none, out word stepcapabilities, byte status                 */
/* the following bits are defined in stepcapabilities                   */
#  define RDIStep_Multiple      1
#  define RDIStep_PCChange      2
#  define RDIStep_Single        4

#define RDIInfo_MMU             3
/* rdi: out ARMword *mmuidentity                                        */
/* rdp: in none, out word mmuidentity, byte status                      */

#define RDIInfo_DownLoad        4
/* Inquires whether configuration download and selection is available.  */
/* rdp: in none, out byte status                                        */
/* No argument, no return value. status == ok if available              */

#define RDIInfo_SemiHosting     5
/* Inquires whether RDISemiHosting_* RDI_Info calls are available.      */
/* rdp: in none, out byte status                                        */
/* No argument, no return value. status == ok if available              */

#define RDIInfo_CoPro           6
/* Inquires whether CoPro RDI_Info calls are available.                 */
/* rdp: in none, out byte status                                        */
/* No argument, no return value. status == ok if available              */

#define RDIInfo_Icebreaker      7
/* Inquires whether debuggee controlled by IceBreaker.                  */
/* rdp: in none, out byte status                                        */
/* No argument, no return value. status == ok if available              */

#define RDIMemory_Access        8
/* rdi: out RDI_MemAccessStats *p, in ARMword *handle                   */
/* rdp: in word handle                                                  */
/*      out word nreads, word nwrites, word sreads, word swrites,       */
/*          word ns, word s, byte status                                */

/* Get memory access information for memory block with specified handle */

#define RDIMemory_Map           9
/* rdi: in  RDI_MemDescr md[n], in ARMword *n                           */
/* rdp: in word n, n * {                                                */
/*           word handle, word start, word limit,                       */
/*           byte width, byte access                                    */
/*           word Nread_ns, word Nwrite_ns,                             */
/*           word Sread_ns, word Swrite_ns}                             */
/*      out byte status                                                 */
/* Sets memory characteristics.                                         */

#define RDISet_CPUSpeed         10
/* rdi: in  ARMword *speed                                              */
/* rdp: in word speed, out byte status                                  */
/* Sets CPU speed (in ns)                                               */

#define RDIRead_Clock           12
/* rdi: out ARMword *ns, out ARMword *s                                 */
/* rdp: in none, out word ns, word s, byte status                       */
/* Reads simulated time                                                 */

#define RDIInfo_Memory_Stats    13
/* Inquires whether RDI_Info codes 8-10 are available                   */
/* rdp: in none, out byte status                                        */
/* No argument, no return value. status == ok if available              */

/* The next two are only to be used if RDIInfo_DownLoad returned no     */
/* error                                                                */
#define RDIConfig_Count         14
/* rdi: out ARMword *count                                              */
/* rdp: out byte status, word count (if status == OK)                   */

/* In addition, the next one is only to be used if RDIConfig_Count      */
/* returned no error                                                    */
typedef struct { unsigned32 version; char name[32]; } RDI_ConfigDesc;
#define RDIConfig_Nth           15
/* rdi: in ARMword *n, out RDI_ConfigDesc *                             */
/* rdp: in word n                                                       */
/*      out word version, byte namelen, bytes * bytelen name,           */
/*          byte status                                                 */

/* Set a front-end polling function to be used from within driver poll  */
/* loops                                                                */
typedef void RDI_PollProc(void *);
typedef struct { RDI_PollProc *p; void *arg; } RDI_PollDesc;
#define RDISet_PollProc         16
/* rdi: in RDI_PollDesc const *from, RDI_PollDesc *to                   */
/*      if from non-NULL, sets the polling function from it             */
/*      if to non-NULL, returns the previous polling function to it     */
/* No corresponding RDP operation                                       */

/* Called on debugger startup to see if the target is ready to execute  */
#define RDIInfo_CanTargetExecute 20
/* rdi: in  void
 *      out byte status (RDIError_NoError => Yes, Otherwise No)
 */

/* Called to set the top of target memory in an ICEman2 system
 * This is then used by ICEman to tell the C Library via the INFOHEAP
 * SWI where the stack should start.
 * Note that only ICEman2 supports this call.  Other systems eg.
 * Demon, Angel, will simply return an error, which means that setting
 * the top of memory in this fashion is not supported.
 */
#define RDIInfo_SetTopMem        21
/* rdi: in  word mem_top
 *      out byte status (RDIError_NoError => Done, Other => Not supported
 */

/* Called before performing a loadagent to determine the endianess of
 * the debug agent, so that images of the wrong bytesex can be
 * complained about
 */
#define RDIInfo_AgentEndianess   22
/* rdi: in void
 *      out byte status
 *      status should be RDIError_LittleEndian or RDIError_BigEndian
 *      any other value indicates the target does not support this
 *      request, so the debugger will have to make a best guess, which
 *      probably means only allow little endian loadagenting.
 */

#define RDIInfo_CanAckHeartbeat  23
/* rdi: in:  void
 *      out: word status
 *
 *      status returns RDIError_NoError to indicate heartbeats can be
 * acknowledged, non-zero otherwise. In addition, host can assume that if
 * target does support it, this call has enabled this support in both
 * host and target.
 */

#define RDIInfo_HeartbeatInterval 24
/* rdi: in:  word new
 *      out: word old
 *
 *      sets the maximum interval in microseconds which can elapse btw.
 * packets being sent on the comms channel. If 'new' is zero, value is 
 * not changed. Returns previous value.
 */

#define RDIInfo_PacketTimeout     25
/* rdi: in:  word new
 *      out: word old
 *
 *      sets the maximum interval in microseconds which can elapse btw.
 * a packet being sent, and a packet being received. If 'new' is zero,
 * value is not changed. Returns previous value. If this timeout is triggered
 * it is assumed that host-target comms has been broken.
 */

#define RDIInfo_AddTimeoutFunc    26
/* rdi: in:  ARMword *func
 *      out: word status
 *
 *      sets a function which will be called on a timeout error (as described
 * for PacketTimeout above) if detected by the lower levels. There can be
 * several such functions defined at once. Returns Error_NoError if ok.
 */

#define RDIInfo_RemTimeoutFunc    27
/* rdi: in:  ARMword *func
 *      out: word status
 *
 *      removes a function previosly added with AddTimeoutFunc above.
 * Returns Error_NoError if function removed ok.
 */

#if RDI_VERSION == 150
/*
 * Function:    RDI_InfoProc(mh, RDIInfo_Modules, arg1, arg2)
 *
 *  Purpose:    Called after RDI_open to obtain a list of Modules from the
 *              debuggee. The handle returned in the structure will be used
 *              in all subsequent calls requiring Module access.
 *
 *              Each module may be given a separate console.
 *              If arg1 == NULL use the target console.
 *
 *  Params:
 *      Input:  agent  handle identified agent
 *
 *              type   RDIInfo_Modules
 *
 *      In/Out:
 *              arg1   unsigned *nProcs
 *                     In: (*nProcs == 0): request to see how many processors
 *                                         are in the system.  The answer is
 *                                         returned in *nProcs
 *                         (*nProcs != 0): request for RDI_ModuleDesc array
 *                                         (arg2) to be filled in.  *nProcs is
 *                                         set to the size of the arg2 array
 *                                         on input, and on output is set to
 *                                         the actual number of processors.
 *
 *              arg2   RDI_ModuleDesc *array.  If (*nProcs != 0) this is an
 *                                         array of size *nProcs, and should
 *                                         be filled in for as many processors
 *                                         as are in the system.  
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      OK:     RDIError_NoMorePoints (no more modules)
 */
#define RDIInfo_Modules           28

typedef struct {
  char type[8];                 /* fill in with name - e.g. "ARM", "Piccolo" */
  char name[8];                 /* unique name of this instance e.g. "ARM_1" */
  struct RDIProcVec *rdi;       /* NULL for "same as agent" */
  RDI_ModuleHandle handle;
} RDI_ModuleDesc;


/*
 *  Function: RDI_InfoProc(mh, RDIInfo_Modifiable, arg1, arg2)
 *   Purpose: Query whether Processor status can be modified
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_Modifiable
 *
 *   Returns:   RDIError_NoError        Status can be modified
 *              RDIError_ReadOnly       Status cannot be modified
 */
#define RDIInfo_Modifiable        29


/*
 *  Function: RDI_InfoProc(mh, RDIInfo_GetClockSpeed, arg1, arg2)
 *   Purpose: Query whether Processor status can be modified
 *
 *    Params:
 *       Input: mh  handle identifies processor
 *
 *              type    RDIInfo_Modifiable
 *
 *       Output:arg1    uint32 *clock_speed.  Returns the clock
 *                      speed of the specified processor in ns.
 *
 *   Returns:   RDIError_NoError
 *              RDIError_UnimplmentedMessage - don't know the speed!
 */
#define RDIInfo_GetClockSpeed     30

#endif


/*
 *  Function: RDI_InfoProc(mh, RDIInfo_MovableVectors, arg1, arg2)
 *   Purpose: Query whether Processor has movable vectors
 *            (a WinCE requirement).  If this is supported (returns
 *            RDIError_NoError) then RDIInfo_{Get,Set}VectorAddress
 *            is also supported.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_MovableVectors
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError        Status can be modified
 *              RDIError_ReadOnly       Status cannot be modified
 */
#define RDIInfo_MovableVectors   31


/*
 *  Function: RDI_InfoProc(mh, RDIInfo_CP15CacheSelection, arg1, arg2)
 *   Purpose: Query whether Processor supports cache manipulation
 *            via CP15, and if it does whether a variable to determine
 *            which cache (I/D/Joint) will be manipulated via CP15.
 *            If this is supported (returns RDIError_NoError) then
 *            RDIInfo_{Get,Set}CP15CacheSelection is also supported.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_CP15CacheSelection
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError        Status can be modified
 *              RDIError_ReadOnly       Status cannot be modified
 */
#define RDIInfo_CP15CacheSelection   32


/*
 *  Function: RDI_InfoProc(mh, RDIInfo_CP15CurrentMemoryArea, arg1, arg2)
 *   Purpose: Query whether Processor supports memory area protection
 *            setup manipulation via CP15, and if it does whether a variable
 *            to determine the current mrmory area being manipulated is to
 *            be supported.
 *            If this is supported (returns RDIError_NoError) then
 *            RDIInfo_{Get,Set}CP15CurrentMemoryArea is also supported.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_CP15CurrentMemoryArea
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError        Status can be modified
 *              RDIError_ReadOnly       Status cannot be modified
 */
#define RDIInfo_CP15CurrentMemoryArea   33


/*
 *  Function: RDI_InfoProc(mh, RDIInfo_SafeNonVectorAddress, arg1, arg2)
 *   Purpose: Query whether the denug agent needs to be told of a safe
 *            non vector address to set the PC to prior to performing
 *            memory reads and writes.  This is only relevent to JTAG
 *            debug based systems.
 *            If this is supported (returns RDIError_NoError) then
 *            RDIInfo_{Get,Set}SafeNonVectorAddress is also supported.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_SafeNonVectorAddress
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError        Status can be modified
 *              RDIError_ReadOnly       Status cannot be modified
 */
#define RDIInfo_SafeNonVectorAddress   34

/*  Function: RDI_InfoProc(mh, RDIInfo_CanForceSystemReset, arg1, arg2)
 *   Purpose: Query whether the target board can be forced to reset
 *            by setting System Reset via the debug agent.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_CanForceSystemReset
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError               System resetting is supported
 *              RDIError_UnimplementedMessage  System resetting not supported
 */
#define RDIInfo_CanForceSystemReset   35

/*  Function: RDI_InfoProc(mh, RDIInfo_ForceSystemReset, arg1, arg2)
 *   Purpose: Actually perform a system reset of the target.  This will only
 *            be supported if RDIInfo_CanForceSystemReset returned
 *            RDIError_NoError.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_ForceSystemReset
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError               System reset performed
 *              RDIError_UnimplementedMessage  System resetting not supported
 */
#define RDIInfo_ForceSystemReset   36

/*  Function: RDI_InfoProc(mh, RDIInfo_IOBitsSupported, arg1, arg2)
 *   Purpose: Query whether the debug agent supports I/O bit writing
 *            (such as the Multi-ICE User Input/Ouput bits).
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_IOBitsSupported
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError               I/O bits are supported
 *              RDIError_UnimplementedMessage  I/O bits are not supported
 */
#define RDIInfo_IOBitsSupported   37


#define RDIInfo_IOBits_Output1  1
#define RDIInfo_IOBits_Output2  2
#define RDIInfo_IOBits_Input1   3
#define RDIInfo_IOBits_Input2   4
 
/*  Function: RDI_InfoProc(mh, RDIInfo_SetIOBits, arg1, arg2)
 *   Purpose: Set the specified I/O Bit
 *
 *    Params:
 *       Input: mh              handle identifies processor
 *       Input: type            RDIInfo_SetIOBits
 *       Input: unsigned *arg1  Indicates the identity of the I/O bit
 *                              to be written.  For Multi-ICE see the
 *                              RDIInfo_IOBits_* definitions above.
 *       Input: unsigned *arg2  0=> set it low, 1=> set it high
 *               
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError               Request performed OK
 *              RDIError_UnimplementedMessage  I/O bits are not supported
 *                                             or invalid I/O bit specified
 */
#define RDIInfo_SetIOBits   38

/*  Function: RDI_InfoProc(mh, RDIInfo_GetIOBits, arg1, arg2)
 *   Purpose: Get the status of the specified I/O Bit
 *
 *    Params:
 *       Input: mh              handle identifies processor
 *       Input: type            RDIInfo_GetIOBits
 *       Input: unsigned *arg1  Indicates the identity of the I/O bit
 *                              to be read.  For Multi-ICE see the
 *                              RDIInfo_IOBits_* definitions above.
 *       Output: unsigned *arg2 Set to 0=> it was low, 1=> it was high
 *               
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError               Request performed OK
 *              RDIError_UnimplementedMessage  I/O bits are not supported
 *                                             or invalid I/O bit specified
 */
#define RDIInfo_GetIOBits   39


/*
 *  Function: RDI_InfoProc(mh, RDISemiHosting_DCC, arg1, arg2)
 *   Purpose: Query whether the debug agent supports debug comms channel
 *            based semihosting.  If it does then it will be possible
 *            to get and set the address of the SWI & DCC handler used
 *            for DCC based semihosting.  The info calls which do
 *            this are: RDIInfo_{Get,Set}SHDCCHandlerAddress.
 *            a return value of RDIError_NoError indicates DCC semihosting
 *            is supported.  RDIError_UnimplementedMessage indicates it
 *            is not supported.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDISemiHosting_DCC
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError               Request performed OK
 *              RDIError_UnimplementedMessage  DCC semihosting is not supported
 */
#define RDISemiHosting_DCC   40




/* The next two are only to be used if the value returned by            */
/* RDIInfo_Points has RDIPointCapability_Status set.                    */
#define RDIPointStatus_Watch    0x80
#define RDIPointStatus_Break    0x81
/* rdi: inout ARMword * (in handle, out hwresource), out ARMword *type  */
/* rdp: in word handle, out word hwresource, word type, byte status     */

#define RDISignal_Stop          0x100
/* Requests that the debuggee stop                                      */
/* No arguments, no return value                                        */
/* rdp: no reply (when the debuggee stops, there will be a reply to the */
/*      step or execute request which started it)                       */

#define RDIVector_Catch         0x180
/* rdi: in ARMword *bitmap                                              */
/* rdp: int word bitmap, out byte status                                */
/* bit i in bitmap set to cause vector i to cause entry to debugger     */

/* The next four are only to be used if RDIInfo_Semihosting returned    */
/* no error                                                             */
#define RDISemiHosting_SetState 0x181
/* rdi: in ARMword *semihostingstate                                    */
/* rdp: in word semihostingstate, out byte status                       */
#define RDISemiHosting_GetState 0x182
/* rdi: out ARMword *semihostingstate                                   */
/* rdp: in none, out word semihostingstate, byte status                 */
#define RDISemiHosting_SetVector 0x183
/* rdi: in ARMword *semihostingvector                                   */
/* rdp: in word semihostingvector, out byte status                      */
#define RDISemiHosting_GetVector 0x184
/* rdi: out ARMword *semihostingvector                                  */
/* rdp: in none, out word semihostingvector, byte status                */

/* The next two are only to be used if RDIInfo_Icebreaker returned      */
/* no error                                                             */
#define RDIIcebreaker_GetLocks  0x185
/* rdi: out ARMword *lockedstate                                        */
/* rdp: in none, out word lockedstate, byte status                      */

#define RDIIcebreaker_SetLocks  0x186
/* rdi: in ARMword *lockedstate                                         */
/* rdp: in word lockedstate, out byte status                            */

/* lockedstate is a bitmap of the icebreaker registers locked against   */
/* use by IceMan (because explicitly written by the user)               */

#define RDIInfo_GetLoadSize     0x187
/* rdi: out ARMword *maxloadsize                                        */
/* rdp: in none, out word maxloadsize, byte status                      */
/* Inquires the maximum length of data transfer the agent is prepared   */
/* to receive                                                           */
/* Only usable if RDIInfo_Target returned RDITarget_CanInquireLoadSize  */
/* rdi: out ARMword *size                                               */

/* Only to be used if the value returned by RDIInfo_Target had          */
/* RDITarget_HasCommsChannel set                                        */
typedef void RDICCProc_ToHost(void *arg, ARMword data);
typedef void RDICCProc_FromHost(void *arg, ARMword *data, int *valid);

#define RDICommsChannel_ToHost  0x188
/* rdi: in RDICCProc_ToHost *, in void *arg                             */
/* rdp: in byte connect, out byte status                                */
#define RDICommsChannel_FromHost 0x189
/* rdi: in RDICCProc_FromHost *, in void *arg                           */
/* rdp: in byte connect, out byte status                                */

/* These 4 are only to be used if RDIInfo_Semihosting returns no error  */
#define RDISemiHosting_SetARMSWI 0x190
/* rdi: in ARMword ARM_SWI_number                                       */
/* rdp: in ARMword ARM_SWI_number, out byte status                      */

#define RDISemiHosting_GetARMSWI 0x191
/* rdi: out ARMword ARM_SWI_number                                      */
/* rdp: out ARMword ARM_SWI_number, byte status                         */

#define RDISemiHosting_SetThumbSWI 0x192
/* rdi: in ARMword Thumb_SWI_number                                     */
/* rdp: in ARMword Thumb_SWI_number, out byte status                    */

#define RDISemiHosting_GetThumbSWI 0x193
/* rdi: out ARMword ARM_Thumb_number                                    */
/* rdp: out ARMword ARM_Thumb_number, byte status                       */

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_GetSafeNonVectorAddress, arg1, arg2)
 *   Purpose: Get the value of the SafeNonVectorAddress, ie. an address
 *            it is safe for JTAG based debug systems to set the PC to
 *            before performing a system speed access.
 *            This is only supported if RDIInfo_SafeNonVectorAddress returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_GetSafeNonVectorAddress
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_GetSafeNonVectorAddress 0x194

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_SetSafeNonVectorAddress, arg1, arg2)
 *   Purpose: Set the value of the SafeNonVectorAddress, ie. an address
 *            it is safe for JTAG based debug systems to set the PC to
 *            before performing a system speed access.
 *            This is only supported if RDIInfo_SafeNonVectorAddress returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_SetSafeNonVectorAddress
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_SetSafeNonVectorAddress 0x195

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_GetVectorAddress, arg1, arg2)
 *   Purpose: Get the value of the VectorAddress, ie. the location of
 *            the exception vectors.
 *            This is only supported if RDIInfo_VectorAddress returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_GetVectorAddress
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_GetVectorAddress 0x196

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_SetVectorAddress, arg1, arg2)
 *   Purpose: Set the value of the VectorAddress, ie. the location of
 *            the exception vectors.
 *            This is only supported if RDIInfo_VectorAddress returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_SetVectorAddress
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_SetVectorAddress 0x197

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_GetCP15CacheSelected, arg1, arg2)
 *   Purpose: Get the identity of cache which has been selected for manipulation
 *            via CP15.  0=>D-Cache, 1=>I-Cache.
 *            This is only supported if RDIInfo_CP15CacheSelection returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_GetCP15CacheSelected
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_GetCP15CacheSelected 0x198

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_SetCP15CacheSelected, arg1, arg2)
 *   Purpose: Set the identity of cache which has been selected for manipulation
 *            via CP15.  0=>D-Cache, 1=>I-Cache.
 *            This is only supported if RDIInfo_CP15CacheSelection returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_SetCP15CacheSelected
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_SetCP15CacheSelected 0x199

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_GetCP15CurrentMemoryArea, arg1, arg2)
 *   Purpose: Get the identity of memory area which is to be manipulated via
 *            CP15.
 *            This is only supported if RDIInfo_CP15CurrentMemoryArea returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_GetCP15CurrentMemoryArea
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_GetCP15CurrentMemoryArea 0x19A

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_SetCP15CurrentMemoryArea, arg1, arg2)
 *   Purpose: Set the identity of memory area which is to be manipulated via
 *            CP15.
 *            This is only supported if RDIInfo_CP15CurrentMemoryArea returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_SetCP15CurrentMemoryArea
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_SetCP15CurrentMemoryArea 0x19B


/*
 *  Function: RDI_InfoProc(mh, RDISemiHosting_GetDCCHandlerAddress, arg1, arg2)
 *   Purpose: Get the value of the semihosting DCCHandler Address, ie. the
 *            address at which a SWI Handler is automatically loaded if
 *            Debug Comms Channel based semihosting is enabled.
 *            This is only supported if RDISemiHosting_DCC returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDISemiHosting_GetDCCHandlerAddress
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDISemiHosting_GetDCCHandlerAddress 0x19C

/*
 *  Function: RDI_InfoProc(mh, RDISemiHosting_SetDCCHandlerAddress, arg1, arg2)
 *   Purpose: Get the value of the semihosting DCCHandler Address, ie. the
 *            address at which a SWI Handler is automatically loaded if
 *            Debug Comms Channel based semihosting is enabled.
 *            This is only supported if RDISemiHosting_DCC returns
 *            RDIError_NoError
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDISemiHosting_SetDCCHandlerAddress
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDISemiHosting_SetDCCHandlerAddress 0x19D



#define RDICycles               0x200
/* rdi: out ARMword cycles[n*2]                                         */
/* rdp: in none, out 6 words cycles, byte status                        */
/* the rdi result represents <n> big-endian doublewords; the rdp        */
/* results return values for the ls halves of 6 of these (n=6 for rdp)  */
/* For rdi the value of <n> depends on the return value of              */
/* RequestCyclesDesc, and defaults to 6.                                */
#  define RDICycles_Size        48

#define RDIErrorP               0x201
/* rdi: out ARMaddress *errorp                                          */
/* rdp: in none, out word errorp, byte status                           */
/* Returns the error pointer associated with the last return from step  */
/* or execute with status RDIError_Error.                               */

#define RDIRequestCyclesDesc    0x202
/* rdi: in  int *size, RDI_CycleDesc *sd                                */
/*      out size modified, sd filled in                                 */
/* rdp: not supported                                                   */
/* target fills in a description of the statistics returned by          */
/* RDICycles. If this call is unsupported (as in rdp), RDICycles is     */
/* assumed to return:                                                   */
/* {"\014Instructions",                                                 */
/*  "\010S_Cycles",                                                     */
/*  "\010N_Cycles",                                                     */
/*  "\010I_Cycles",                                                     */
/*  "\010C_Cycles",                                                     */
/*  "\010F_Cycles"}                                                     */
/* Cycles are assumed to be monotonically increasing counters. All the  */
/* fields are treated as such, and the debugger uses this fact to       */
/* generate "increment" counters.                                       */
/* The size of the array "sd" is passed in in "size" (number of         */
/* entries), which the target should modify to indicate the number of   */
/* entries used.                                                        */


#define RDISet_Cmdline          0x300
/* rdi: in char *commandline (a null-terminated string)                 */
/* No corresponding RDP operation (cmdline is sent to the agent in      */
/* response to SWI_GetEnv)                                              */

#define RDISet_RDILevel         0x301
/* rdi: in ARMword *level                                               */
/* rdp: in word level, out byte status                                  */
/* Sets the RDI/RDP protocol level to be used (must lie between the     */
/* limits returned by RDIInfo_Target).                                  */

#define RDISet_Thread           0x302
/* rdi: in ARMword *threadhandle                                        */
/* rdp: in word threadhandle, out byte status                           */
/* Sets the thread context for subsequent thread-sensitive operations   */
/* (null value sets no thread)                                          */

/* Only to be used if RDI_read or RDI_write returned                    */
/* RDIError_LittleEndian or RDIError_BigEndian, to signify that the     */
/* debugger has noticed.                                                */
#define RDIInfo_AckByteSex  0x303
/* rdi: in ARMword *sex (RDISex_Little or RDISex_Big)                   */

/* Read endian sex of module                                            */
#define RDIInfo_ByteSex 0x304
/* rdi: out ARMword *sex (RDISex_Little, RDISex_Big or RDISex_DontCare  */


/* The next two are only to be used if RDIInfo_CoPro returned no error  */
#define RDIInfo_DescribeCoPro   0x400
/* rdi: in int *cpno, Dbg_CoProDesc *cpd                                */
/* rdp: in byte cpno,                                                   */
/*         cpd->entries * {                                             */
/*           byte rmin, byte rmax, byte nbytes, byte access,            */
/*           byte cprt_r_b0, cprt_r_b1, cprt_w_b0, cprt_w_b1}           */
/*         byte = 255                                                   */
/*      out byte status                                                 */

#define RDIInfo_RequestCoProDesc 0x401
/* rdi: in int *cpno, out Dbg_CoProDesc *cpd                            */
/* rpd: in byte cpno                                                    */
/*      out nentries * {                                                */
/*            byte rmin, byte rmax, byte nbytes, byte access,           */
/*          }                                                           */
/*          byte = 255, byte status                                     */

#define RDIInfo_Log             0x800
/* rdi: out ARMword *logsetting                                         */
/* No corresponding RDP operation                                       */
#define RDIInfo_SetLog          0x801
/* rdi: in ARMword *logsetting                                          */
/* No corresponding RDP operation                                       */

#define RDIProfile_Stop         0x500
/* No arguments, no return value                                        */
/* rdp: in none, out byte status                                        */
/* Requests that pc sampling stop                                       */

#define RDIProfile_Start        0x501
/* rdi: in ARMword *interval                                            */
/* rdp: in word interval, out byte status                               */
/* Requests that pc sampling start, with period <interval> usec         */

#define RDIProfile_WriteMap     0x502
/* rdi: in ARMword map[]                                                */
/* map[0] is the length of the array, subsequent elements are sorted    */
/* and are the base of ranges for pc sampling (so if the sampled pc     */
/* lies between map[i] and map[i+1], count[i] is incremented).          */
/* rdp: a number of messages, each of form:                             */
/*        in word len, word size, word offset, <size> words map data    */
/*        out status                                                    */
/* len, size and offset are all word counts.                            */

#define RDIProfile_ReadMap      0x503
/* rdi: in ARMword *len, out ARMword counts[len]                        */
/* Requests that the counts array be set to the accumulated pc sample   */
/* counts                                                               */
/* rdp: a number of messages, each of form:                             */
/*        in word offset, word size                                     */
/*        out <size> words, status                                      */
/* len, size and offset are all word counts.                            */

#define RDIProfile_ClearCounts  0x504
/* No arguments, no return value                                        */
/* rdp: in none, out byte status                                        */
/* Requests that pc sample counts be set to zero                        */

#define RDIInfo_RequestReset    0x900
/* Request reset of the target - used only in RDP based systems         */
/* No arguments, no return value                                        */
/* No RDP equivalent, sends an RDP reset                                */

/* To support extension RDI components                                  */
#define RDIInfo_SetProcessID    0x0a00
/* Inform the RDI component of the current process ID                   */
/* rdi in: int *process_id                                              */

#define RDIInfo_LoadStart       0x0a01
#define RDIInfo_CustomLoad      0x8a01
/* Inform the RDI component when the download is about to start         */
/* rdi: in address                                                      */

#define RDIInfo_LoadEnd         0x0a02
/* Inform the RDI component when the download has completed             */
/* rdi: out read registers flag                                         */

#define RDIInfo_GeneralPurpose    0x0a03
#define RDIInfo_CanDoGeneralPurpose 0x8a03

/* General pupose RDI Info call                                         */
/* rdi: in pointer to implementation specific data structure            */
/* rdi: out pointer to implementation specific data structure           */

#define RDIInfo_CapabilityRequest 0x8000
/* Request whether the interface supports the named capability. The     */
/* capability is specified by or'ing the RDIInfo number with this, and  */
/* sending that request                                                 */
/* rdi: in none                                                         */
/* rdp: in none, out byte status                                        */

typedef struct {
  ARMword len;
  ARMword map[1];
} RDI_ProfileMap;

typedef unsigned32 PointHandle;
typedef unsigned32 ThreadHandle;
#define RDINoPointHandle        ((PointHandle)-1L)
#define RDINoHandle             ((ThreadHandle)-1L)

struct Dbg_HostosInterface;
struct Dbg_MCState;
struct Dbg_AgentState;

/**********************************************************************/

/*
 * RDI Implementation Prototypes
 */

/*
 * RDI VERSION 1.50
 */

#if RDI_VERSION == 150


/*
 *  Function:   RDI_OpenAgent
 *   Purpose:   Initialisation routine for debug agent, which supports both
 *              hot and cold calls; the cold call is guaranteed to be the
 *              very first call made from the debugger into the debuggee.
 *
 *      Input:  type
 *                  b0          0   cold start
 *                              1   warm start
 *                  b1          1   reset the comms link
 *                  b2          0/1 little/big endian
 *                  b3          1   return endian
 *
 *              config          structure containing configuration
 *                              information
 *
 *              hostif          used for reading/writing to a console
 *
 *              dbg_state       handle to identify the current Debugger
 *                              state
 *
 *     In/Out:  agent           cold start:     passed in as NULL, returns
 *                                              pointer to debug agent instance
 *
 *                              warm start:     pointer to debug agent instance
 *
 *  Returns:                                 b3     b2
 *      Error:  RDIError_SoftInitialiseError X      X   If things go wrong!
 *      Error:  RDIError_WrongByteSex        0      E   bad match
 *      OK:     RDIError_NoError             0      E   match
 *      OK:     RDIError_LittleEndian        1      X
 *      OK:     RDIError_BigEndian           1      X
 *
 * Post-conditions: If no error occured, then *agent will contain a pointer
 *                      to the debug agent instance (newly created if a cold
 *                      start), and all parts of the system other than the
 *                      Processor modules will have seen the open too.
 */
typedef int rdi_openagent_proc(
    RDI_AgentHandle *agent, unsigned type, toolconf config,
    struct Dbg_HostosInterface const *i, struct Dbg_AgentState *dbg_state);


/*
 * Function:    RDI_CloseAgent
 *  Purpose:    Equivalent to the debuggee destructor.
 *              Ensure that all processors are also closed down
 *              Release any memory and close down the emulator.
 *              Only an RDI_open call can follow.
 *
 *  Params:     agent - handle of agent to close down
 *
 *  Returns:    RDIError_NoError
 */
typedef int rdi_closeagent_proc(RDI_AgentHandle agent);


/*
 * Function:    RDI_OpenProc
 *  Purpose:    This is the Initialization routine and as such is the first
 *              call made to the RDI. It allows the program to set up the
 *              initial debuggee working environment. It will be called
 *              later if the emulator is to be reset. There are hot and
 *              cold calls. On the very first cold call is equivalent to a
 *              constructor when memory should be claimed.
 *              The initialization also provides a console window for
 *              reading and writing text. This can be used now and in the
 *              future for messages. The call provides information on the
 *              settings on the debugger.
 *
 *  Pre-conditions:
 *
 *  Params:
 *      Input:  handle          Processor module handle
 *
 *              type
 *                  b0          0   cold start
 *                              1   warm start
 *                  b1          1   reset the comms link
 *                  b2          0/1 little/big endian
 *                  b3          1   return endian
 *
 *              config          structure containing configuration
 *                              information
 *
 *              hostif          used for reading/writing to a console
 *
 *              dbg_state       handle to identify the current Debugger
 *                              state
 *
 *  Returns:                                 b3     b2
 *      Error:  RDIError_SoftInitialiseError X      X   If things go wrong!
 *      Error:  RDIError_WrongByteSex        0      E   bad match
 *      OK:     RDIError_NoError             0      E   match
 *      OK:     RDIError_LittleEndian        1      X
 *      OK:     RDIError_BigEndian           1      X
 */
typedef int rdi_open_proc(
    RDI_ModuleHandle rdi_handle, unsigned type, toolconf config,
    struct Dbg_HostosInterface const *i, struct Dbg_MCState *dbg_state);


/*
 * Function:    RDI_CloseProc
 *  Purpose:    Close down an individual processor.
 *              > ISSUE <
 *              Behaviour is currently undefined if only a subset of
 *              processors are 'open' and execution is requested.
 *
 *  Params:     mh - Handle of processor to close down
 *
 *  Returns:    RDIError_NoError
 */
typedef int rdi_close_proc(RDI_ModuleHandle mh);


/*
 * Access types for RDI(Read,Write}Proc
 */
typedef enum {
  rdi_data8 = 0,
  rdi_data16,
  rdi_data32,
  rdi_data64,
  rdi_code16 = 6,
  rdi_code32
} RDI_AccessType;


/*
 * Function:    RDI_ReadProc
 *  Purpose:    Read memory contents in byte format from the debuggee.
 *              read *nbytes of data from source address and write to dest
 *              in the caller's address space using the processor specified.
 *
 *              Assuming nothing went wrong with the transfer, the debuggee
 *              will normally return RDIError_NoError.  However, if the
 *              debuggee has changed its byte sex, it should return a
 *              notification of this change, i.e. either RDIError_BigEndian
 *              or RDI_ErrorLittleEndian; the caller must then acknowledge
 *              this notification by calling RDI_Info RDIInfo_AckByteSex.
 *              The debuggee will keep returning this notification until an
 *              acknowledgement has been received.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *
 *              source      address
 *
 *              access      type and width of access
 *
 *      In/Out: *nbytes     In:  number of bytes to read
 *                          Out: number of bytes actually read
 *
 *      Output: *dest       destination buffer
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *              RDIError_BigEndian
 *              RDIError_LittleEndian
 *      Error:  RDIError_DataAbort  (if a memory exception occurs)
 */
typedef int rdi_read_proc(
    RDI_ModuleHandle mh, ARMword source, void *dest, unsigned *nbytes,
    RDI_AccessType type);


/*
 * Function:    RDI_WriteProc
 *  Purpose:    Write memory contents in byte format to the debuggee.
 *              read *nbytes of data from source address in the caller's
 *              address space, and write to dest using the processor specified.
 *
 *              Assuming nothing went wrong with the transfer, the debuggee
 *              will normally return RDIError_NoError.  However, if the
 *              debuggee has changed its byte sex, it should return a
 *              notification of this change, i.e. either RDIError_BigEndian
 *              or RDI_ErrorLittleEndian; the caller must then acknowledge
 *              this notification by calling RDI_Info RDIInfo_AckByteSex.
 *              The debuggee will keep returning this notification until an
 *              acknowledgement has been received.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *
 *              source      source buffer
 *
 *              dest        destination address
 *
 *              access      type and width of access
 *
 *      In/Out: *nbytes     In:  number of bytes to write
 *                          Out: number of bytes actually written
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *              RDIError_BigEndian
 *              RDIError_LittleEndian
 *      Error:  RDIError_DataAbort  (if a memory exception occurs)
 */
typedef int rdi_write_proc(
    RDI_ModuleHandle mh, const void *source, ARMword dest, unsigned *nbytes,
    RDI_AccessType type);

/*
 * Function:    RDI_CPUreadProc
 *  Purpose:    Read the register content from the specified processor.
 *              The mask specifies which register(s) are to be transferred
 *              as 32-bit ARMwords.
 *              The mask has been extended to support more than 32 registers
 *  Params:
 *      Input:  mh          handle identifies processor
 *
 *      Input:  mode        mode defines register context
 *                          ARM
 *                              RDIMode_Curr    use the current mode
 *                              Other values match the mode bits of the PSR.
 *                          PICCOLO
 *                              RDIModeBankX    Transfer registers
 *                                              from bankX
 *
 *      Input:  mask[]      bits defining which registers are to be
 *                          transferred. These bits can be ORed to read any
 *                          combination of registers.
 *                          ARM
 *                              b0 - b14    r0 - r14
 *                              b15         Program Counter including the
 *                                          mode and flag bits in 26-bit modes
 *                              RDIReg_PC   Just the program counter
 *                              RDIReg_CPSR CPSR register
 *                              RDIReg_SPSR SPSR register
 *                          PICCOLO
 *                              b0-b<x>     mask of which words in the
 *                                          bank to transfer
 *
 *      Output: state[]     destination array of ARMwords
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  error code
 */
typedef int rdi_CPUread_proc(
    RDI_ModuleHandle mh, unsigned mode, unsigned32 mask, ARMword *state);


/*
 * Function:    RDI_CPUwriteProc
 *  Purpose:    Write the register content to the specified processor.
 *              The mask specifies which register(s) are to be transferred
 *              as 32-bit ARMwords. The mask has been extended to support
 *              more than 32 registers in RDI2.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *      Input:  mode        mode defines register context
 *                          ARM
 *                              RDIMode_Curr    use the current mode
 *                              Other values match the mode bits of the PSR.
 *                          PICCOLO
 *                              As for RDI_CPUreadProc
 *      Input:  mask[]      bits defining which registers are to be
 *                          transferred. These bits can be ORed to read any
 *                          combination of registers.
 *                          ARM
 *                              b0 - b14    r0 - r14
 *                              b15         Program Counter including the mode
 *                                          and flag bits in 26-bit modes
 *                              RDIReg_PC   Just the program counter
 *                              RDIReg_CPSR CPSR register
 *                              RDIReg_SPSR SPSR register
 *                          PICCOLO
 *                              As for RDI_CPUreadProc
 *      Input:  state[]     source array of ARMwords
 *      InOut:  none
 *      Output: none
 *
 *  Returns:
 *         OK:  RDIError_NoError
 *     Not OK:  RDIError_ReadOnly - if writing is not allowed at this time
 *      Error:  error code
 */
typedef int rdi_CPUwrite_proc(
    RDI_ModuleHandle mh, unsigned mode, unsigned32 mask, ARMword const *state);


/*
 * Function:    RDI_CPreadProc
 *  Purpose:    Read the register content from the specified coprocessor.
 *              The mask specifies which register(s) are to be transferred
 *              as 32-bit ARMwords. The mask has been extended to support
 *              more than 32 registers in RDI2.
 *
 *              Each coprocessor will have its own register set defined in
 *              another header file. The actual registers transferred, and
 *              their size is dependent on the coprocessor specified.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *      Input:  CPnum       coprocessor number for this processor
 *      Input   mask[]      bits define which registers are to be transferred
 *      InOut:  none
 *      Output: state[]     destination array for register values
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  error code
 */
typedef int rdi_CPread_proc(
    RDI_ModuleHandle mh, unsigned CPnum, unsigned32 mask, ARMword *state);

/*
 * Function:    RDI_CPwriteProc
 *  Purpose:    Write the register content to the specified coprocessor.
 *              The mask specifies which register(s) are to be transferred
 *              as 32-bit ARMwords. The mask has been extended to support
 *              more than 32 registers in RDI2.
 *
 *              Each coprocessor will have its own register set defined in
 *              another header file. The actual registers transferred, and
 *              their size is dependent on the coprocessor specified.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *      Input:  CPnum       coprocessor number for this processor
 *      Input   mask[]      bits define which registers are to be transferred
 *      Input:  state[]     source array of register values
 *      InOut:  none
 *      Output: none
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  error code
 */
typedef int rdi_CPwrite_proc(
    RDI_ModuleHandle mh, unsigned CPnum, unsigned32 mask, ARMword const *state);


/*
 * Function:    RDI_SetBreakProc
 *  Purpose:    Set a breakpoint in the debuggee.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *      Input:  address     address of breakpoint
 *      Input:  type        one of RDIPoint_ types
 *
 *   RDIPoint_EQ          equal to address
 *   RDIPoint_GT          greater than address
 *   RDIPoint_GE          greater than or equal to address
 *   RDIPoint_LT          less than address
 *   RDIPoint_LE          less than or equal to address
 *   RDIPoint_IN          in the range from address to bound, inclusive
 *   RDIPoint_OUT         not in the range from address to bound, inclusive
 *   RDIPoint_MASK        halt execution if (pc & bound) = address
 *   RDIPoint_16Bit       if set breakpoint is on 16-bit (Thumb) instruction
 *   RDIPoint_Conditional if set breakpoint only occurs when the
 *                        instructions condition is true.
 *
 *      Input:  bound       upper address range
 *      Input:  ThreadHandle thread used by OS (set to RDINoHandle for global)
 *      InOut:  none
 *      Output: *handle     A handle to identify the breakpoint
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      OK:     RDIError_NoMorePoints - successful,
 *                                      but no more points available
 *      Error:  RDIError_CantSetPoint
 */
typedef int rdi_setbreak_proc(
    RDI_ModuleHandle mh, ARMword address, unsigned type, ARMword bound,
    ThreadHandle thread, PointHandle *handle);


/*
 * Function:    RDI_ClearBreakProc
 *  Purpose:    Clear a breakpoint in the debuggee.
 *
 *  Params:
 *      Input:  mh          identifies processor
 *      Input:  handle      identifies breakpoint
 *      InOut:  none
 *      Output: none
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  RDIError_NoSuchPoint
 */
typedef int rdi_clearbreak_proc(RDI_ModuleHandle mh, PointHandle handle);


/*
 * Function:    RDI_SetWatchProc
 *  Purpose:    Set a watchpoint in the debuggee.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *      Input:  address     address of watchpoint
 *      Input:  type        one of RDIPoint_ types
 *
 *   RDIPoint_EQ          equal to address
 *   RDIPoint_GT          greater than address
 *   RDIPoint_GE          greater than or equal to address
 *   RDIPoint_LT          less than address
 *   RDIPoint_LE          less than or equal to address
 *   RDIPoint_IN          in the range from address to bound, inclusive
 *   RDIPoint_OUT         not in the range from address to bound, inclusive
 *   RDIPoint_MASK        halt execution if (pc & bound) = address
 *
 *      Input:  datatype    one of RDIWatch_ data types
 *      Input:  bound       upper address range
 *      Input:  ThreadHandle thread used by OS (set to RDINoHandle for global)
 *      InOut:  none
 *      Output: *handle     A handle to identify the watchpoint
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      OK:     RDIError_NoMorePoints - successful,
 *                                      but no more points available
 *      Error:  RDIError_CantSetPoint
 */
typedef int rdi_setwatch_proc(
    RDI_ModuleHandle mh, ARMword address, unsigned type, unsigned datatype,
    ARMword bound, ThreadHandle thread, PointHandle *handle);


/*
 * Function:    RDI_ClearWatchProc
 *  Purpose:    Clear a watchpoint in the debuggee.
 *
 *  Params:
 *      Input:  mh          identifies processor
 *      Input:  handle      identifies watchpoint
 *      InOut:  none
 *      Output: none
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  RDIError_NoSuchPoint
 */
typedef int rdi_clearwatch_proc(RDI_ModuleHandle mh, PointHandle handle);


/*
 * Function:    RDI_ExecuteProc
 *  Purpose:    Starts execution of the debuggee from the current state.
 *              The debuggee continues until a natural exit occurs.
 *              The reason for exit is returned.
 *
 *  Params:
 *      Input:  agent   Identifies the debug agent.
 *      Input:  stop_others
 *                      TRUE => Only run the specified processor (*mh).  All
 *                              other processors remain halted
 *                      FALSE =>Start the whole target running
 *
 *      InOut:  *mh
 *          In:         (stop_others == TRUE): 
 *                           identifies a specific processor to execute
 *                      (stop_others == FALSE):
 *                           ignored
 *
 *          Out:        Handle of the processor that caused the exit.
 *                      If no specific processor caused the exit, this is the
 *                      handle of any valid processor.
 *
 *      Output: *point  if (return == RDIError_BreakpointReached)
 *                          handle of breakpoint
 *                      if (return == RDIError_WatchpointAccessed)
 *                          handle of watchpoint
 *                      else RDINoHandle
 *                      In the current Demon debuggee the handles are not set.
 *                      Here the debugger is required to fetch the pc to
 *                      determine the point.
 *                      (Demon condition is (pointReached && RDINoHandle))
 *
 *  Returns:
 *      OK:         RDIError_NoError
 *      OK:         RDIError_BreakpointReached    Breakpoint reached
 *      OK:         RDIError_WatchpointAccessed   Watch point accessed
 *      OK:         RDIError_UserInterrupt        User stop via RDISignal_Stop
 *
 *      Vector Catches (see RDIVector_Catch Info call)
 *      OK:         RDIError_BranchThrough0       Branch through zero
 *      OK:         RDIError_UndefinedInstruction Undefined instruction
 *      OK:         RDIError_SoftwareInterrupt    Software interrupt
 *      OK:         RDIError_PrefetchAbort        Prefetch abort
 *      OK:         RDIError_DataAbort            Data abort
 *      OK:         RDIError_AddressException     Address exception
 *      OK:         RDIError_IRQ                  Interrupt (IRQ)
 *      OK:         RDIError_FIQ                  Fast Interrupt (FRQ)
 *      OK:         RDIError_Error                Error (see RDIErrorP)
 *
 *      Error:      RDIError_UnimplementedMessage under the following conditions
 *                      if stop_others = TRUE and *mh = RDINoHandle
 *                  or
 *                      if stop_others = TRUE and stopping of individual
 *                      processors is not supported on the target
 *                  or
 *                      if stop_others = TRUE and the particular configuration
 *                      of running the specified processor and stopping others
 *                      is not permitted, even though other 'stop_others'
 *                      configurations are supported.
 */
typedef int rdi_execute_proc(
    RDI_AgentHandle agent, RDI_ModuleHandle *mh, bool stop_others, 
    PointHandle *handle);


/*
 * Function:    RDI_StepProc
 *  Purpose:    Steps the debuggee from the current state.
 *              The debuggee continues until a natural exit occurs
 *              or the specified processor completes its next instruction.
 *              If a processor if not specified (RDINoHandle) then the exit
 *              shall be when any processor reaches its next instruction
 *              completion. The reason for exit is returned.
 *
 *  Params:
 *      Input:  agent   Identifies the debug agent.
 *      Input:  stop_others
 *                      TRUE => Only run the specified processor (*mh).  All
 *                              other processors remain halted
 *                      FALSE =>Start the whole target running
 *
 *      InOut:  *mh
 *          In:         identifies a specific processor to step.  In the absence
 *                      of other exit conditions, the execution will halt when
 *                      the specified processor completes a step.
 *
 *                      if (*mh == RDINoHandle), then in the absence of other
 *                      exit conditions, the execution will halt when any
 *                      processor completes a step.
 *
 *          Out:        Handle of the processor that caused the exit.
 *                      If no specific processor caused the exit, this is the
 *                      handle of any valid processor.
 *
 *      Output: *point  if (return == RDIError_BreakpointReached)
 *                          handle of breakpoint
 *                      if (return == RDIError_WatchpointAccessed)
 *                          handle of watchpoint
 *                      else RDINoHandle
 *                      In the current Demon debuggee the handles are not set.
 *                      Here the debugger is required to fetch the pc to
 *                      determine the point.
 *                      (Demon condition is (pointReached && RDINoHandle))
 *
 *  Returns:
 *      OK:         RDIError_NoError
 *      OK:         RDIError_BreakpointReached    Breakpoint reached
 *      OK:         RDIError_WatchpointAccessed   Watch point accessed
 *      OK:         RDIError_UserInterrupt        User stop via RDISignal_Stop
 *
 *      Vector Catches (see RDIVector_Catch Info call)
 *      OK:         RDIError_BranchThrough0       Branch through zero
 *      OK:         RDIError_UndefinedInstruction Undefined instruction
 *      OK:         RDIError_SoftwareInterrupt    Software interrupt
 *      OK:         RDIError_PrefetchAbort        Prefetch abort
 *      OK:         RDIError_DataAbort            Data abort
 *      OK:         RDIError_AddressException     Address exception
 *      OK:         RDIError_IRQ                  Interrupt (IRQ)
 *      OK:         RDIError_FIQ                  Fast Interrupt (FRQ)
 *      OK:         RDIError_Error                Error (see RDIErrorP)
 *
 *      Error:      RDIError_UnimplementedMessage under the following conditions
 *                      if stop_others = TRUE and *mh = RDINoHandle
 *                  or
 *                      if stop_others = TRUE and stopping of individual
 *                      processors is not supported on the target
 *                  or
 *                      if stop_others = TRUE and the particular configuration
 *                      of running the specified processor and stopping others
 *                      is not permitted, even though other 'stop_others'
 *                      configurations are supported.
 */
typedef int rdi_step_proc(
    RDI_AgentHandle agent, RDI_ModuleHandle *mh, bool stop_others,
    unsigned ninstr, PointHandle *handle);


/* The argument to rdi_info_proc is either an RDI_ModuleHandle (for the
 * majority of calls) or an RDI_AgentHandle (for a small number of
 * "Agent" calls). Which it is depends on the info code. Unless otherwise
 * stated (above) all calls take a ModuleHandle.
 */

/*
 * Function:    RDI_InfoProc
 *  Purpose:    Used to transfer miscellaneous information between the
 *              Debuger and the Debuggee. The parameters meanings change for
 *              each information type.
 *              Each type is defined in RDI_Info subcodes.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *                          if RDINoHandle this is general target information
 *                          (e.g. hadware configuration) ?? This looks wrong ??
 *      Input:  type        Information type code
 *
 *      Use of arg parameters varies dependent on type code.
 *      See each individual definition below.
 *      InOut:  arg1        parameter 1
 *      InOut:  arg2        parameter 2
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      OK:     codes dependent on type
 *      Error:  codes dependent on type
 *      Error:  RDIError_UnimplementedMessage (unimplemented functions)
 */
typedef int rdi_info_proc(
    void *, unsigned type, ARMword *arg1, ARMword *arg2);


/*
 * Function:    RDI_PointinqProc
 *  Purpose:    Breakpoint or watchpoint inquiry.
 *              Used to check before setbreak or setwatch is used.
 *              Returns what will happen if a corresponding call is made
 *              to setbreak or setwatch.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *      Input:  type        see setbreak or setwatch for definition
 *      Input:  datatype    0       breakpoint
 *                          != 0    see setwatch for definition
 *      InOut:  *address
 *          In:             address of point
 *          Out:            set to actual value that will be used
 *      InOut:  *bound
 *          In:             upper address range
 *          Out:            set to actual value that will be used
 *      Output: none
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  RDIError_NoMorePoints (if cannot set)
 */
typedef int rdi_pointinq_proc(
    RDI_ModuleHandle, ARMword *address, unsigned type, unsigned datatype,
    ARMword *bound);

#else

/*
 * RDI 1.00 definitions
 */

struct Dbg_ConfigBlock;

typedef int rdi_open_proc(
    unsigned type, struct Dbg_ConfigBlock const *config,
    struct Dbg_HostosInterface const *i, struct Dbg_MCState *dbg_state);

typedef int rdi_close_proc(void);

typedef int rdi_read_proc(ARMword source, void *dest, unsigned *nbytes);
typedef int rdi_write_proc(const void *source, ARMword dest, unsigned *nbyte);
typedef int rdi_CPUread_proc(unsigned mode, unsigned32 mask, ARMword *state);
typedef int rdi_CPUwrite_proc(
    unsigned mode, unsigned32 mask, ARMword const *state);
typedef int rdi_CPread_proc(unsigned CPnum, unsigned32 mask, ARMword *state);
typedef int rdi_CPwrite_proc(
    unsigned CPnum, unsigned32 mask, ARMword const *state);
typedef int rdi_setbreak_proc(
    ARMword address, unsigned type, ARMword bound, PointHandle *handle);
typedef int rdi_clearbreak_proc(PointHandle handle);
typedef int rdi_setwatch_proc(
    ARMword address, unsigned type, unsigned datatype, ARMword bound,
    PointHandle *handle);
typedef int rdi_clearwatch_proc(PointHandle handle);
typedef int rdi_execute_proc(PointHandle *handle);
typedef int rdi_step_proc(unsigned ninstr, PointHandle *handle);
typedef int rdi_info_proc(unsigned type, ARMword *arg1, ARMword *arg2);
typedef int rdi_pointinq_proc(
    ARMword *address, unsigned type, unsigned datatype, ARMword *bound);

#endif

typedef enum {
    RDI_ConfigCPU,
    RDI_ConfigSystem
} RDI_ConfigAspect;

typedef enum {
    RDI_MatchAny,
    RDI_MatchExactly,
    RDI_MatchNoEarlier
} RDI_ConfigMatchType;

#if RDI_VERSION == 150

/*
 * More RDI 1.50 prototypes.  Why are these separated from the above list?
 */


/*
 * Function:    RDI_AddConfigProc
 *  Purpose:    Declares the size of a config block about to be loaded.
 *              Only use if RDIInfo_DownLoad returns no error.
 *
 *  Params:
 *      Input:  agent   handle identifies degug agent
 *
 *              nbytes  size of configuration block to follow
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  RDIError_UnimplementedMessage (unimplemented functions)
 */
typedef int rdi_addconfig_proc(RDI_AgentHandle agent, unsigned32 nbytes);


/*
 * Function:    RDI_LoadConfigDataProc
 *  Purpose:    Loads the config block.
 *              The block specifies target-dependent information to the
 *              debuggee. (e.g. ICEman)
 *              Only use if RDIInfo_DownLoad returns no error.
 *
 *  Params:
 *      Input:  agent   handle identifies debug agent
 *
 *              nBytes  size of block (as declared in addconfig)
 *
 *              data    source of block
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  RDIError_UnimplementedMessage (unimplemented functions)
 */
typedef int rdi_loadconfigdata_proc(
    RDI_AgentHandle agent, unsigned32 nbytes, char const *data);


/*
 * Function:    RDI_SelectConfigProc
 *  Purpose:    Selects which of the loaded config blocks should be used
 *              and then reinitialises the Debuggee to use the selected
 *              config data.
 *              Only use if RDIInfo_DownLoad returns no error.
 *
 *  Params:
 *      Input:  mh          handle identifies processor
 *                          > ISSUE < Specification says 'processor', but
 *                          implmentation says 'agent'.  Surely processor
 *                          is required, in case the aspect = ConfigCPU.
 *
 *              aspect      RDI_ConfigCPU or RDI_ConfigSystem
 *
 *              name        name of the configuration to be selected
 *
 *              matchtype   specifies exactly how the version number
 *                          must be matched
 *                              RDI_MatchAny
 *                              RDI_MatchExactly
 *                              RDI_MatchNoEarlier
 *
 *              versionreq  the version number requested
 *
 *      Output: *versionp   the version actually selected
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  RDIError_UnimplementedMessage (unimplemented functions)
 */
typedef int rdi_selectconfig_proc(
    RDI_AgentHandle, RDI_ConfigAspect aspect, char const *name,
    RDI_ConfigMatchType matchtype, unsigned versionreq, unsigned *versionp);

/*
 * Function:    RDI_LoadAgentProc
 *  Purpose:    Loads a replacement EmbeddedICE ROM image, and starts it
 *              (in RAM).
 *              Only if RDIInfo_Target returns a value with
 *              RDITarget_CanReloadAgent set.
 *
 *  Params:
 *      Input:  agent       handle identifies agent
 *      Input:  dest        address in the Debuggee's memory where the new
 *                          version will be put
 *      Input:  size        size of the new version
 *      Input:  getb        a function that can be called (with getbarg as
 *                          the first argument) and the number of bytes to
 *                          down load as the second argument.
 *      InOut:  none
 *      Output: none
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  RDIError_UnimplementedMessage (unimplemented functions)
 */
typedef char *getbufferproc(void *getbarg, unsigned32 *sizep);
typedef int rdi_loadagentproc(
    RDI_AgentHandle, ARMword dest, unsigned32 size, getbufferproc *getb,
    void *getbarg);


/*
 * Function:    RDI_TargetIsDeadProc
 *  Purpose:    Used to inform local device drivers that a remote
 *              (typically hardware) debuggee is unresponsive.
 *
 *  Params: 
 *      Input:  agent
 *
 *  Returns:
 *      OK:     RDIError_NoError
 *      Error:  RDIError_UnimplementedMessage
 */
typedef int rdi_targetisdead(RDI_AgentHandle agent);


/*
 * Function:    RDI_ErrMessProc
 *  Purpose:    Gets an error message corresponding to the error number.
 *
 *  Params:
 *      Input:  agent
 *      Input:  buflen      length of the buffer
 *      Input:  errnum      the error number
 *      InOut:  none
 *      Output: buf         destination for the message
 *
 *  Returns:    errmessproc returns the number of bytes written,
                rather than an RDIError
 */
typedef int rdi_errmessproc(RDI_AgentHandle agent, char *buf, int buflen, 
                            int errnum);

#else
typedef int rdi_addconfig_proc(unsigned32 nbytes);
typedef int rdi_loadconfigdata_proc(
    unsigned32 nbytes, char const *data);
typedef int rdi_selectconfig_proc(
    RDI_ConfigAspect aspect, char const *name,
    RDI_ConfigMatchType matchtype, unsigned versionreq, unsigned *versionp);

typedef char *getbufferproc(void *getbarg, unsigned32 *sizep);
typedef int rdi_loadagentproc(
    ARMword dest, unsigned32 size, getbufferproc *getb,
    void *getbarg);
typedef int rdi_targetisdead(void);

/* errmessproc returns the number of bytes written, rather than an RDIError */
typedef int rdi_errmessproc(char *buf, int buflen, int errnum);

#endif

    /* Added for Tools 2.11 ADP support */
typedef void rdi_onerror_fn(const void *device, int errtyp, void *data);
    /* End Tools 2.11 ADP support */

typedef struct {
    int itemmax;
    char const * const *names;
} RDI_NameList;

#if RDI_VERSION == 150
/*
 * Function:    RDI_CpuNamesProc and RDI_DriverNamesProc
 *  Purpose:    Get a list of names.
 *              The returned structure contains the number of names
 *              and a list of pointers to the sz names.
 *              The index to the chosen name is used in:
 *                  Dbg_ConfigBlock.procesor    (RDI 1)
 *                  Dbg_ConfigBlock.drivertype
 *                  RDI_ModuleDesc.procesor     (RDI 2)
 *              This is used to select a harware driver for the target
 *              or a processor from a processor family.
 *              E.g. If the processor family was PROCESSOR_ARM
 *              the list would show the members of that family
 *              (ARM2, ARM3, ARM6, ARM7 etc.)
 *  Params:
 *      Input:  handle      handle identifies processor in cpunames
 *                          RDINoHandle for drivernames
 *                          (i.e. target hadware configuration)
 *      InOut:  none
 *      Output: none
 *
 *  Returns:
 *      OK:     RDI_NameList
 *      Error:  none
 */
typedef RDI_NameList const *rdi_namelistproc(RDI_AgentHandle handle);
#else
typedef RDI_NameList const *rdi_namelistproc(void);
#endif

struct RDIProcVec
{
    char rditypename[12];

#if RDI_VERSION == 150
    rdi_openagent_proc  *openagent;
    rdi_closeagent_proc *closeagent;
#endif
    rdi_open_proc       *open;
    rdi_close_proc      *close;
    rdi_read_proc       *read;
    rdi_write_proc      *write;
    rdi_CPUread_proc    *CPUread;
    rdi_CPUwrite_proc   *CPUwrite;
    rdi_CPread_proc     *CPread;
    rdi_CPwrite_proc    *CPwrite;
    rdi_setbreak_proc   *setbreak;
    rdi_clearbreak_proc *clearbreak;
    rdi_setwatch_proc   *setwatch;
    rdi_clearwatch_proc *clearwatch;
    rdi_execute_proc    *execute;
    rdi_step_proc       *step;
    rdi_info_proc       *info;
    /* V2 RDI */
    rdi_pointinq_proc   *pointinquiry;

    /* These three useable only if RDIInfo_DownLoad returns no error */
    rdi_addconfig_proc  *addconfig;
    rdi_loadconfigdata_proc *loadconfigdata;
    rdi_selectconfig_proc *selectconfig;

    rdi_namelistproc    *drivernames;
    rdi_namelistproc    *cpunames;

    rdi_errmessproc     *errmess;

    /* Only if RDIInfo_Target returns a value with RDITarget_LoadAgent set */
    rdi_loadagentproc   *loadagent;
    rdi_targetisdead    *targetisdead;
};


#endif
