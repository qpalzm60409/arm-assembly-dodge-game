/* -*-C-*-
 *
 * $Revision: 1.8.2.3 $
 *   $Author: tdouglas $
 *     $Date: 1998/10/20 19:14:14 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: The message to send up when we boot
 */

#ifndef angel_banner_h
#define angel_banner_h

#include "toolver.h"

/* convenience definitions of various banner strings */
#include "configmacros.h"

#undef SER_STR
#if ST16C552_NUM_PORTS == 1
# define SER_STR "Serial(x1)"
#else
# if HANDLE_INTERRUPTS_ON_IRQ == 0
#  define SER_STR "Serial"
# else
#  define SER_STR "Serial(x2)"
# endif
#endif

/* PID cannot do anything but a single serial port with just FIQ */
#if HANDLE_INTERRUPTS_ON_IRQ == 0
#undef PRF_STR
#undef DCC_STR
#undef PAR_STR
#undef ETH_STR
#endif

#define ANGEL_BANNER ANGEL_NAME " V" TOOLVER_ANGEL " for PID\n" ANGEL_CONFIG "\n" BUILD_STRING "\n"

#endif

 /* EOF banner_h */
