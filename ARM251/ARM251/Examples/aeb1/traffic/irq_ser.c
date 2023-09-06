/*******************************************************************************
 * 
 * ARM Strategic Support Group
 *
 *******************************************************************************/

/*******************************************************************************
 *
 * Module       : irq_ser.c
 * Description  : Setups 77790 IRQ registers and serives the IRQ's (timer and 
 *                button interrupt requests)
 * Status       : complete
 * Platform     : AEB-1
 * History      : 980416 ASloss
 *
 *                - added button interrupt service
 *                - added header information
 *
 * Notes        : Refer to the Sharp 77790 documentation 
 *
 * Copyright (C) 1998 ARM Ltd. All rights reserved.
 *
 *
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1998/08/06 18:49:57 $
 * Revising $Author: swoodall $
 *
 *******************************************************************************/

/*******************************************************************************
 * IMPORT
 *******************************************************************************/

#include <stdio.h>
#include "traffic.h"

/*******************************************************************************
 * MACRO'S
 *******************************************************************************/

/* -- Interrupt controller ------------------------------------------ */

#define INT_CTLR_BASE   (0xFFFFA800)
#define ICR0            ((volatile int *)( INT_CTLR_BASE ))
#define ICLR            ((volatile int *)( INT_CTLR_BASE + 0x08))
#define IRQER           ((volatile int *)( INT_CTLR_BASE + 0x0c))
#define IRQSR           ((volatile int *)( INT_CTLR_BASE + 0x14))
#define IRQ_TIMER1      (0x0080)
#define IRQ_BUTTON      (0x0001)

/* -- Cache -------------------------------------------------------- */

#define CACHE_ON        ((volatile int *) (0xffffa400))

/* -- Counter timer ------------------------------------------------ */

#define CT_BASE         (0xFFFF1800)
#define CT_CNTR1        ((volatile unsigned char *)( CT_BASE + 0x04))
#define CT_CWR          ((volatile unsigned char *)( CT_BASE + 0x0C))
#define CT1_LATCH       (0x40)
#define CT1_MODE2       (0x74)

/* -- Load in the maximum count ------------------------------------ */

#define CT1_RELOAD      *CT_CNTR1 = 0; *CT_CNTR1 = 0

/* -- I/O Configuration -------------------------------------------- */

#define IOCR            ((volatile int *) 0xFFFFA410)
#define CT1_GATE        (0x1800) /* force gate active */

/* -- General ------------------------------------------------------ */

#define IRQVector (unsigned *) 0x18
#define PIPE_OFFSET            0x08
#define WORD_OFFSET            0x02
#define CHECK_24_BIT    0xff000000
#define BRANCH_OP_CODE  0xea000000
#define MAX_CT_LOAD     0x0000ffff
#define LOWER_16_MASK   0x0000ffff

/*****************************************************************************
 * EXTERN's
 *****************************************************************************/

extern void handler_irq(void);
extern void SetupSVC(void);
extern int  Angel_IRQ_Address;

/*****************************************************************************
 * GLOBALS
 *****************************************************************************/


volatile unsigned int AuxCT_repetitions; /* Number of times the CT has
                                            completed its count */

/*****************************************************************************
 * ROUTINES
 *****************************************************************************/

/* -- irq_installhandler ------------------------------------------------------
 *                                                            
 * Description  : Places a branch instruction for the routine into the defined 
 *                'vector' location - this function returns the original 
 *                contents of the 'vector      
 *
 * Parameters   : unsigned routine - IRQ handler
 *              : unsigned *vector - IRQ vector   
 * Return       : 
 * Notes        : 'routine' MUST BE WITHIN A  RANGE OF 32Mbytes FROM 'vector'
 *                                                                             
 */

static void irq_installhandler (unsigned routine, unsigned *vector) 
{
unsigned old_vector_value = 0;
unsigned *absolute;   
  
   old_vector_value = 0;

        /*******************************************************************************        
         * Now the inverse must be done on the original vector to retrieve the Angel   *
         * IRQ Handler routine .                                                       *
         *******************************************************************************/
   
   old_vector_value  = *vector;          // Get old vector contents ....................
   old_vector_value ^= 0xe59ff000;       // Mask off the instruction to get the offset
 
        // ** calculate absolute address

   absolute             = (unsigned *) (0x18 + old_vector_value+0x8); 
                                        // IRQ Address
   Angel_IRQ_Address    = *absolute;    // chain Angel Interrupt Handler          
   *absolute            = routine;      // IRQ handler

}

/* -- irq_inittimer1 ----------------------------------------------------------
 *                                                            
 * Description  : Initialises the counter timer (most work done by 
 *                irq_auxctstart)      
 *
 * Parameters   : none...   
 * Return       : none...
 * Notes        : none...
 *                                                                             
 */

