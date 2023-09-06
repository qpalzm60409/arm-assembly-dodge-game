/* -*-C-*-
 *
 * $Revision: 1.2.2.1 $
 *   $Author: rivimey $
 *     $Date: 1998/09/22 13:46:32 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via Serial writes to 16c552 serial port B.
 */

#ifndef angel_logserial_h
#define angel_logserial_h

#include "logging.h"
#include "logpriv.h"

#define DEFBAUD               115200              /* default baud rate */

#if DEFBAUD == 9600
# define BAUDVALUE Baud9600

#elif DEFBAUD == 19200
# define BAUDVALUE Baud19200

#elif DEFBAUD == 38400
# define BAUDVALUE Baud38400

#elif DEFBAUD == 57600
# define BAUDVALUE Baud57600

#elif DEFBAUD == 115200
# define BAUDVALUE Baud115200

#else
# error invalid baud rate
#endif

#pragma no_check_stack

#ifdef RAW_ST16C552_B
# if RAW_ST16C552_B == 1
#  error Serial chip B in use!
# endif
#endif

#define LOGSERIAL_PORT  DEBUG_COMPORT


bool logserial_PreWarn(WarnLevel level);

int  logserial_PutChar(char c);

void logserial_PostWarn(unsigned int len);


#endif /* ndef angel_logserial_h */

/* EOF logserial.h */
