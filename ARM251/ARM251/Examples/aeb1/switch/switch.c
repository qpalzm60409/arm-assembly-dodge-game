/**************************************************************************
 *
 * ARM - Strategic Support Group
 *
 ***************************************************************************/

/***************************************************************************
 *
 * Title        : Traffic
 * Module       : traffic.c
 * Description  :
 *
 *    Traffic light program is designed to simulate the US and European
 *    Traffic Light System. It is designed to work with the ARM Evaluation
 *    Board (KPI-00041a). 
 *
 * Platform     : AEB-1 / KPI-00041a
 * Status       : DEVELOPING
 * History      :
 *
 *      980301 ASloss
 *      
 *              started working on the design and structure.
 *
 *      980408 ASloss
 *
 *              added in-line assembler example.
 *
 *      980428 ASloss
 *
 *              added interrupt handler
 *
 *      980625 ASloss
 *
 *              added Angel SWI
 *
 * Copyright (C) 1998 ARM Ltd. All rights reserved.
 *
 *
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1998/08/06 18:49:56 $
 * Revising $Author: swoodall $
 *
 **************************************************************************/

/**************************************************************************
 * IMPORTS
 **************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "led.h"
#include "irq_ser.h"

/**************************************************************************
 * MACROS
 **************************************************************************/

/* -- uncomment for EUROPEAN style traffic lights ..... */

#define         EUROPEAN                1 

/* -- Non embedded library ............................ */

// #define      FULL_LIB        1       

#define         DEBUG           1
#define         HARDWARE        1

#define         LIGHT           int
#define         RED             0
#define         AMBER           1
#define         GREEN           2
#define         COMPLETE        3

#define         STATE           int
#define         ON              1
#define         OFF             0

#define         pRED            active.light_red
#define         pAMBER          active.light_amber
#define         pGREEN          active.light_green

#define         angel_SWI       0x123456

/**************************************************************************
 * MISC
 **************************************************************************/

__swi (angel_SWI) void _Exit(unsigned op, unsigned except);
#define Exit() _Exit(0x18,0x20026) 

/**************************************************************************
 * DATATYPES
 **************************************************************************/

// none...

/**************************************************************************
 * STATICS
 **************************************************************************/

/*
extern char                     Image$$RW$$Limit[];

char *                          __heap_base = Image$$RW$$Limit;
int                             __heap_limit = 0x20000;
int                             __stack_base = 0x20000;
*/
static int                      counter = 0;
        
/**************************************************************************
 * ROUTINUES
 **************************************************************************/

/* -- switch_init ---------------------------------------------------------
 *
 * Description  :
 * 
 * Parameters   :
 *
 */

void switch_init (void)
{ 
        LED_1_ON;
        LED_2_ON;
        LED_3_ON;
        LED_4_ON;

        irq_init (); 
}

/* -- switch_interrupt ----------------------------------------------------
 *
 * Description  : The interrupt button has been pressed. Toggles D1 LED...
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : none...
 *
 */

void switch_interrupt (void)
{
        if ( counter & 0x08 ) LED_1_ON; else LED_1_OFF;
        if ( counter & 0x04 ) LED_2_ON; else LED_2_OFF;
        if ( counter & 0x02 ) LED_3_ON; else LED_3_OFF;
        if ( counter & 0x01 ) LED_4_ON; else LED_4_OFF;

        counter ++;
}

/* -- C_Entry --------------------------------------------------------------
 *
 * Description  : entry point for switch
 * 
 * Parameters   : none...
 * Return       : return 1 if successful
 * Notes        : none...
 *
 */

int C_Entry ( void )
{

        // -- initialize all internal structures .......................

        switch_init ();

        // -- process ..................................................


        // while (counter!=1000) { switch_interrupt (); } 


        Exit(); // Exiting from the Embedded System

        return 0;
}

/**************************************************************************
 * End of traffic.c
 **************************************************************************/
