/* -*-C-*-
 *
 * $Revision: 1.5.2.2 $
 *   $Author: rivimey $
 *     $Date: 1998/10/02 18:50:59 $
 *
 * Copyright (c) 1995 Advanced RISC Machines Limited
 * All Rights Reserved.
 *
 * msgbuild.c - utilities for assembling and interpreting ADP messages
 *
 */

#include <stdarg.h>             /* ANSI varargs support */

#ifdef TARGET
#include "angel.h"
#include "devconf.h"
#else
#include <setjmp.h>

#include "ardi_defs.h"
#include "devsw.h"
#include "ardi.h"
#include "hostchan.h"
#endif

#include "channels.h"
#include "buffers.h"
#include "endian.h"             /* Endianness support macros */
#include "msgbuild.h"           /* Header file for this source code */

#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif

unsigned int 
vmsgbuild(unsigned char *buffer, const char *format, va_list args)
{
    unsigned int blen = 0;
    int ch;

    /* Step through the format string */
    while ((ch = *format++) != '\0')
    {
        if (ch != '%')
        {
            if (buffer != NULL)
                *buffer++ = (unsigned char)ch;

            blen++;
        }
        else
        {
            switch (ch = *format++)
            {
                case 'w':
                case 'W':
                    /* 32bit pointer */
                case 'p':
                case 'P':
                    {
                        /* 32bit word / 32bit pointer */
                        unsigned int na = va_arg(args, unsigned int);

                        if (buffer != NULL)
                        {
                            PUT32LE(buffer, na);
                            buffer += sizeof(unsigned int);
                        }

                        blen += sizeof(unsigned int);

                        break;
                    }

                case 'h':
                case 'H':
                    {
                        /* 16bit value */
                        unsigned int na = va_arg(args, unsigned int);

                        if (buffer != NULL)
                        {
                            PUT16LE(buffer, na);
                            buffer += sizeof(unsigned short);
                        }

                        blen += sizeof(unsigned short);

                        break;
                    }

                case 'c':
                case 'C':
                case 'b':
                case 'B':
                    /* 8bit character / 8bit byte */
                    ch = va_arg(args, int);

                    /*
                     * XXX
                     *
                     * fall through to the normal character processing
                     */

                case '%':
                default:
                    /* normal '%' character, or a different normal character */
                    if (buffer != NULL)
                        *buffer++ = (unsigned char)ch;

                    blen++;
                    break;
            }
        }
    }

    return blen;
}

/*
 * msgbuild
 * --------
 * Simple routine to aid in construction of Angel messages. See the
 * "msgbuild.h" header file for a detailed description of the operation
 * of this routine.
 */
unsigned int 
msgbuild(unsigned char *buffer, const char *format,...)
{
    va_list args;
    unsigned int blen;

    va_start(args, format);
    blen = vmsgbuild(buffer, format, args);
    va_end(args);

    return blen;
}

#if !defined(MSG_UTILS_ONLY)
/*
 * This routine allocates a buffer, puts the data supplied as
 * parameters into the buffer and sends the message. It does *NOT*
 * wait for a reply.
 * RIC 6/97: 
 */
int 
msgsend(ChannelID chan, const char *format,...)
{
    unsigned int length;
    p_Buffer buffer = NULL;
    va_list args;
    int err = 0;

#ifndef TARGET
    Packet *packet;

    packet = DevSW_AllocatePacket(AGENTVAR(bufferSize));
    if (packet != NULL)
        buffer = packet->pk_buffer;

#else
    buffer = angel_ChannelAllocBuffer(Angel_ChanBuffSize);
#endif

    if (buffer != NULL)
    {
        va_start(args, format);

        length = vmsgbuild(BUFFERDATA(buffer), format, args);

#ifdef TARGET
        angel_ChannelSend(CH_DEFAULT_DEV, chan, buffer, length);
        /* should check for errors */
#else
        packet->pk_length = length;
        err = Adp_ChannelWrite(chan, packet);
#endif

        va_end(args);
        return err;             /* this is an an adp_ error code, hopefully adp_ok */
    }
    else
        return -1;
}

#endif /* ndef MSG_UTILS_ONLY */

/*
 * unpack_message
 * --------------
 */
extern unsigned int 
unpack_message(unsigned char *buffer, const char *format,...)
{
    va_list args;
    unsigned int blen = 0;
    int ch;
    char *chp = NULL;

    va_start(args, format);

    /* Step through the format string. */
    while ((ch = *format++) != '\0')
    {
        if (ch != '%')
        {
            if (buffer != NULL)
                ch = (unsigned char)*buffer++;

            blen++;
        }
        else
        {
            switch (ch = *format++)
            {
                case 'w':
                case 'W':
                    {
                        /* 32bit word. */
                        unsigned int *nap = va_arg(args, unsigned int *);

                        if (buffer != NULL)
                        {
                            *nap = PREAD32(LE, buffer);
                            buffer += sizeof(unsigned int);
                        }

                        blen += sizeof(unsigned int);

                        break;
                    }

                case 'h':
                case 'H':
                    {
                        /* 16bit value. */
                        unsigned int *nap = va_arg(args, unsigned int *);

                        if (buffer != NULL)
                        {
                            *nap = PREAD16(LE, buffer);
                            buffer += sizeof(unsigned short);
                        }

                        blen += sizeof(unsigned short);

                        break;
                    }

                case 'c':
                case 'C':
                case 'b':
                case 'B':
                    /* 8-bit character, or 8-bit byte */
                    chp = va_arg(args, char *);

                    /*
                     * XXX
                     *
                     * fall through to the normal character processing.
                     */

                case '%':
                default:
                    /* normal '%' character, or a different normal character */
                    if (buffer != NULL)
                        *chp = (unsigned char)*buffer++;

                    blen++;

                    break;
            }
        }
    }

    va_end(args);
    return (blen);
}


/* EOF msgbuild.c */
