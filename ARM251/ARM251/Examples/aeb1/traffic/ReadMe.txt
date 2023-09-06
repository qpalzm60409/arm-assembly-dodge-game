***************************************************
*
* ARM Strategic Support Group
*
***************************************************
Title   : Traffic ReadMe

This directory contains the sources to build the 
Traffic Light example.

The Traffic Light example is a simple interrupt driven 
code for the AEB-1 board. There are two interrupt. One
syconous interrupt coming from the internal counter 
timer on the Microprocessor and the other interrupt
asyconous interrupt coming from a button that the user
can press. On the AEB-1 board 3 LED's are used to 
represent traffic light and the 4th LED is toggled when
the user press the button interrupt.

Sources      Type    Description
===================================================
traffic      .c      contain the program entry 
                     point.
traffic      .h      contain the export functions
irq_ven      .s      interrupt handler
irq_ser      .c      interrupt service (installer
                     and handler)
irq_ser      .h      export interrupt service 
led          .h      MACRO's to switch LED's on and
                     off
startup      .s      Start up code for the Embedded
                     Libraries
traffic      .apj    Traffic Project File used by
                     APM
***************************************************
* End of Traffic ReadMe
***************************************************
   
