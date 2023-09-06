/* -*-C-*-
 *
 * $Revision: 1.2 $
 *   $Author: rivimey $
 *     $Date: 1998/08/06 16:53:18 $
 *
 * Copyright (c) 1997 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface via Serial port (target dependant).
 *              PID uses st16c552 port B
 */

#include "channels.h"
#include "debug.h"
#include "logging.h"
#include "logserial.h"
#include "platform.h"

static char  logserial_buf[128];
static char *logserial_pos;
static char *logserial_end;
static WarnLevel logserial_level;
static short logserial_firstmsg = TRUE;
static short logserial_deferred_prefix = FALSE;
static int   logserial_minlevel; /* use dbg to set this to 0 for all msgs up to 3 for errors only */

/*
 * NT's HyperTerminal needs CRLF, not just LF!
 */
#define LOGSERIAL_ADD_CR_TO_LF

#include "banner.h"

/* #define LOGONMESSAGE "\n\nAngel Debug Monitor\n\n" */
#define LOGONMESSAGE "\n\n" ANGEL_BANNER "\n\n"

/*
 * prefix for lines not at start of message -- see stuff in putchar too
 */
#define PREFIXLEN   6

/*
 * prototypes
 */

void logserial_flushbuf(void);


bool logserial_PreWarn(WarnLevel level)
{
    char *p;

    logserial_Reset(LOGSERIAL_PORT, BAUDVALUE);

    /*
     * set up the buffer pointers... reset in flushbuf
     */
    logserial_pos = logserial_buf;
    logserial_end = (logserial_buf + sizeof(logserial_buf) - 1);

    if (logserial_firstmsg)
    {
        /*
         * print a logon banner to say we're here!
         */
        p = LOGONMESSAGE;
        while(*p)
            logserial_PutChar(*p++);

        logserial_flushbuf();

        logserial_firstmsg = FALSE;
        logserial_deferred_prefix = TRUE;
        logserial_minlevel = 0;
    }
    else if (logserial_level != level && !logserial_deferred_prefix)
    {
        logserial_PutChar('\n');
    }
    logserial_level = level;

    /*if (level >= logserial_minlevel)
        return FALSE; */
    return TRUE;
}

void logserial_flushbuf(void)
{
    char *p;
    
    p = logserial_buf;
    while(p < logserial_pos)
    {
        /* Now wait for UART to drain */
        while( ! LOG_TX_EMPTY(LOGSERIAL_PORT) )
                ;

        LOG_PUT_CHAR(LOGSERIAL_PORT, *p);
        p++;
    }

    logserial_pos = logserial_buf;
    logserial_end = logserial_buf + sizeof(logserial_buf) - 1;
}


int logserial_PutChar(char c)
{
    if (logserial_deferred_prefix == TRUE && logserial_firstmsg == FALSE)
    {
        char *p;
        if ((logserial_pos + PREFIXLEN) >= logserial_end)
            logserial_flushbuf();

        switch(logserial_level)
        {
        case WL_TRACE:
            p = "Trace:";
            while(*p != '\0')
                *logserial_pos++ = *p++;
            break;
        case WL_INFO:
            p = "Info :";
            while(*p != '\0')
                *logserial_pos++ = *p++;
            break;
        case WL_WARN:
            p = "Warn :";
            while(*p != '\0')
                *logserial_pos++ = *p++;
            break;
        case WL_ERROR:
            p = "Error:";
            while(*p != '\0')
                *logserial_pos++ = *p++;
            break;
        }

        logserial_deferred_prefix = FALSE;
    }
    else
    {
        if (logserial_pos >= logserial_end)
            logserial_flushbuf();
    }

    if (c == '\n')
    {
#ifdef LOGSERIAL_ADD_CR_TO_LF
        *logserial_pos++ = '\r';
        
        if (logserial_pos >= logserial_end)
            logserial_flushbuf();
#endif
        logserial_deferred_prefix = TRUE;
    }
    
    *logserial_pos++ = c;

    return 0;
}

void logserial_PostWarn(unsigned int len)
{
    IGNORE(len) ;

    logserial_flushbuf();
}

#pragma check_stack

/* EOF logserial.c */
