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

#define         pRED             active.light_red
#define         pAMBER           active.light_amber
#define         pGREEN           active.light_green

#define         angel_SWI       0x123456

/**************************************************************************
 * MISC
 **************************************************************************/

__swi (angel_SWI) void _Exit(unsigned op, unsigned except);
#define Exit() _Exit(0x18,0x20026) 


/**************************************************************************
 * DATATYPES
 **************************************************************************/

typedef struct {

        LIGHT                   light_red;
        LIGHT                   light_amber;
        LIGHT                   light_green;

} traffic_lightstr;

/**************************************************************************
 * STATICS
 **************************************************************************/

static traffic_lightstr         active;

extern char                     Image$$RW$$Limit[];

char *                          __heap_base = Image$$RW$$Limit;
int                             __heap_limit = 0x20000;
int                             __stack_base = 0x20000;

static int                      toggle = 0;
        
/**************************************************************************
 * ROUTINUES
 **************************************************************************/

void traffic_printf ( char *s)
{ 
#ifdef FULL_LIB
        printf ( s );
#endif
}

void traffic_printf_int (char *s, int x)
{
#ifdef FULL_LIB
        printf ( s, x);
#endif
}


/* traffic_debugprobe ----------------------------------------------------
 *
 * Description  : Outputs the information about the change in the Traffic 
 *                Light Model.
 *
 * Parameters   : LIGHT w - which light is being changed
 *              : STATE s - the new state of the light.
 * Return       : none...
 * Notes        : none...
 *
 */

void traffic_debugprobe (LIGHT w, STATE s) 
{
#ifdef FULL_LIB
                switch (w) {

                case RED        : 

                        printf ("TL: changing RED (%d) -> ",pRED);
                        printf ("(%d)\n",s); 
                        break;

                case AMBER      : 

                        printf ("TL: changing AMBER (%d) -> ",pAMBER);
                        printf ("(%d)\n",s); 
                        break;

                case GREEN      : 

                        printf ("TL: changing GREEN (%d) -> ",pGREEN);
                        printf ("(%d)\n",s); 
                        break;
                }

#endif
}

/* -- traffic_change -----------------------------------------------------
 *
 * Description  : This routine changes the state of a particular LED.
 *
 * Parameters   : LIGHT w -  RED | GREEN | AMBER
 *              : STATE s - ON | OFF
 * Return       : none...
 * Notes        : Updates 'active' an internal global variable.
 *
 */

void traffic_change (LIGHT w, STATE s)
{ 

        if(DEBUG) traffic_printf ("TL: entering traffic_change \n");

#ifdef FULL_LIB
        assert ((w==RED||w==AMBER||w==GREEN)&&(s==ON||s==OFF));
#endif          

        if(DEBUG) traffic_debugprobe (w,s);

        // -- process ................................................

                switch (w) {
                case RED        : 

                        pRED = s; 

                        if(HARDWARE) {
                        
                                if(s) LED_2_ON; else LED_2_OFF;

                        }

                        break;
                case AMBER      : 
                
                        pAMBER = s; 
                        
                        if(HARDWARE) {

                                if(s) LED_3_ON; else LED_3_OFF;

                        }
 
                        break;
                case GREEN      : 
                
                        pGREEN = s;

                        if(HARDWARE) {
                        
                                if(s) LED_4_ON; else LED_4_OFF;

                        }

                        break;

                case COMPLETE   :

                        if(HARDWARE) {
                        
                                if(s) LED_1_ON; else LED_1_OFF;

                        }       

                }

        if(DEBUG) traffic_printf ("TL: leaving traffic_change \n");
}

/* -- traffic_init --------------------------------------------------------
 *
 * Description  : Initializes the LED's on the ARM Evaluation board to be 
 *                OFF. Setup the internal GLOBALS to indicate that the 
 *                LED's are OFF.
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : none...
 *                
 */

void traffic_init (void)
{

        if(DEBUG) traffic_printf ("TL: entering traffic_init \n");

        // -- init internal ..........................................

        pRED            = 0;
        pAMBER          = 0;
        pGREEN          = 0;

        // -- init hardware ..........................................
        
        if(HARDWARE) { PPI_INIT; }

                
        traffic_change (RED, OFF);
        traffic_change (AMBER, OFF);
        traffic_change (GREEN, OFF);
        traffic_change (COMPLETE, OFF);

        if(HARDWARE) {
        int             i;      
        
        irq_init ();

        // -- semihosting time functions when no hardware available ...
        
        irq_auxctstart ();

                i = AuxCT_repetitions + 500;
                while ( i > AuxCT_repetitions )
                {
                ;
                }
        

        }


        traffic_change  (GREEN,ON);

        if(DEBUG) traffic_printf ("TL: leaving traffic_init \n");
}

/* -- traffic_addone -----------------------------------------------------
 *
 * Description  : return an number incremented by 1
 *
 * Parameters   : int num - number passed into the function.
 * Return       : int
 * Notes        : an example of using in-line assembler.
 *
 */

int traffic_addone ( int num )
{
        __asm 
        {
        mov             r0, num         // r0 = num
        add             r0, r0, #1      // r0 = r0 + 1
        mov             num, r0         // num = r0
        }
        
        return num;
}

/* -- traffic_delay ------------------------------------------------------
 *
 * Description  : This provides the delay required for the traffic lights
 *
 * Parameters   : int delay - the delay is in seconds
 * Retrun       : none...
 * Notes        :
 *
 *              Uses the internal clock on the ARM Evaluation Board.
 *
 */

