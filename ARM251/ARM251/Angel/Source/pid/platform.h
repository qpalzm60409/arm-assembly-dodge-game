/* -*-C-*-
 *
 * $Revision: 1.2 $
 *   $Author: rivimey $
 *     $Date: 1998/08/06 16:54:44 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Hardware description of the PID board.
 */

#ifndef pid_platform_h
#define pid_platform_h

#include "pid.h"
#include "st16c552.h"

#define DEBUG_COMPORT   NISA_SER_B

/* Macros for logging */
#define LOG_GET_STATUS(p)       (((ST16C552Reg *)p)->lsr)
#define LOG_TX_EMPTY(p)         ((((ST16C552Reg *)p)->lsr & (1 << 6)) == 0)     
#define LOG_TX_READY(s)         ((s & (1 << 5)) != 0)   
#define LOG_GET_CHAR(p)         (((ST16C552Reg *)p)->rhr)
#define LOG_PUT_CHAR(p, c)      (((ST16C552Reg *)p)->thr = (c))

#define LOG_RX_DATA(s)          ((s & (1 << 0)) != 0)   

#define LOG_READ_INTERRUPT(p)   \
                (((ST16C552Reg *)p)->isr & 0x01 ? ((ST16C552Reg *)p)->isr : 0)
#define LOG_RX_INTERRUPT        4
#define LOG_TX_INTERRUPT        2

#endif
