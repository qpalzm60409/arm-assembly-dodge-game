/*
 * ARM RDI - RDIInfo codes: rdi_info.h
 * Copyright (C) 1998 Advanced RISC Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.7.4.2 $
 * Checkin $Date: 1998/10/29 15:01:27 $
 * Revising $Author: bclarke $
 */

#ifndef rdi_info_h
#define rdi_info_h

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

#define RDIInfo_ReadClock       0x8012
/*
 * Capability request for RDIRead_Clock
 * Returns: RDIError_NoError if read clock is allowed,
 *          RDIError_UnimplementedMessage otherwise
 */

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

/*
 * Function:    RDI_InfoProc(mh, RDIInfo_Modules, arg1, arg2)
 *  Version:    RDI 1.5
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


/*
 *  Function: RDI_InfoProc(mh, RDIInfo_Modifiable, arg1, arg2)
 *   Version: RDI 1.5
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
 *   Version: RDI 1.5
 *   Purpose: Query the target clock speed
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

/*  Function: RDI_InfoProc(mh, RDISemiHosting_DCC, arg1, arg2)
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


/*  Function: RDI_InfoProc(ah, RDIInfo_SynchronizationControl, arg1, arg2)
 *   Purpose: Determine the sequence of "stop_others" flags to pass to
 *            the debug target to perform synchronous execution in
 *            multiprocessor systems. Normally, this will be "always
 *            TRUE" for asynchronous, "always FALSE" for
 *            synchronous. However some targets--e.g. in the presence
 *            of multiple controllers--may instead wish the controller
 *            to use "occasionally FALSE" in the second time. For
 *            instance, a controller may generate a sequence of
 *            steps/gos for a single user request to step or go, and
 *            the target will need to be told "FALSE" for only one of
 *            these (typically, the last).
 *            The controller may abort before reaching the "user" request,
 *            for example, if it hits a breakpoint.
 *
 *   Params:
 *      Input: ah          handle identifies controller
 *      Input: type        RDIInfo_SynchronizationControl
 *      Input: char *arg1  Pointer to an array of four bytes to be filled
 *                         in as below
 *
 *   The array pointed to by (arg1) is filled in with:
 *      arg1[0] => boolean value to pass for stop_others for controller
 *                 generated requests for synchronous execution
 *      arg1[1] => boolean value to pass for stop_others for user
 *                 generated requests for synchronous execution
 *      arg1[2] => boolean value to pass for stop_others for controller
 *                 generated requests for asynchronous execution
 *      arg1[3] => boolean value to pass for stop_others for user
 *                 generated requests for asynchronous execution
 *
 *   Returns:   RDIError_NoError               Request performed OK
 *              RDIError_UnimplementedMessage  Target ignores "stop_others"
 */
#define RDIInfo_SynchronizationControl  41


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

/* These 4 are only to be used if RDIInfo_Semihosting returns no error, or
 * the relevant RDIInfo_SemiHosting_... returns no error
 */
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

#define RDIInfo_SemiHostingSetARMSWI 0x8190
#define RDIInfo_SemiHostingGetARMSWI 0x8191
#define RDIInfo_SemiHostingSetThumbSWI 0x8192
#define RDIInfo_SemiHostingGetThumbSWI 0x8193
/*
 * Return RDIError_NoError if the function is servicable, or 
 * RDIError_UnimplementedMessage otherwise
 */

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

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_GetCPAccessCodeAddress, arg1, arg2)
 *   Purpose: Get the address of the area of memory (guarenteed at least
 *            40 bytes) which can be used by the Debug Agent (specifically
 *            Multi-ICE) to read / write coprocessor state.  The memory
 *            at this address must be RAM, and must be readable and writeable.
 *            This memory is however not corrupted, so the user may use this
 *            area of memory for other purposes too.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_GetCPAccessCodeAddress
 *
 *              *arg1   is used to return the address
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_GetCPAccessCodeAddress 0x19E

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_SetCPAccessCodeAddress, arg1, arg2)
 *   Purpose: Set the address of the area of memory (guarenteed at least
 *            40 bytes) which can be used by the Debug Agent (specifically
 *            Multi-ICE) to read / write coprocessor state.  The memory
 *            at this address must be RAM, and must be readable and writeable.
 *            This memory is however not corrupted, so the user may use this
 *            area of memory for other purposes too.
 *
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_SetCPAccessCodeAddress
 *
 *              *arg1   is the new address
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_SetCPAccessCodeAddress 0x19F


