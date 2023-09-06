
/******************************************************************************
**                                                                           **
**  Copyright (c) 1996 Advanced Risc Machines Limited                        **
**  All rights reserved                                                      **
**                                                                           **
**      Filename  :  leds.c                                                  **
**      Author    :  Dave Brooke                                             **
**      Revision  :  1.0                                                     **
**                                                                           **
**                                                                           **
**                                                                           **
*******************************************************************************/

/*******************************************************************************/
/* Include Standard c Libraries to allow stand alone compiling and operation   */
/*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************/
/* Include the specific files for the pid7t system                             */
/*******************************************************************************/

#include "pid.h"
#include "devraw.h"
#include "devconf.h"

/*******************************************************************************/
/* Local routine defines for the leds.c module                                 */
/*******************************************************************************/

#define INFINITE_LOOP         1
#define INITIAL_DELAY_VAL     1 
#define LED_SHIFT_DELAY    5000
#define LED_SHIFT_VAL         1
#define LED_START_OFFSET      4
#define MAX_LED_VAL           8
#define MIN_LED_VAL           1
#define SWITCH_NIBBLE_MASK 0x0f 
#define WRITE_SETUP_DELAY    10

#define PCNT_IO_Select      0x20   /* b5 - Sets the I/O Direction       - Input when set   */

/******************************************************************************
* Parallel_Demonstration                                                      *
*                                                                             *
* This routine shows a very simple operation of the Parallel Interface.       *
* Four Switches are hard linked into the bottom nibble of the Parallel        *
* Register and 4 LEDs are hard linked into the top nibble.                    *
*                                                                             *
*  This exercise sets up the parallel port and then , in an infinite loop. The*
* switches are read and the value is used as a delay element - to change the  *
* rate rate of writing to the LEDS.                                           *
*                                                                             *
* The LEDs are lit by writing to the corresponding bit on the parallel port.  *
* The bits are shifted automatically each time round the loop from LED1 to    *
* LED4 and then reset to LED1.                                                *
*                                                                             *
* The loop can only be halted by ^C from the controlling system               *
*                                                                             *
*******************************************************************************/

int Parallel_Demonstration(void) 
{
 
   /***********************************************************************/
   /* Define local Variables and initialise to zero                       */
   /***********************************************************************/


    char Command_Value = 0, /* Printer Command Register storage   */
         Led_Value     = 1, /* LED Shift value                    */
         Switch_Value  = 0; /* Switch Value Read in from port     */
    int  Delay_Count   = 0; /* Loop delay count for timing delays */

    /****************************************************************************/   
    /* Initialise the Parallel Data Register to all zero's                      */
    /****************************************************************************/

    *ParPR = 0;                             /* Initialise Data Port Register to zero */

    
    /****************************************************************************/   
    /* Read and write to the port in an infinite loop                           */
    /****************************************************************************/

    while (INFINITE_LOOP) 
        {

    /****************************************************************************/   
    /* Set the port into input mode to read the  switch value set               */
    /****************************************************************************/

        Command_Value = *ParCom;                   /* Read Command Register          */

        *ParCon = Command_Value | PCNT_IO_Select ; /* set b5 - set input mode        */

        for ( Delay_Count = 0; Delay_Count <= WRITE_SETUP_DELAY ; Delay_Count++)
          { /* Delay loop to ensure that the change from input to output is stable   */
          }    
            /*read switch value - lower nibble of data port                          */
        Switch_Value = *ParPR & SWITCH_NIBBLE_MASK; 

    /****************************************************************************/   
    /* Set the port into write mode and write to the LEDs to switch the next one*/
    /****************************************************************************/

        Command_Value = *ParCom;                      /* Read Command Register                       */

        *ParCon = Command_Value & ~(PCNT_IO_Select);  /* clear b5 - set output mode                  */

        *ParPR = Led_Value << LED_START_OFFSET ; /* Write the stored LED Value to the upper nibble of*/ 
                                                 /* the Parallel Port Register                       */
    /****************************************************************************/   
    /* Shift the LED bit left for the next write - shift from LED4 back to LED1 */
    /****************************************************************************/

        if (Led_Value == MAX_LED_VAL)                /* Ensure that we don't shift beyond the 4th LED */
            Led_Value = MIN_LED_VAL;                 /* By starting back at the first                 */
        else
            Led_Value = Led_Value << LED_SHIFT_VAL ; /* otherwise shift the lit LED one to the right  */

    /****************************************************************************/   
    /* set a delay value dependant upon the switch setting for the LED Switching*/
    /****************************************************************************/

        for ( Delay_Count = 0; Delay_Count <= (INITIAL_DELAY_VAL + (Switch_Value * Switch_Value)) * LED_SHIFT_DELAY; Delay_Count++) 
          {   /* Set up a simple delay loop dependant upon the switch value  */
          }
        }

    return 0;
} /* Parallel_Demonstration */


/* by default use the PID A serial port */
#define SERIAL_DEVICE   DI_ST16C552_A 

static void print(char *str)
{
    DevError      err;
    char strb[ 128 ], *p, *q;
    int  len;

    for (q = strb, p = str; *p;)
    {
        if (*p == '\n')
                *q++ = '\r';
        *q++ = *p++;
    }
    len = (q - strb) + 1;

    err = Angel_RawDeviceWrite( SERIAL_DEVICE, (byte*)strb, len );
    
    return;
}

int angel_main(void)
{
    print("Hello from Minimal Angel\n");
    Parallel_Demonstration();
    return 0;
}
