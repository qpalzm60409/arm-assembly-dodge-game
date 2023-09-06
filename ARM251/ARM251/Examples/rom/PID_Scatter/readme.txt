1.1.2.2

PID_Scatter
-----------

This is a simple interrupt-driven LED flasher, which runs on an ARM PID7T board.
It is derived from the LED example given in the PID7T Example Code Suite, but 
modified to use scatterloading.
It reads the switches S3 to determine the LED flash speed and pattern.
It requires the links LK8 on the PID board to be closed.

It features:
- a main application program, written in C
- an interrupt handler, written in C
- ROM boot code and exception vectors, written in ARM assembler
- ROM-RAM remapping on reset, as featured on the ARM PID7T board
- linked with the Embedded C library

Use the supplied scatter.apj to build the example with APM.
This creates an ELF image (scatter.axf) for loading into a debugger (ADW or armsd).
It also creates a binary ROM image (scatter.bin) suitable for downloading into the 
flash memory of a PID board.

Alternatively, build the (debug) images using a batch-file or make-file containing:
armasm -g boot.s -list
armasm -g regioninit.s -list
armasm -g vectors.s -list
armcc  -g -c C_main.c
armcc  -g -c C_int_handler.c
armlink boot.o regioninit.o vectors.o C_main.o C_int_handler.o -info total 
              -info sizes -scatter scat.txt -list out.txt -map -symb -xref 
              c:\arm250\lib\armlib_cn.32l -o scatter.axf
fromelf -nozeropad scatter.axf -bin scatter.bin


On initialization, the boot code relocates the exception vectors, interrupt 
handler and RW data from ROM to RAM, then zero-initializes the ZI data areas.

To execute/debug the image with ADW and EmbeddedICE, perform the following steps:
1) Ensure 'REMAP' link LK18 is OUT to flash-download
2) Switch on the power to the PID board and launch ADW.
3) In ADW, Options->Configure debugger, select remote_a
4) Select File->Flash download, then enter the name of the ROM image (scatter.bin).
You should see in the Command Window:
 ARM Flash Programming Utility
 AT29C040A recognised
 Input File Is : - (your ROM filename)
 Please enter the number of the first sector to write
 Default is to start at sector 0
 Start at sector 0x0
5) Press 'Enter' to start the flash programming.
You should see in the Command Window:
 Now programming flash from sector 0x0 - please wait
 16 32
 Flash written and verified successfully
6) Quit ADW, switch off the power to the PID board
7) Put 'REMAP' link LK18 IN to execute from Flash
8) Switch on the power to the PID board and launch ADW.
9) In ADW, select File->Load, then enter the name of the debug image (scatter.axf)
10) Select View->Debugger Internals, make vector_catch=0, to free-up a watchpoint unit
11) You should then be able to debug your ROM code (set breakpoints, single-step, 
view backtrace), etc.
12) To break on each interrupt, put a breakpoint on the 'if (IntCT1)' in C_main.c (line 112).