/*
 *  Function: RDI_InfoProc(mh, RDIInfo_GetARM9RestartCodeAddress, arg1, arg2)
 *   Purpose: Get the address of the area of memory (guarenteed at least
 *            32 bytes) which can be used by the Debug Agent (specifically
 *            Multi-ICE) to restart the processor correctly.  The memory
 *            at this address must be RAM, and must be readable and writeable.
 *            This memory is dedicated to use by the Debug Agent, and cannot
 *            be used for any other purpose.
 *            In practice this will only be used for ARM9T rev 0 based targets.
 *            
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_GetARM9RestartCodeAddress
 *
 *              *arg1   is used to return the address
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_GetARM9RestartCodeAddress 0x1A0

/*
 *  Function: RDI_InfoProc(mh, RDIInfo_SetARM9RestartCodeAddress, arg1, arg2)
 *   Purpose: Set the address of the area of memory (guarenteed at least
 *            32 bytes) which can be used by the Debug Agent (specifically
 *            Multi-ICE) to restart the processor correctly.  The memory
 *            at this address must be RAM, and must be readable and writeable.
 *            This memory is dedicated to use by the Debug Agent, and cannot
 *            be used for any other purpose.
 *            In practice this will only be used for ARM9T rev 0 based targets.
 *            
 *    Params:
 *       Input: mh      handle identifies processor
 *
 *              type    RDIInfo_SetARM9RestartCodeAddress
 *
 *              *arg1   is the new address
 *
 *   This is also supported in RDI1, in which case mh is left out.
 *
 *   Returns:   RDIError_NoError
 */
#define RDIInfo_SetARM9RestartCodeAddress 0x1A1

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

/*
 * Inquiry for RDIRequestCyclesDesc
 * RDIError_NoError => I support this
 * RDIError_UnimplementedMessage => I don't support this
 */
#define RDIInfo_RequestCyclesDesc 0x8202

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

/*
 * InfoProc: RDI_InfoProc(mh, RDIInfo_Trace, arg1, arg2)
 *
 * Purpose:  Inquire whether a processor on a Debug Target supports
 *           tracing. A Debug Controller should call this Info call
 *           before trying to use the other Trace calls.
 *
 * Base of RDITrace info calls - see rdi_rti.h for actual calls
 * RDIInfo_Trace -> RDIInfo_Trace + 0xff are reserved for trace.
 *
 * Params:   mh  handle identifies processor
 *
 * Return:   RDIError_NoError              Processor supports tracing.
 *           RDIError_UnimplementedMessage Processor does not support tracing.
 */
#define RDIInfo_Trace 0x600

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

/*
 * HGB 6-May-1998
 *
 * RDIInfo_CapabilityRequest is deprecated.  To use this facility for
 * RDI info code 0xabc, explicitly define a capability request of 0x8abc,
 * and name it.  See RDIInfo_ReadClock for an example.
 */

/*
 * HGB 12-May-1998
 *
 * Unfortunately, some parts of the world are still using
 * RDIInfo_CapabilityRequest.  So, for now we continue to define
 * RDIInfo_CapabilityRequest.  This must be obsoleted soon, since
 * the current set of RDI targets do not support it consistently :(
 *
 * #ifdef OldCode
 */

#define RDIInfo_CapabilityRequest 0x8000
/* Request whether the interface supports the named capability. The     */
/* capability is specified by or'ing the RDIInfo number with this, and  */
/* sending that request                                                 */
/* rdi: in none                                                         */
/* rdp: in none, out byte status                                        */

/*
 * HGB 12-May-1998.  See above
 *
 * #endif
 */

#endif /* rdi_info_h */
