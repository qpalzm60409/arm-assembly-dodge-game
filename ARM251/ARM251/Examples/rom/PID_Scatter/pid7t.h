/******************************************************************************
**                                                                           **
**  Copyright (c) 1996 Advanced Risc Machines Limited                        **
**  All rights reserved                                                      **
**                                                                           **
**      Filename  :  pid7t.h                                                 **
**      Author    :  Dave Brooke                                             **
**      Revision  :  1.0                                                     **
**                                                                           **
**                                                                           **
**                                                                           **
*******************************************************************************/
#ifndef __PID7TDEF
#define __PID7TDEF

/*******************************************************************************/
/*  Base addresses for standard memory-mapped peripherals                      */
/*******************************************************************************/
#define IntBase         0x0A000000  /* Interrupt Controller Base       */
#define TimerBase       0x0A800000  /* Counter/Timer Base              */
#define ResetBase       0x0B000000  /* Reset and Pause Controller Base */

/*******************************************************************************/
/*  Now load in definitions from Reference Peripheral Specification            */
/*******************************************************************************/
#include "rps.h"

/*******************************************************************************/
/*  Add extra sources defined for PID7T card Interrupt Controller              */
/*******************************************************************************/
#define IRQCardA        0x0040
#define IRQCardB        0x0080
#define IRQSerialA      0x0100
#define IRQSerialB      0x0200
#define IRQParallel     0x0400
#define IRQASBex0       0x0800
#define IRQASBex1       0x1000
#define IRQAPBex0       0x2000
#define IRQAPBex1       0x4000
#define IRQAPBex2       0x8000

/*******************************************************************************/
/* FIQ source register                                                         */
/*******************************************************************************/
#define FIQSourceIRQ   ((volatile unsigned *)(IntBase + 0x114))

/*******************************************************************************/
/* Counter/Timer definitions as per RPS - defined in rps.h                     */
/*******************************************************************************/

/*******************************************************************************/
/* Reset and Wait definitions as per RPS - defined in rps.h                    */
/*******************************************************************************/

#endif

