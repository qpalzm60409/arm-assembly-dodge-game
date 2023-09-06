/******************************************************************************
**                                                                           **
**  Copyright (c) 1996 Advanced Risc Machines Limited                        **
**  All rights reserved                                                      **
**                                                                           **
**      Filename  :  nisa.h                                                  **
**      Author    :  Dave Brooke                                             **
**      Revision  :  1.0                                                     **
**                                                                           **
**                                                                           **
**                                                                           **
*******************************************************************************/
#ifndef __NISAHEADER
#define __NISAHEADER

/*******************************************************************************/
/* Define the base address of the NISA Peripherals                             */
/*******************************************************************************/
#define NisaBase        0x0C000000 /* NISA Base address */

/*******************************************************************************/
/* Define the Base addresses of the separate peripherals                       */
/*******************************************************************************/
#define NisaSerA        0x0D800000 /* Serial port A */
#define NisaSerB        0x0D800020 /* Serial port B */
#define NisaPar         0x0D800040 /* Parallel port */

/*******************************************************************************/
/* Serial Link A Register Addresses                                            */
/*******************************************************************************/
#define SerA_RHR        ((volatile unsigned  *) (NisaSerA + 0x0))  /* Port A Rx holding data */
#define SerA_THR        ((volatile unsigned  *) (NisaSerA + 0x0))  /* Port A Tx holding data */
#define SerA_IER        ((volatile unsigned  *) (NisaSerA + 0x4))  /* Port A Interrupt enable */
#define SerA_FCR        ((volatile unsigned  *) (NisaSerA + 0x8))  /* Port A Fifo control */
#define SerA_ISR        ((volatile unsigned  *) (NisaSerA + 0x8))  /* Port A Interrupt status */
#define SerA_LCR        ((volatile unsigned  *) (NisaSerA + 0xC))  /* Port A Line control */
#define SerA_MCR        ((volatile unsigned  *) (NisaSerA + 0x10)) /* Port A Modem control */
#define SerA_LSR        ((volatile unsigned  *) (NisaSerA + 0x14)) /* Port A Line status */
#define SerA_MSR        ((volatile unsigned  *) (NisaSerA + 0x18)) /* Port A Modem status */
#define SerA_SPR        ((volatile unsigned  *) (NisaSerA + 0x1C)) /* Port A Scratchpad */
#define SerA_DLL        ((volatile unsigned  *) (NisaSerA + 0x0))  /* Port A LSB divisor latch */
#define SerA_DLM        ((volatile unsigned  *) (NisaSerA + 0x4))  /* Port A MSB divisor latch */

/*******************************************************************************/
/* Serial Link B Register Addresses                                            */
/*******************************************************************************/
#define SerB_RHR        ((volatile unsigned  *) (NisaSerB + 0x0))  /* Port B Rx holding data */
#define SerB_THR        ((volatile unsigned  *) (NisaSerB + 0x0))  /* Port B Tx holding data */
#define SerB_IER        ((volatile unsigned  *) (NisaSerB + 0x4))  /* Port B Interrupt enable */
#define SerB_FCR        ((volatile unsigned  *) (NisaSerB + 0x8))  /* Port B Fifo control */
#define SerB_ISR        ((volatile unsigned  *) (NisaSerB + 0x8))  /* Port B Interrupt status */
#define SerB_LCR        ((volatile unsigned  *) (NisaSerB + 0xC))  /* Port B Line control */
#define SerB_MCR        ((volatile unsigned  *) (NisaSerB + 0x10)) /* Port B Modem control */
#define SerB_LSR        ((volatile unsigned  *) (NisaSerB + 0x14)) /* Port B Line status */
#define SerB_MSR        ((volatile unsigned  *) (NisaSerB + 0x18)) /* Port B Modem status */
#define SerB_SPR        ((volatile unsigned  *) (NisaSerB + 0x1C)) /* Port B Scratchpad */
#define SerB_DLL        ((volatile unsigned  *) (NisaSerB + 0x0))  /* Port B LSB divisor latch */
#define SerB_DLM        ((volatile unsigned  *) (NisaSerB + 0x4))  /* Port B MSB divisor latch */

/*******************************************************************************/
/* Parallel/Printer Register Addresses                                         */
/*******************************************************************************/
#define ParPR           ((volatile unsigned  *)(NisaPar + 0x0)) /* Parallel port data register */
#define ParIO           ((volatile unsigned  *)(NisaPar + 0x4)) /* Parallel port I/O select register */
#define ParStat         ((volatile unsigned  *)(NisaPar + 0x4)) /* Parallel port status register */
#define ParCon          ((volatile unsigned  *)(NisaPar + 0x8)) /* Parallel port control register */
#define ParCom          ((volatile unsigned  *)(NisaPar + 0x8)) /* Parallel port command register */


#endif
