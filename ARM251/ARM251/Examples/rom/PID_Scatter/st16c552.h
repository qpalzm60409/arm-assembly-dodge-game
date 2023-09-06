/******************************************************************************
**                                                                           **
**  Copyright (c) 1996 Advanced Risc Machines Limited                        **
**  All rights reserved                                                      **
**                                                                           **
**      Filename  :  st16c552.h                                              **
**      Author    :  Dave Brooke                                             **
**      Revision  :  1.0                                                     **
**                                                                           **
**                                                                           **
**                                                                           **
*******************************************************************************/

/****************************************************************************************************
 *                                                                                                  *
 * The following are for the st16c552 UART/PPP system. They are the bits to enable the functions in *
 * the Command Registers and the individual masks for the returned Status registers                 *
 *                                                                                                  *
 * Register Bits - these require to be set to enable the defined functions                          *
 *                                                                                                  *
 ****************************************************************************************************/
#ifndef __ST16CDEF
#define __ST16CDEF

/*******************************************************************************/
/* Interrupt Enable Register - IER                                             */
/*******************************************************************************/
#define IER_Rx_Holding_Reg  0x01   /* b0 - Recieve Holding Register Interrupt  - Enabled When Set   */
#define IER_Tx_Holding_Reg  0x02   /* b1 - Transmit Holding Register Interrupt - Enabled When Set   */
#define IER_Rx_Line_Stat    0x04   /* b2 - Receiver Line Status Interrupt      - Enabled When Set   */
#define IER_Modem_Status    0x08   /* b3 - Modem Status Register Interrupt     - Enabled When Set   */
                                   /* b7 - b4 are not used and are set to zero                      */

/*******************************************************************************/
/* FIFO Control Register - FCR                                                 */
/*******************************************************************************/
#define FCR_Fifo_Enable     0x01   /* b0 - Tx and Rx FIFO Enable               - Enabled When Set   */
#define FCR_Rx_Fifo_Reset   0x02   /* b1 - Clear Rx FIFO and reset its counter - Clears When Set    */
#define FCR_Tx_Fifo_Reset   0x04   /* b2 - Clear Tx FIFO and reset its counter - Clears When Set    */
#define FCR_DMA_Mode_Select 0x08   /* b3 - Change DMA Mode State from m0 to m1 - Mode 1 When Set    */

/* FCR b7 - b6 FIFO Trigger Level  */
 
#define FCR_Rx_Trig_Lvl_01  0x00   /* 0 0 - FIFO Rx Trigger Level 01 */              
#define FCR_Rx_Trig_Lvl_04  0x40   /* 0 1 - FIFO Rx Trigger Level 04 */          
#define FCR_Rx_Trig_Lvl_08  0x80   /* 1 0 - FIFO Rx Trigger Level 08 */
#define FCR_Rx_Trig_Lvl_16  0xc0   /* 1 1 - FIFO Rx Trigger Level 16 */

/*******************************************************************************/
/*  Latch Control Register - LCR                                               */
/*******************************************************************************/

/* LCR b2 defines the stop bits setup b1 - b0 define the Tx - Rx Word Length                        */
/* The following defines cover all of the available options                                         */

#define LCR_5_Bit_Word_1    0x00   /* 0 0 0  - 5 Bit Word - 1 Stop Bit   */
#define LCR_6_Bit_Word_1    0x01   /* 0 0 1  - 6 Bit Word - 1 Stop Bit   */ 
#define LCR_7_Bit_Word_1    0x02   /* 0 1 0  - 7 Bit Word - 1 Stop Bit   */
#define LCR_8_Bit_Word_1    0x03   /* 0 1 1  - 8 Bit Word - 1 Stop Bit   */
#define LCR_5_Bit_Word_1p5  0x04   /* 1 0 0  - 5 Bit Word - 1.5 Stop Bit */
#define LCR_6_Bit_Word_2    0x05   /* 1 0 1  - 6 Bit Word - 2 Stop Bit   */
#define LCR_7_Bit_Word_2    0x06   /* 1 1 0  - 6 Bit Word - 1 Stop Bit   */
#define LCR_8_Bit_Word_2    0x07   /* 1 1 1  - 6 Bit Word - 1 Stop Bit   */

