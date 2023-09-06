/* -*-C-*-
 *
 * $Revision: 1.3 $
 *   $Author: rivimey $
 *     $Date: 1998/08/06 16:49:39 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debugging via panic block - header file.
 */

#ifndef angel_panicblk_h
#define angel_panicblk_h

#include "logging.h"

int panicmsg(void);

int panicblk_PreWarn(WarnLevel l);

void panicblk_PostWarn(int n);

int panicblk_PutChar(char c);


#endif /* ndef angel_panicblk_h */

/* EOF panicblk.h */