void traffic_delay (int delay)
{       
        if(DEBUG) traffic_printf ("TL: entering traffic_delay \n");

#ifdef  FULL_LIB
        assert (delay>0);
#endif  

        if(HARDWARE) {
        int             i;

                i = AuxCT_repetitions + 500;
                while ( i > AuxCT_repetitions )
                {
                ;
                }
        
        } else {
#ifdef FULL_LIB
        time_t          old,new;        
        
        // -- semihosting time functions when no hardware available ...

        new = old = time(&old);

                do {
                        // -- spin 
                } while ( (old+delay) > time(&new));
#endif
        }

        if(DEBUG) traffic_printf ("TL: leaving traffic_delay \n");
}
        
/* -- traffic_sequence ------------------------------------------------
 *
 * Description  : Runs the traffic light sequence...
 *
 * Parameters   : int cycles - number of cycles 
 * Return       : none...
 * Notes        : none...
 *
 */


/****************************************************
 * USA Traffic Lights
 ****************************************************/

#ifndef EUROPEAN

void traffic_sequence ( int cycles )
{
int     xcount;
int     phase;

        // -- init ...............................................................

        if(DEBUG) traffic_printf ("TL: entering traffic_sequence \n");

#ifdef FULL_LIB
        assert  (cycles>0);
#endif

        xcount  = 1;
        phase   = 0;

        if(DEBUG) traffic_printf_int ("TL: ** Traffic Sequence with %d cycles \n",cycles);
        
        // -- process ............................................................

        while (xcount <= cycles) {
                
                switch (phase) {

                case 0          : // -- switch the GREEN(off) then AMBA(on) ...

                        if(DEBUG) traffic_printf_int ("\n\nTL: *** New Cycle [%d] *** \n",xcount);
                        traffic_change  (GREEN,OFF);
                        traffic_change  (AMBER,ON);
                        phase           = 1;
                        traffic_delay   (10);
                        
                        break;  

                case 1          : // -- switch the AMBA(off) then RED(on) ....
                
                        traffic_change  (AMBER, OFF);
                        traffic_change  (RED,   ON);
                        phase           = 2;
                        traffic_delay   (10);
                        
                        break;
                

                case 2          : // -- switch the RED(off) then GREEN(on) ...

                        traffic_change  (RED,OFF);
                        traffic_change  (AMBER,OFF);
                        traffic_change  (GREEN,ON);
                        phase           = 0;
                        traffic_delay   (10);
                        xcount = traffic_addone (xcount);
                        break;
                
                }
        }

        if(DEBUG) traffic_printf ( "TL: leaving traffic_sequence \n");
}

#else

/****************************************************
 * EUROPEAN Traffic Lights
 ****************************************************/


void traffic_sequence ( int cycles )
{
int     xcount;
int     phase;

        // -- init ...............................................................

        if(DEBUG) traffic_printf ("TL: entering traffic_sequence \n");

#ifdef FULL_LIB
        assert  (cycles>0);
#endif

        xcount  = 1;
        phase   = 0;

        if(DEBUG) traffic_printf_int ("TL: ** Traffic Sequence with %d cycles \n",cycles);
        
        // -- process ............................................................

        while (xcount <= cycles) {
                
                switch (phase) {

                case 0          : // -- switch the GREEN(off) then AMBA(on) ...

                        if(DEBUG) traffic_printf_int ("\n\nTL: *** New Cycle [%d] *** \n",xcount);
                        traffic_change  (GREEN,OFF);
                        traffic_change  (AMBER,ON);
                        phase           = 1;
                        traffic_delay   (10);
                        
                        break;  

                case 1          : // -- switch the AMBA(off) then RED(on) ....
                
                        traffic_change  (AMBER,OFF);
                        traffic_change  (RED,ON);
                        phase           = 2;
                        traffic_delay   (10);
                        
                        break;


                case 2          : // -- switch the RED(on) then AMBER(on) ...

                        traffic_change  (AMBER,ON);
                        phase           = 3;
                        traffic_delay   (10);

                        break;

                case 3          : // -- switch the RED(off) then GREEN(on) ...

                        traffic_change  (RED,OFF);
                        traffic_change  (AMBER,OFF);
                        traffic_change  (GREEN,ON);
                        phase           = 0;
                        traffic_delay   (10);
                        xcount = traffic_addone (xcount);
                        break;
                
                }
        }

        if(DEBUG) traffic_printf ( "TL: leaving traffic_sequence \n");
}

#endif

/* -- traffic_interrupt ----------------------------------------------------
 *
 * Description  : The interrupt button has been pressed. Toggles D1 LED...
 *
 * Parameters   : none...
 * Return       : none...
 * Notes        : none...
 *
 */

void traffic_interrupt (void)
{
        if( toggle == 0 ) {
        traffic_change  (COMPLETE, ON);
        toggle = 1;
        } else {
        toggle = 0;
        traffic_change  (COMPLETE, OFF);
        }
}

/* -- C_Entry --------------------------------------------------------------
 *
 * Description  : entry point for traffic lights
 * 
 * Parameters   : none...
 * Return       : return 1 if successful
 * Notes        : none...
 *
 */

int C_Entry ( void )
{
        if(DEBUG) traffic_printf ("TL: entering main \n");
        
        // -- initialize all internal structures .......................

                traffic_init ();

        // -- process ..................................................

                traffic_sequence (50);

        if(DEBUG) traffic_printf ("TL: leaving main \n");

        traffic_change (COMPLETE, ON);

        Exit();

        return 0;
}

/**************************************************************************
 * End of traffic.c
 **************************************************************************/