#define LCR_Parity_Enable   0x08   /* b3 - Enable Parity Bit Generation and Check - Enabled When Set */
#define LCR_Parity_Even     0x10   /* b4 - Odd/Even Parity Generation and Check   - Even When Set    */
#define LCR_Parity_Set      0x20   /* b5 - Toggle Generated Parity Bit 0/1        - 0 When Set       */
#define LCR_Break_Set       0x40   /* b6 - Force Break Control ( Tx o/p low)      - Forced When Set  */
#define LCR_Divisor_Latch   0x80   /* b7 - Enable Internal Baud Rate Latch        - Enabled When Set */

/*******************************************************************************/
/* Modem Control Register - MCR                                                */
/*******************************************************************************/

#define MCR_DTR_Low         0x01   /* b0 - Set DTR Signal Low/High - DTR Low when Set */
#define MCR_RTS_Low         0x02   /* b1 - Set RTS Signal Low/High - RTS Low when Set */
                                   /* MCR b2 is not used                              */
#define MCR_Interrupt_En    0x08   /* b3 - Interrupt output pin Operate/3-State  - Operate when Set */
#define MCR_Loopback_Mode   0x10   /* b4 - Loopback(Test) Mode Enable            - Enabled When Set */

/* The Following Registers are Status Registers which Report conditions within the UART/PPP during  *
 * operation. The defined values are masks to ensure that the register flags are correctly accessed */

/*******************************************************************************/
/* Interrupt Status Register - ISR                                             */
/*******************************************************************************/

/* ISR b0 indicates that an interrupt is pending when clear. b3 - b1 signal which interrupt as per:- */

#define ISR_LSR_Source      0x06   /* 0 1 1 - Receiver Line Status Register Priority 1 */
#define ISR_Rx_Rdy_Source   0x04   /* 0 1 0 - Received Data Ready           Priority 2 */
#define ISR_Rx_Rdy_TO_Src   0x0c   /* 1 1 0 - Received Data Ready Time Out  Priority 2 */
#define ISR_Tx_Rdy_Source   0x02   /* 0 0 1 - Transmitter Holding Reg Empty Priority 3 */
#define ISR_MODEM_Source    0x00   /* 0 0 0 - Modem Status Register         Priority 4 */

/* ISR b7 - b4 are not used - in st16c552 b7 - b6 are Set b5 - b4 are Clear   */

/*******************************************************************************/
/* Line Status Register - LSR                                                                              */
/*******************************************************************************/

#define LSR_Rx_Data_Ready   0x01   /* b0 - Data Received and Saved in Holding Reg - Set when Valid */
#define LSR_Overrun_Error   0x02   /* b1 - Overrun Error Occured                  - Set When Valid */
#define LSR_Parity_Error    0x04   /* b2 - Received Data has Incorrect Parity     - Set When Valid */
#define LSR_Framing_Error   0x08   /* b3 - Framing Error (No Stop Bit)            - Set When Valid */
#define LSR_Break_Interrupt 0x10   /* b4 - Break Signal Received                  - Set When Valid */
#define LSR_Tx_Hold_Empty   0x20   /* b5 - Tx Holding Register is empty and ready - Set When Valid */
#define LSR_Tx_Fifo_Empty   0x40   /* b6 - Tx Shift Registers and FIFO are Empty  - Set When Valid */
#define LSR_Fifo_Error      0x80   /* b7 - At Least one of b4 - b2 has occurred   - Set When Valid */

/*******************************************************************************/
/* Modem Status Register - MSR                                                 */
/*******************************************************************************/
/* */
/* ---------------------       */

#define MSR_CTS_Change      0x01   /* b0 - Set When CTS Input has Changed State */
#define MSR_DSR_Change      0x02   /* b1 - Set When DSR Input has Changed State */
#define MSR_RI_Change       0x04   /* b2 - Set When RI  Input has Changed State */
#define MSR_CD_Change       0x08   /* b3 - Set When CD  Input has Changed State */
#define MSR_CTS_Lp_State    0x10   /* b4 - RTS Equivalent during loopback - inverse of CTS */
#define MSR_DSR_Lp_State    0x20   /* b5 - DTR Equivalent during loopback - inverse of DSR */
#define MSR_RI_Lp_State     0x40   /* b6 - MCR b2 Equivalent during loopback - inverse of RI */
#define MSR_CD_Lp_State     0x88   /* b7 - INT EN Equivalent during loopback - inverse of CD */

