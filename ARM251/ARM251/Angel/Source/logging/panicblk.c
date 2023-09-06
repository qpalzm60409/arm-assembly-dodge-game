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
 *     Title: Debug interface via a small one-line block of memory
 */

#include "panicblk.h"
#ifdef DEBUG_EB285
#include "platform.h"
#endif

#define PANIC_SIZE   80
#define PANIC_NLINES 4

static char            panic_block[PANIC_NLINES][PANIC_SIZE];
static unsigned int    panic_index = 0;
static int             panic_msgno = 0;
static int             panic_minlvl = 0;
static int             panic_lastlvl = 0;
static int             panic_line = 0;

#pragma no_check_stack

int panicmsg(void)
{
    if (panic_lastlvl >= panic_minlvl)
        return 1;   /* this exists so you can breakpoint it! */
    return 0;
}

int panicblk_PreWarn(WarnLevel l)
{
    panic_lastlvl = l;
    
    return 1;
}

            
void panicblk_PostWarn(int n)
{
    n=n;
    
    
    panicmsg();
}


int panicblk_PutChar(char c)
{
    int i;
    
#ifdef DEBUG
#if DEBUG != 0
#ifdef DEBUG_EB285
#if DEBUG_EB285 != 0
    putchar(c) ;
#endif
#endif
#endif
#endif

    if (panic_index < PANIC_SIZE - 1)
        panic_block[panic_line][panic_index++] = c;

    if (c == '\n')
    {

#ifdef DEBUG
#if DEBUG != 0
#ifdef DEBUG_EB285
#if DEBUG_EB285 != 0
        put(panic_msgno) ;
        putchar('|') ;
#endif
#endif
#endif
#endif

        panic_block[panic_line][panic_index] = '\0';
        panic_line++;
        if (panic_line >= PANIC_NLINES)
            panic_line = 0;

        for(i = 0; i < PANIC_SIZE; i++)
            panic_block[panic_line][i] = '\0';
        
        panic_block[panic_line][0] = '@' + (panic_msgno++ & 0x1F);
        panic_index = 1;

    }
    return c;
}


/* EOF panicblk.c */
