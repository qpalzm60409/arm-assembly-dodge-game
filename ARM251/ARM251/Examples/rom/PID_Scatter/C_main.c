/**************************************************************/
/*     File: C_main.c                                         */
/*  Purpose: Main Application Code                            */
/**************************************************************/

/*
 * Copyright (C) ARM Limited, 1999. All rights reserved.
 */

/*
This application flashes the LEDs on the PID board.
It reads the switches S3 connected to the Parallel Port to determine
the flash speed and pattern.  It requires the links LK8 to be closed.

The C entry point is call 'C_Entry()' not 'main()' to prevent the ANSI C 
libraries being pulled in during the link step, because the Embedded C 
libraries are being used here instead.

The Embedded C libraries do not contain printf(), so here Semihosting SWIs 
together with sprintf() are used to display text onto the console.  
This mechanism is portable across ARMulator, Angel, EmbeddedICE and Multi-ICE.
*/


#include <stdio.h>
#include "stand.h"


#ifdef __thumb
/* Thumb Semihosting SWI */
#define SemiSWI 0xAB
#else
/* ARM Semihosting SWI */
#define SemiSWI 0x123456
#endif


/* Write a string */
__swi(SemiSWI) void _Write0(unsigned op, char *string);
#define Write0(string) _Write0 (0x4,string)

char buf[40];   /* will consume some ZI data */


int IRQ_TIMER1;
int IntCT1;
int CT1Value;
int CT1Val;
int CT1Clear;
int IRQ_TIMER2;
int IntCT2;
int CT2Value;
int CT2Val;
int CT2Clear;
int IntPAR;

char switch_val = 0, 
     sequence = 0, 
     seq_init = 0,
     count = 0,
     flash_speed = 0, 
     toggle = 0,
     init = INIT_VALUE;


void Set_Timers (char time);
void Led_Write (char val);
void Initialise(void);
int Read_Switch(void);
void Init_Timers(void);


void C_Entry(void)
{
/*  sprintf(buf, "Initializing...\n"); */
/*  Write0(buf); */

  Initialise();

/*  sprintf(buf, "Running...\n"); */
/*  Write0(buf); */

while (1)
  {
    /****************************************************************************/
    /*    Only read the switch if a parallel interrupt has been forced or the   */
    /*    loop is on its first cycle - where the switches need initialization   */
    /****************************************************************************/

    if (IntPAR || init)
      {
        switch_val = Read_Switch(); 

        IntPAR = 0;  /* Reset the Parallel Interrupt Flag                         */

        flash_speed = switch_val & SPEED_MASK;/*Speed controlled by lower 2 bits */
        sequence = ((switch_val & SEQUENCE_MASK) >> SEQUENCE_SHIFT); 
                                               /* sequence - by the upper 2 bits */

        seq_init = INIT_VALUE; /* Initialise new LED sequence                    */

        Set_Timers(flash_speed); /* Set the sequence speed dependant upon switch */

        init = 0; /* Ensure that the initialisation is not repeated              */
      }

    /****************************************************************************/
    /*  Strobe the LEDs when the timer 1 interrupt occurs                       */
    /****************************************************************************/

    if (IntCT1)
      {
        IntCT1 = 0;     /* Reset the Timer 1 Interrupt Flag */
        Led_Write(count);/* Write the current LED State      */ 

        switch (sequence)
          {


    /****************************************************************************/
    /* Sequence - Binary Counter                                                */
    /****************************************************************************/
          case BIN_COUNT:
            if (seq_init)
              {
              count = 0;
              seq_init = 0;
              }

            if (count > MAX_BIN_COUNT)
              count = 0;
            else
              count++;
           
            break;

    /****************************************************************************/
    /* Sequence - Single Bit Left Shift                                         */
    /****************************************************************************/

          case LEFT_SHIFT:
            if (seq_init)
              {
                count =  MIN_LED_VAL;
                seq_init =0;
              }

            if (count == MAX_LED_VAL)
              count = MIN_LED_VAL;
            else
              count <<= SHIFT_VALUE;
            break;

    /****************************************************************************/
    /* Sequence - Two Bit Alternate Pattern                                     */
    /****************************************************************************/

          case ALTERNATE:
            if (!toggle)
              {
              count = ALT_RIGHT;
              toggle++;
              }
            else
              {
              count = ALT_LEFT;
              toggle = 0;
              }
            break;

    /****************************************************************************/
    /* Sequence - Single Bit Right Shift                                        */
    /****************************************************************************/

          case RIGHT_SHIFT:
            if (seq_init)
              {
                count = MAX_LED_VAL ;
                seq_init =0;
              }
            if (count == MIN_LED_VAL)
              count = MAX_LED_VAL;
            else
              count >>= SHIFT_VALUE;
            break;
          }
      }
  }
}


/******************************************************************************
* Read_Switch                                                                 *
* This function reads the lower nibble of the Parallel port to get the current*
* switch setting. The 2 least significant bits control the sequence speed, the*
* upper 2 bits control the sequence type. This module returns the full switch *
* value.                                                                      *
*******************************************************************************/

