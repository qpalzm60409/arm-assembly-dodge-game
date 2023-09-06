/**************************************************************************
 *
 * ARM - Strategic Support Group
 *
 ***************************************************************************/

/***************************************************************************
 *
 * Title        : Traffic
 * Module       : led.h
 * Description  :
 *
 *              Set of Macro's to control the LED's on a AEB-1 board.   
 *
 * Platform     : AEB-1
 * Status       : DEVELOPING
 * History      :
 *
 *      980329 ASloss
 *      
 *              Added header
 *
 * Copyright (C) 1998 ARM Ltd. All rights reserved.
 *
 *
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1998/08/06 18:49:58 $
 * Revising $Author: swoodall $
 *
 **************************************************************************/

/**************************************************************************
 * IMPORTS
 **************************************************************************/

/* none... */

/**************************************************************************
 * MACROS
 **************************************************************************/

/* -- Programmable Peripheral Interface Base Address  */
                
#define PPI_BASE        (0xFFFF1C00) 

/* -- PPI Port C */

#define PPI_PC          ((volatile char *) (PPI_BASE + 0x08)) 

/* -- PPI_CTLR is used to both control over all IO modes and set or reset
   -- individual Port C IO lines */

#define PPI_CTLR_ADDR   ( (volatile char *) (PPI_BASE + 0x0C) )

/* -- PPI_INIT macro sets the modes for the IO ports TBD: What is the best 
   -- state for unassigned ports? */

#define PPI_INIT        ( *PPI_CTLR_ADDR = 0x80 ); \
                        ( *PPI_PC = 0x0 )

/* -- On LAB, the LEDs are connected through PPI Port C
   LED1 is Green, LED2 is Red, LED3 is Yellow, LED4 is Green
 */

#define PPI_PC_SEL_0    (0)
#define PPI_PC_SEL_1    (2)
#define PPI_PC_SEL_2    (4)
#define PPI_PC_SEL_3    (6)
#define PPI_PC_SEL_4    (8)
#define PPI_PC_SEL_5    (10)
#define PPI_PC_SEL_6    (12)
#define PPI_PC_SEL_7    (14)

#define PPI_PC_RESET    (0)
#define PPI_PC_SET      (1)

#define LED_1_ON        (*PPI_CTLR_ADDR = PPI_PC_SEL_7 + PPI_PC_SET )
#define LED_2_ON        (*PPI_CTLR_ADDR = PPI_PC_SEL_6 + PPI_PC_SET )
#define LED_3_ON        (*PPI_CTLR_ADDR = PPI_PC_SEL_5 + PPI_PC_SET )
#define LED_4_ON        (*PPI_CTLR_ADDR = PPI_PC_SEL_4 + PPI_PC_SET )

#define LED_1_OFF       (*PPI_CTLR_ADDR = PPI_PC_SEL_7 + PPI_PC_RESET )
#define LED_2_OFF       (*PPI_CTLR_ADDR = PPI_PC_SEL_6 + PPI_PC_RESET )
#define LED_3_OFF       (*PPI_CTLR_ADDR = PPI_PC_SEL_5 + PPI_PC_RESET )
#define LED_4_OFF       (*PPI_CTLR_ADDR = PPI_PC_SEL_4 + PPI_PC_RESET )


/**************************************************************************
 * DATATYPES
 **************************************************************************/

/* none... */

/**************************************************************************
 * STATICS
 **************************************************************************/

/* none... */

/**************************************************************************
 * PROTOTYPES
 **************************************************************************/

/* none... */

/**************************************************************************
 * End of led.h
 **************************************************************************/
