/*-*-C-*-
 *
 * $Revision: 1.5 $
 *   $Author: rivimey $
 *     $Date: 1998/08/06 16:49:18 $
 *
 * Copyright Advanced RISC Machines Limited, 1995.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Character based packet transmission engine
 */

#include <stdarg.h>             /* ANSI varargs support */
#include "angel.h"              /* Angel system definitions */
#include "endian.h"             /* Endian independant memory access macros */
#include "crc.h"                /* crc generation definitions and headers */
#include "rxtx.h"
#include "channels.h"
#include "buffers.h"
#include "logging.h"

/* definitions to describe the engines state */
#define N_STX           0x0     /* first 2 bits for N_ */
#define N_BODY          0x1
#define N_ETX           0x2
#define N_IDLE          0x3
#define N_MASK          0x3     /* mask for the Encapsulator state */

#define E_PLAIN         (0x0 << 2)  /* 3rd bit for E_ */
#define E_ESC           (0x1 << 2)  /* 3rd bit for E_ */
#define E_MASK          (0x1 << 2)  /* mask for the Escaper state */

#define F_HEAD          (0x0 << 3)  /* 4th and 5th bits for F_ */
#define F_DATA          (0x1 << 3)
#define F_CRC           (0x1 << 4)
#define F_MASK          (0x3 << 3)  /* mask for the Escaper state */

static unsigned char escape(unsigned char ch_in, struct te_state *txstate);

void 
Angel_TxEngineInit(const struct re_config *txconfig,
                   const struct data_packet *packet,
                   struct te_state *txstate)
{
    /* Note: SerialOpen passes NULL as packet here! */
    IGNORE(packet);
    
    txstate->tx_state = N_STX | E_PLAIN | F_HEAD;
    txstate->field_c = 0;
    txstate->encoded = 0;
    txstate->config = txconfig;
    txstate->crc = 0;
}

te_status 
Angel_TxEngine(const struct data_packet *packet,
               struct te_state *txstate,
               unsigned char *tx_ch)
{
    /* TODO: gaurd on long/bad packets */
    /*
     * encapsulate the packet, framing has been moved from a seperate
     * function into the encapsulation routine as it needed too much
     * inherited state for it to be sensibly located elsewhere
     */
    switch ((txstate->tx_state) & N_MASK)
    {
        case N_STX:
            LogTrace("txe-stx ");
            txstate->tx_state = (txstate->tx_state & ~N_MASK) | N_BODY;
            *tx_ch = txstate->config->stx;
            txstate->field_c = 3;  /* set up for the header */
            txstate->crc = startCRC32;  /* set up basic crc */
            return TS_IN_PKT;
            
        case N_BODY:
            {
                switch (txstate->tx_state & F_MASK)
                {
                    case F_HEAD:
                        LogTrace("txe-head ");
                        if (txstate->field_c == 3)
                        {
                            /* send type */
                            *tx_ch = escape(packet->type, txstate);
                            return TS_IN_PKT;
                        }
                        else
                        {
                            *tx_ch = escape((packet->len >> (txstate->field_c - 1) * 8) & 0xff,
                                            txstate);
                            if (txstate->field_c == 0)
                            {
                                /* move on to the next state */
                                txstate->tx_state = (txstate->tx_state & ~F_MASK) | F_DATA;
                                txstate->field_c = packet->len;
                            }
                            return TS_IN_PKT;
                        }
                    case F_DATA:
                        LogTrace("txe-data ");
                        *tx_ch = escape(packet->data[packet->len - txstate->field_c], txstate);
                        if (txstate->field_c == 0)
                        {
                            /* move on to the next state */
                            txstate->tx_state = (txstate->tx_state & ~F_MASK) | F_CRC;
                            txstate->field_c = 4;
                        }
                        return TS_IN_PKT;
                    case F_CRC:
                        LogTrace("txe-crc ");
                        *tx_ch = escape((txstate->crc >> ((txstate->field_c - 1) * 8)) & 0xff,
                                        txstate);

                        if (txstate->field_c == 0)
                        {
                            LogTrace1("txe crc = 0x%x\n", txstate->crc);
                            /* move on to the next state */
                            txstate->tx_state = (txstate->tx_state & ~N_MASK) | N_ETX;
                        }
                        return TS_IN_PKT;
                }
            }
        case N_ETX:
            LogTrace("txe-etx\n");
            txstate->tx_state = (txstate->tx_state & ~N_MASK) | N_IDLE;
            *tx_ch = txstate->config->etx;
            return TS_DONE_PKT;
            
        case N_IDLE:
            LogTrace("txe-idle\n");
            return TS_IDLE;
            
        default:
            LogTrace("tx-def\n");
            txstate->tx_state = (txstate->tx_state & ~N_MASK) | N_IDLE;
            return TS_IDLE;
            
    }
    
    /* stop a silly -Wall warning */
    return (te_status) - 1;
}

/*
 * crc generation occurs in the escape function because it is the only
 * place where we know that we're putting a real char into the buffer
 * rather than an escaped one.
 * We must be careful here not to update the crc when we're sending it
 */
static unsigned char 
escape(unsigned char ch_in, struct te_state *txstate)
{
    if (((txstate->tx_state) & E_MASK) == E_ESC)
    {
        /* char has been escaped so send the real char */
        LogTrace("txe-echar ");
        txstate->tx_state = (txstate->tx_state & ~E_MASK) | E_PLAIN;
        txstate->field_c--;
        if ((txstate->tx_state & F_MASK) != F_CRC)
            txstate->crc = crc32(&ch_in, 1, txstate->crc);
        return ch_in | serial_ESCAPE;
    }
    if ((ch_in < 32) && ((txstate->config->esc_set & (1 << ch_in)) != 0))
    {
        /* char needs escaping */
        LogTrace("txe-esc ");
        txstate->tx_state = (txstate->tx_state & ~E_MASK) | E_ESC;
        return txstate->config->esc;
    }
    /* must be a char that can be sent plain */
    txstate->field_c--;
    if ((txstate->tx_state & F_MASK) != F_CRC)
        txstate->crc = crc32(&ch_in, 1, txstate->crc);
    return ch_in;
}

/* EOF tx.c */
