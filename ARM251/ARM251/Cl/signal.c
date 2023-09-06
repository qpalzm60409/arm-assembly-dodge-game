/* signal.c: ANSI draft (X3J11 Oct 86) library code, section 4.7 */
/* Copyright (C) Codemist Ltd, 1988                              */
/* Copyright (C) Advanced Risc Machines Ltd., 1991-1998.         */ 
/* All rights reserved. */

/*
 * RCS $Revision: 1.4 $
 * Checkin $Date: 1998/06/04 09:53:20 $
 * Revising $Author: mnonweil $
 */

/* N.B. machine dependent messages (only) below. */

#include "hostsys.h"
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "interns.h"

#define SIGLAST 11  /* one after highest signal number (see <signal.h>) */

static void (*_signalvector[SIGLAST+1])(int);

/* Turn off stack checking in the signal handlers */
#pragma -s1

extern void __ignore_signal_handler(int sig)
{
    /* do this in case called because of SharedCLibrary botch... */
    signal(sig, SIG_IGN);
}

static void __default_signal_handler(int sig)
{   const char *s = NULL;
    switch (sig)
    {
case SIGABRT:   s = "Abnormal termination (e.g. abort() function)";
                break;
case SIGILL:    s = "Illegal instruction (call to non-function/code corrupted)"
                    "\n[is the floating point emulator installed?]";
                break;
case SIGINT:    s = "Interrupt received from user - program terminated";
                break;
case SIGSEGV:   s = "Illegal address (e.g. wildly outside array bounds)";
                break;
case SIGTERM:   s = "Termination request received";
                break;
case SIGUSR1:
case SIGUSR2:   s = "User-defined signal";
                break;
case SIGSTAK:
                _init_flags.error_recursion = 1; /* lie to prevent attempt to use stdio */
                _sys_msg("Stack overflow\n", NL_PRE+NL_POST);
                __rt_exit(100);
                break;
default:        /* Note: including SIGFP */
                s = _hostos_signal_string(sig);
                break;
    }
    _sys_msg(s, NL_PRE+NL_POST);        /* ensure out even if stderr problem */
    if (sig == SIGINT)
        __rt_exit(1);
    else
        _postmortem();
}

int raise(int sig)
{
    void (*handler)(int);
    if (sig<=0 || sig>=SIGLAST) return (errno = ESIGNUM);
    handler = _signalvector[sig];
    if (handler==__SIG_DFL)
        __default_signal_handler(sig);
    else if (handler!=__SIG_IGN)
    {   
        _signalvector[sig] = __SIG_DFL;
        handler(sig);
    }
    return 0;
}

/* Turn stack check back on (if we are compiling with /swst) */
#pragma -s0

void (*signal(int sig, void (*func)(int)))(int)
{
    void (*oldf)(int);
    if (sig<=0 || sig>=SIGLAST) return __SIG_ERR;
    oldf = _signalvector[sig];
    _signalvector[sig] = func;
    return oldf;
}

void _signal_init()
{
    int i;
    /* do the following initialisation explicitly so code restartable */
    for (i=1; i<SIGLAST; i++) signal(i, __SIG_DFL);
}

/* end of signal.c */