/*******************************************************************************/
/* ScratchPad Register - SPR                                                   */
/*******************************************************************************/

/* This is a user register for any required bit storage required */

#define SPR_User_0          0x01
#define SPR_User_1          0x02
#define SPR_User_2          0x04
#define SPR_User_3          0x08
#define SPR_User_4          0x10
#define SPR_User_5          0x20
#define SPR_User_6          0x40
#define SPR_User_7          0x80

/*******************************************************************************/
/*Divisor Latch Lower and Upper Byte Values - DLL DLM                          */
/*******************************************************************************/

/* These are the required 16 bit divisor values for the internal baud rate based on a 1.8MHz Clock */
#define DLM_50_Baud         0x09
#define DLL_50_Baud         0x00

#define DLM_110_Baud        0x04
#define DLL_110_Baud        0x17

#define DLM_150_Baud        0x03
#define DLL_150_Baud        0x00

#define DLM_300_Baud        0x01
#define DLL_300_Baud        0x5c

#define DLM_600_Baud        0x00
#define DLL_600_Baud        0xc0

#define DLM_1200_Baud       0x00
#define DLL_1200_Baud       0x60

#define DLM_2400_Baud       0x00
#define DLL_2400_Baud       0x30

#define DLM_4800_Baud       0x00
#define DLL_4800_Baud       0x18

#define DLM_7200_Baud       0x00
#define DLL_7200_Baud       0x10

#define DLM_9600_Baud       0x00
#define DLL_9600_Baud       0x0c

#define DLM_19200_Baud      0x00
#define DLL_19200_Baud      0x06

#define DLM_38400_Baud      0x00
#define DLL_38400_Baud      0x03

#define DLM_56000_Baud      0x00
#define DLL_56000_Baud      0x02

#define DLM_115200_Baud     0x00
#define DLL_115200_Baud     0x01


/*******************************************************************************/
/* Parallel Printer Port Definitions For The st16c552 Interface                */
/*******************************************************************************/

/* Parallel Control Register Sets the defined output pins */
/* This is a WRITE ONLY Register                          */
 
#define PCNT_STROBE         0x01   /* b0 - Sets the Parallel strobe pin - Low when set     */
#define PCNT_Auto_FDXT      0x02   /* b1 - Sets the Auto FDXT pin       - Low when set     */
#define PCNT_INIT           0x04   /* b2 - Sets the INIT pin            - High when set    */
#define PCNT_SLCTIN         0x08   /* b3 - Sets the SLCTIN Pin          - Low when set     */
#define PCNT_IRQ_Mask       0x10   /* b4 - Enables the PAR Interrupt    - Enabled when set */
#define PCNT_IO_Select      0x20   /* b5 - Sets the I/O Direction       - Input when set   */
                                   /* b7 - b6  Not Used                                    */
/* Parallel Command Register echoes the states of the Parallel control pins (above) */
/* This is a READ ONLY Register                                                     */

#define PCMD_STROBE         0x01   /* b0 - Parallel strobe pin is Low when set  */
#define PCMD_Auto_FDXT      0x02   /* b1 - Auto FDXT pin is Low when set        */
#define PCMD_INIT           0x04   /* b2 - INIT pin is High when set            */
#define PCMD_SLCTIN         0x08   /* b3 - SLCTIN Pin is Low when set           */
#define PCMD_IRQ_Enable     0x10   /* b4 - PAR Interrupt is Enabled when set    */
                                   /* b7 - b5  Not Used set to 1                */

/* Parallel Status Register shows the states of the printer outputs and the interrupt */
/* This is a READ ONLY Register                                                       */

                                   /* b1 - b0  Not Used set to 1           */
#define PSR_IRQ_State       0x04   /* b2 - Interrupt is pending when clear */
#define PSR_ERROR           0x08   /* b3 - ERROR Input is high when set    */
#define PSR_SLCT            0x10   /* b4 - SLCT Input is high when set     */
#define PSR_PE              0x20   /* b5 - PE Input is high when set       */
#define PSR_ACK             0x40   /* b6 - ACK Input is high when set      */
#define PSR_BUSY            0x80   /* b7 - BUSY Input is high when set     */

#endif