int Read_Switch(void)
{
char Command_Value;
int Switch_Value;
int Delay_Count;

  Command_Value = *ParCom;                   /* Read Command Register         */

  *ParCon = Command_Value | PCNT_IO_Select ; /* set b5 - set input mode       */

  for ( Delay_Count = 0; Delay_Count <= WRITE_SETUP_DELAY ; Delay_Count++)
    { /* Delay loop to ensure that the change from input to output is stable  */
    }    
 
  /*read switch value - lower nibble of data port                             */
  Switch_Value = *ParPR & SWITCH_NIBBLE_MASK; 

return (Switch_Value);
}


/******************************************************************************
* Led_Write                                                                   *
* This function writes the current sequence to the upper nibble of the        *
* Parallel Port.                                                              *
*******************************************************************************/
void Led_Write(char val)
{
char Command_Value;

  Command_Value = *ParCom;                      
  /* Read Command Register                       */

  *ParCon = Command_Value & ~(PCNT_IO_Select);  
  /* clear b5 - set output mode                  */

  *ParPR = val << LED_START_OFFSET ; 
  /* Write the stored LED Value to the upper nibble of*/ 
  /* the Parallel Port Register                       */
}


/******************************************************************************
* Init_Timers                                                                 *
* Initialises the Interrupt and sets the counter timers to zero, also ensures *
* that the Interrupts from the timers are fully reset.                        *
*******************************************************************************/
void Init_Timers(void)
{
    /****************************************************************************/
    /* disable all interrupts                                                   */
    /****************************************************************************/
 
  *IRQEnableClear = ~0;


    /****************************************************************************/
    /* disable counters by clearing the control bytes                           */
    /****************************************************************************/

  *Timer1Control = 0;
  *Timer2Control = 0;

    /****************************************************************************/
    /* clear counter/timer interrupts by writing to the clear register          */
    /****************************************************************************/

  *Timer1Clear = 0 ; /* any data will work */
  *Timer2Clear = 0 ; /* any data will work */
}


/******************************************************************************
* Initialise                                                                  *
* This module is where all system setup would be placed - in this case it is  *
* only Counter/Timer 1 that we are initialising so we only have a routine to  *
* initialise this. Also called from here is the Interrupt Handler install     *
* routine                                                                     *
*******************************************************************************/

void Initialise(void)
{
    /****************************************************************************/
    /* Initialise Counter Timers                                                */
    /****************************************************************************/
Init_Timers();

    /****************************************************************************/
    /*  Initialise Interrupt Variables                                          */
    /****************************************************************************/

IntPAR = 0;
IntCT1 = 0;
IntCT2 = 0;

}

/******************************************************************************
* Set_Timers                                                                  *
* This module controls the setting up and loading of the counter timers with  *
* the required time to set the sequence speed - a value of 0 - 3 is passed    *
* into the routine which decides the load value of the two timers.            *
* The timer controls and interrupts are set on exit.                          *
*******************************************************************************/

void Set_Timers(char time)
{
char i;

    /****************************************************************************/
    /* Select load time from input value                                        */
    /****************************************************************************/

switch (time)
  {
  case SLOW:
    *Timer1Load = SLOW_LOAD;
    *Timer2Load = SLOW_LOAD;
    break;

  case SLOW_MED:
    *Timer1Load = SLOW_MED_LOAD;
    *Timer2Load = SLOW_MED_LOAD;
    break;

  case MED_FAST:
    *Timer1Load = MED_FAST_LOAD;
    *Timer2Load = MED_FAST_LOAD;
    break;

  case FAST:
    *Timer1Load = FAST_LOAD;
    *Timer2Load = FAST_LOAD;
    break;
  }

    /****************************************************************************/
    /* Set Up the Counter Timers Control Registers                              */
    /****************************************************************************/

  *Timer1Control = (TimerEnable   |   /* Enable the Timer                       */
                    TimerPeriodic |   /* Periodic Timer producing interrupt     */
                    TimerPrescale8 ); /* Set Maximum Prescale - 8  bits         */

  *Timer2Control = (TimerEnable   |   /* Enable the Timer                       */
                    TimerPeriodic |   /* Periodic Timer producing interrupt     */
                    TimerPrescale8 ); /* Set Maximum Prescale - 8  bits         */

    /****************************************************************************/
    /* now initialise the System Interrupts and Enable the Timer 1 Interrupt    */
    /****************************************************************************/

   *IRQEnableClear = ~0;       /* Clear all interrupts                          */
   IntCT1 = 0;                 /* Clear CT 1 Flag                               */
   IntCT2 = 0;                 /* Clear CT 2 Flag                               */
   i = *ParCom;                /* Read the Parallel Command Register            */
   *ParCon = i | PCNT_IRQ_Mask;/* Set the interrupt Enable Mask                 */
   IntPAR = 0;                 /* Clear the Parallel Interrupt Flag             */
   

   *IRQEnableSet = IRQTimer1 | IRQTimer2 | IRQSourcePAR ; 
   /* Enable the counter timer interrupts and the Parallel Interrupt            */
}