static void irq_inittimer1 (void)
{
  *IOCR         = 0;            // Set counter gate to external .........
  *ICLR         |= IRQ_TIMER1;  // Clear pending interrupts .............
}

/* -- irq_initbutton ---------------------------------------------------------
 *                                                            
 * Description  :  Initialises the button interrupt    
 *
 * Parameters   : none...   
 * Return       : none...
 * Notes        : none...
 *                                                                             
 */

static void irq_initbutton (void)
{
  *ICLR         |= IRQ_BUTTON;  // Clear pending interrupts ..............
  *ICR0         |= 0x0003;      // Enable low-high edge edge detection ... 
  *IRQER        |= 0x0001;      // Enable Button interrupt ............... 
}

/******************************************************************************
* IRQHandler                                                                  *
*                                                                             *
* This IRQ handler only services two interrupt source - the Counter Timer 1   *
* source and the button interrupt. When this is sensed the handler resets     *
* the counter interrupt - and increments the AuxCT_repetitions global         *
* variable...                                                                 *
*                                                                             *
* THIS PORTION OF CODE MUST BE COMPILED USING THE ARM STATE COMPILER ARMCC    *
* EVEN WHEN USED IN A THUMB STATE ENVIRONMENT. FAILURE TO DO THIS WOULD RESULT*
* IN UNPREDICTABLE RETURNS FROM THE IRQ HANDLER.                              *
*                                                                             *
*******************************************************************************/

/* -- irq_auxtimer ------------------------------------------------------------
 *
 * Description  : handles the aux timer interrupt.
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : Other interrupt sources are handled by angel. See 
 *                ang_irq_ven.s
 *
 *
 */ 

void  irq_auxtimer (void) 
{
unsigned status; 

   /* Read in the Interrupt source ......................................... */

   status = *IRQSR;             // Get current status ...................... 

   /* Deal only with the Timer 1 Interrupt in this handler ................. */

   if (status & IRQ_TIMER1 )
   {
     AuxCT_repetitions++;               // Another cycle completed .........
     *ICLR  = IRQ_TIMER1;               // Clear Timer interrupt ...........
   }    

   *ICLR        = IRQ_BUTTON;           // Clear Button interrupt ..........
} 

/* -- irq_buttonpress -----------------------------------------------------------
 *
 * Description  : handles the button press interrupt...
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : none...
 *
 */ 

void  irq_buttonpress (void) 
{
unsigned status; 

   /* Read in the Interrupt source .................................. */

        status  = *IRQSR;
        *ICLR   = IRQ_BUTTON;           // Clear interrupt ..............

   if (status & IRQ_BUTTON ) {
        traffic_interrupt ();           // Call traffic sequencer .......
   }    
}

/* -- irq_init ----------------------------------------------------------------
 * 
 * Description  : This module is where all system setup would be placed - in 
 *                this case it is only Counter/Timer that we are initialising 
 *                so we only have a routine to initialise this. Also called 
 *                from here is the Interrupt Handler install routine and 
 *                initialize button interrupt
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : none...
 *
 */

void irq_init (void)
{
        SetupSVC();             /* This is necessary if Angel is not running
                                 *  on the target (as when running semi-hosted
                                 *  from E-Ice) otherwise, let Angel handle it */

        irq_inittimer1 ();
        irq_initbutton ();

        irq_installhandler ((unsigned)handler_irq, IRQVector);
}

/* -- irq_auxctstart -----------------------------------------------------------
 *
 * Description  : switches the timer on
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : none...
 *
 */  

void irq_auxctstart (void)
{
  AuxCT_repetitions = 0;
  *CT_CWR = CT1_MODE2;
  CT1_RELOAD;                   // reload CT ..............................
  *IRQER |= IRQ_TIMER1;         // enable interrupt .......................
  
  /* Set counter gate to true ............................................. */
  *IOCR = CT1_GATE;
}

/* -- irq_auxctstop -----------------------------------------------------------
 *
 * Description  : switches the timer off
 *
 * Parameters   : none...
 * Return       : unsigned int - return the duration...
 * Notes        : none...
 *
 */  

unsigned int irq_auxctstop( void )
{
unsigned int ticks;

  /* Set counter gate to external .......................................... */
  *IOCR = 0;

  *CT_CWR = CT1_LATCH;
  *IRQER &= ~IRQ_TIMER1;                        // disable CT1 interrupts ..........
  *CT_CWR = CT1_MODE2;
  ticks = *CT_CNTR1;                            // lsb ............................. 
  ticks |= (*CT_CNTR1 << 8);                    // msb .............................
  ticks = (0x10000 - ticks) & 0x0FFFF;          /* counters count down, FFFF means one
                                                   tick has ocuured ................*/
  ticks = ( AuxCT_repetitions << 16) + ticks;
  return( ticks );
}

/*******************************************************************************
 * END OF irq.c
 *******************************************************************************/
