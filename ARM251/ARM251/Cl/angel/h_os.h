/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: mnonweil $
 *     $Date: 1998/06/03 18:35:44 $
 *
 * Copyright (c) 1996-1998 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * THESE ARE DUPLICATES OF DEFINITIONS IN h_os.s.  CHANGE AT YOUR PERIL!
 */

#ifndef h_os_h
#define h_os_h

#include "config.h"

/*
 * Angel Undefined Instructions and SWI's
 *
 * Angel requires one SWI and one Undefined Instruction for operation in
 * ARM state (as opposed to Thumb state).  The SWI is used whenever it
 * is possible to pass arguments to Angel - eg. for semihosting support,
 * whereas the Undefined Instruction is used solely for breakpoints.
 *
 * Refer to the Spec for full discussion of this area.
 */

/*
 * We choose a different Undefined Instr depending on whether we are
 * on a little or big endian system.  Great care should be taken
 * before changing the choice of undefined instructions.
 */

#ifdef HOSTSEX_l
# define LITTLE_ENDIAN
#endif

#ifdef HOSTSEX_b
# define BIG_ENDIAN
#endif

#ifndef ENDIAN_DONT_CARE
#  ifdef BIG_ENDIAN
#    define angel_BreakPointInstruction_ARM       (0xE7FFDEFE)
#  else
#    ifdef LITTLE_ENDIAN
#      define angel_BreakPointInstruction_ARM     (0xE7FDDEFE)
#    else
#      error Must define either BIG_ENDIAN or LITTLE_ENDIAN
#    endif
#  endif
#endif

#define angel_BreakPointInstruction_THUMB (0xDEFE)

/*
 * If the SWIs chosen here are inconvenient it may be changed to be any
 * other SWI.
 *
 * We have a SWI for ARM state code and a SWI for Thumb code too
 */
#define angel_SWI_ARM       (0x123456)

#define angel_SWI_THUMB     (0xAB)

/*
 * The reason codes (passed in r0) may also be modified should they
 * be inconvenient.  Note that the exception handler will need to
 * be modified suitably too however.
 */
#define angel_SWIreasonBase  0x01

/*
 * The following reason codes are used by Angel as part of the C library
 * support.  The C library calls into Angel by using a SWI with the
 * following reason codes.
 */
#define angel_SWIreason_CLibBase  0x01

/*
 * The following are the C Library reason codes ...
 */
#define SYS_OPEN          0x1
#define SYS_CLOSE         0x2
#define SYS_WRITEC        0x3
#define SYS_WRITE0        0x4
#define SYS_WRITE         0x5
#define SYS_READ          0x6
#define SYS_READC         0x7
#define SYS_ISERROR       0x8
#define SYS_ISTTY         0x9
#define SYS_SEEK          0xA
#define SYS_ENSURE        0xB
#define SYS_FLEN          0xC
#define SYS_TMPNAM        0xD
#define SYS_REMOVE        0xE
#define SYS_RENAME        0xF
#define SYS_CLOCK         0x10
#define SYS_TIME          0x11
#define SYS_SYSTEM        0x12
#define SYS_ERRNO         0x13
/* 0x14 was the now defunct SYS_INSTALL_RAISE */
#define SYS_GET_CMDLINE   0x15
#define SYS_HEAPINFO      0x16

#define angel_SWIreason_CLibLimit 0x16

/*
 * The following reason code is used to get the calling code into SVC
 * mode:
 */
#define angel_SWIreason_EnterSVC 0x17

/*
 * The following reason code is used by the C Library support code and
 * / or the C library itself to request that a particular exception is
 * reported to the debugger.  This will only be used when it has been
 * checked that there is no appropriate signal handler installed.  r1
 * must contain one of ADP_Stopped subreason codes which is to be
 * reported.
 */
#define angel_SWIreason_ReportException 0x18

/*
 * The following reason code is used by devappl.c in a full Angel
 * system when the application wants to make use of the device
 * drivers.
 *
 * In addition, values are defined for specifying whether the
 * ApplDevice call is Read, Write or Yield.  These are sub-parameters
 * to angel_SWIreason_ApplDevice
 */
#define angel_SWIreason_ApplDevice 0x19

#define angel_SWIreason_ApplDevice_Read          0x0
#define angel_SWIreason_ApplDevice_Write         0x1
#define angel_SWIreason_ApplDevice_Control       0x2
#define angel_SWIreason_ApplDevice_Yield         0x3
#define angel_SWIreason_ApplDevice_Init          0x4

/*
 * The following reason code is used by the application in a late startup
 * world to indicate that it wants to attach to a debugger now
 */
#define angel_SWIreason_LateStartup 0x20

#define angel_SWIreasonLimit 0x20

/*
 * 24-Apr-98 HGB
 *
 * Two calls (SYS_ELAPSED and SYS_TICKFREQ) added to track target time
 *
 * Instigated by Dominic Symes, approved by Ruth Ivimey-Cook
 */

/*
 * Reason code: SYS_ELAPSED
 *     Purpose: Returns the number of elapsed target 'ticks' since the
 *              support code started executing.  Ticks are defined by
 *              SYS_TICKFREQ.  (If the target cannot define the length of
 *              a tick, it may still supply SYS_ELAPSED.)
 *
 *      Inputs: r0 = 0x30 (reason code)
 *              r1 = pointer to a doubleword in which to put the number of
 *                   elapsed ticks.  The first word in the doubleword is
 *                   the least significant word.  The last word is the
 *                   most significant word.  This follows the convention 
 *                   used by armcc's "long long" data type.
 *
 *     Outputs: r1: If (*(uint64 *)r1) does not contain the number of
 *                  elapsed ticks, r1 is set to -1.
 */
#define SYS_ELAPSED 0x30

/*
 * Reason code: SYS_TICKFREQ
 *     Purpose: Define a tick frequency.  (The natural implementation is
 *              to have a software model specify one core cycle as one tick,
 *              and for Angel to return Timer1 ticks)
 *
 *      Inputs: r0 = 0x31 (reason code)
 *
 *     Outputs: r0 = ticks per second, or -1 if the target does not know the
 *              value of one tick.
 */
#define SYS_TICKFREQ 0x31 


#endif /* !defined(h_os_h) */


/* EOF h_os.h */
