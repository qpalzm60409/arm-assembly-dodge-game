/* -*-C-*-
 *
 * $Revision: 1.10.2.2 $
 *   $Author: rivimey $
 *     $Date: 1998/10/15 18:00:27 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Serial device driver for PID board, derived from PIE
 *              board serial driver.
 */
#ifndef angel_st16c552_h
#define angel_st16c552_h

#include "devdriv.h"
#include "serring.h"

#define ST16C552_IDENT_A        0
#define ST16C552_IDENT_B        1

extern const struct angel_DeviceEntry angel_ST16C552Serial[];

/*
 * Layout of the Serial controller; from our perspective, each
 * byte-wide register in the serial chip is on a word boundary
 *
 * 960604 KWelton
 *
 * On a big-endian machine, we need to offset I/O addresses by 3
 * (unfortunately, there is no byte-lane steering on APB, so we
 * have to do it here in software =8( ).
 */
typedef struct ST16C552Reg
{
#ifdef BIG_ENDIAN
    volatile unsigned char _BEpad[3];
#endif
    volatile unsigned char rhrthr;
    volatile unsigned char _pad0[3];
    volatile unsigned char ier;
    volatile unsigned char _pad1[3];
    volatile unsigned char isrfcr;
    volatile unsigned char _pad2[3];
    volatile unsigned char lcr;
    volatile unsigned char _pad3[3];
    volatile unsigned char mcr;
    volatile unsigned char _pad4[3];
    volatile unsigned char lsr;
    volatile unsigned char _pad5[3];
    volatile unsigned char msr;
    volatile unsigned char _pad6[3];
    volatile unsigned char spr;
} ST16C552Reg;

#define rhr             rhrthr
#define thr             rhrthr
#define isr             isrfcr
#define fcr             isrfcr

#define dll             rhrthr
#define dlm             ier

/*
 * interrupt bit masks
 */
#define RxReadyInt      (1 << 0)
#define TxReadyInt      (1 << 1)
#define RxLineInt       (1 << 2)
#define ModemInt        (1 << 3)

/*
 * Line Status Register bits
 */
#define LSRRxData       (1 << 0)
#define LSROverrun      (1 << 1)
#define LSRParity       (1 << 2)
#define LSRFraming      (1 << 3)
#define LSRBreak        (1 << 4)
#define LSRFIFOEmpty    (1 << 5)  /* FIFO (holding reg) is empty */
#define LSRTxEmpty      (1 << 6)  /* FIFO (holding reg) and shift reg empty */
#define LSRFIFOError    (1 << 7)

#define FCRFIFOEnable   0x01
#define FCRClearRX      0x02
#define FCRClearTX      0x04
#define FCRMode1        0x08
#define FCRRxTrig01     0x00
#define FCRRxTrig04     0x40
#define FCRRxTrig08     0x80
#define FCRRxTrig14     0xC0

#define MCRForceDTR     0x01
#define MCRForceRTS     0x02
/* bit 2 not used */
#define MCREnableINT    0x08
#define MCREnableLoop   0x10
/* bits 5-7 not used */

#define LCRWord5        0x00
#define LCRWord6        0x01
#define LCRWord7        0x02
#define LCRWord8        0x03
#define LCRStopBit1     0x00
#define LCRStopBit2     0x04  /* 1.5 not 2 for 5 bits/word */
#define LCREnableParity 0x08
#define LCROddParity    0x00
#define LCREvenParity   0x10
#define LCRMarkParity   0x20
#define LCRSpaceParity  0x30
#define LCRBreak        0x40
#define LCRSelectDivisor 0x80  /* select divisor latch */


/*
 * Clock Divisors for various Baud rates (assuming that
 * the PID is using a 1.843 MHz clock for the UART)
 */
#define Baud1200        0x0060
#define Baud2400        0x0030
#define Baud4800        0x0018
#define Baud9600        0x000c
#define Baud19200       0x0006
#define Baud38400       0x0003
#define Baud56000       0x0002  /* 2.77% error */
#define Baud57600       0x0002  /* 0% error */
#define Baud115200      0x0001

/*
 * Layout of the Parallel controller; once again, the registers
 * appear to be on word boundaries from our perspective
 */
typedef struct ST16C552PP
{
#ifdef BIG_ENDIAN
    volatile unsigned char _BEpad[3];
#endif
    volatile unsigned char port;
    volatile unsigned char _pad0[3];
    volatile unsigned char selstat;
    volatile unsigned char _pad1[3];
    volatile unsigned char ctrlcmd;
} ST16C552PP;

#define ppselect          selstat
#define ppstatus          selstat
#define ppcontrol         ctrlcmd
#define ppcommand         ctrlcmd

#define SRInterrupt     0x04
#define SRError         0x08
#define SRSelect        0x10
#define SRPaperError    0x20
#define SRAck           0x40
#define SRBusy          0x80

#define COMStrobe       0x01
#define COMAutoFDxt     0x02
#define COMInit         0x04
#define COMSelectIn     0x08
#define COMInterrupt    0x10

#define CONStrobe       0x01
#define CONAutoFDxt     0x02
#define CONInit         0x04
#define CONSelectIn     0x08
#define CONInterrupt    0x10

#define CONDirInput     0x20
#define CONDirOutput    0x00

#define IOSELinput      0xaa
#define IOSELoutput     0x55

/*
 * see devdriv.h for description of Interrupt handler functions
 */
extern void angel_ST16C552IntHandler(unsigned int ident,
                                     unsigned int data,
                                     unsigned int empty_stack);

void logserial_Reset(int port, int baudvalue);

#endif /* ndef angel_st16c552_h */

/* EOF st16c552.h */
