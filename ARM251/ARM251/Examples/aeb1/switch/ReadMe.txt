***************************************************
*
* ARM Strategic Support Group
*
***************************************************
Title   : Switch ReadMe

This directory contains the sources to build the 
Interrupt Switch example.

The Interrupt Switch example is a simple interrupt 
driven code for the AEB-1 board. Everytime the 
interrupt button on the board is pressed an internal 
counter is incremented and the counter binary 
pattern is displayed via the 4 LED's on the AEB-1 
board.

Sources      Type    Description
===================================================
switch       .c      contain the program entry 
                     point.
switch       .h      contain the export functions
irq_ven      .s      interrupt handler
irq_ser      .c      interrupt service (installer
                     and handler)
irq_ser      .h      export interrupt service 
led          .h      MACRO's to switch LED's on and
                     off
startup      .s      start up code for the Embedded
                     Libraries.
switch       .apj    Switch Project File used by
                     APM
***************************************************
* End of Switch ReadMe
***************************************************
   
