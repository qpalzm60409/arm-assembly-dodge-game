/* -*-C-*-
 *
 * $Revision: 1.3.2.1 $
 *   $Author: rivimey $
 *     $Date: 1998/09/22 13:46:33 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via writes to serial port.
 */

#ifndef angel_logterm_h
#define angel_logterm_h

#include "logging.h"
#include "logpriv.h"
#include "logserial.h"
#include "stdarg.h"

#ifdef RAW_ST16C552_B
# if RAW_ST16C552_B == 1
#  error Serial chip B in use!
# endif
#endif

#define LOGTERM_PORT  LOGSERIAL_PORT

bool logterm_PreWarn(WarnLevel level);
int  logterm_PutChar(char c);
void logterm_PostWarn(unsigned int len);

void logterm_PutString(char *s);
void logterm_fatalhandler(void);

#ifndef DEFBAUD
#define DEFBAUD               115200              /* default baud rate */
#endif

void angel_LogtermIntHandler(unsigned int ident, unsigned int devid,
                             unsigned int empty_stack);

bool logterm_Initialise(void);

struct LogSaveBuffer *log_getlogtermbuf(void);

#endif /* ndef angel_logterm_h */

/* EOF logterm.h */
