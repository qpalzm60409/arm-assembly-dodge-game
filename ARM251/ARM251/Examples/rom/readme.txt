Writing Code for ROM Examples
=============================

This directory contains four examples, based on the material in the "Writing
Code for ROM" chapter of the SDT 2.50 User Guide.

1) The "init" directory contains an example application initialisation file.

2) The "embed_lib" directory contains an example application which makes use 
   of sprintf() from the Embedded C library.

3) The "ARMul_Scatter" directory contains a simple scatterloading application
   which runs under the ARMulator. This also uses sprintf() from the Embedded 
   C library.

4) The "PID_Scatter" directory contains a more complex scatterloading
   application which flashes the LEDs on an ARM Development Board (PID7T).


Scatter-Loading Examples
------------------------

The examples 3 & 4 both illustrate the use of boot code (boot.s - which is an
extended version of the init.s code used in the "init" example) and region
initialization code (regioninit.s) to perform all the initialization required
before branching to the main C application code.  The boot code defines the 
ENTRY point and initializes the stack pointers for each mode.  The region 
initialization code copies RO code and RW data from ROM to RAM, and 
zero-initializes the ZI data areas used by the C code.

The function InitRegions in regioninit.s uses a macro (called RegionInit) to 
initialize the specified execution regions.  These execution region names 
match those given in the scatter description file scat.txt.


Notes for Users of EmbeddedICE 2.04 or earlier
----------------------------------------------

SWI SYS_Write0 (used in these examples to print to the debugger's console) was
unfortunately broken for EmbeddedICE 2.04 and earlier.
You should upgrade to the latest ICEagent (currently 2.07).

Alternatively it is possible to workaround this problem by using the following
code to replace the Write0() SWI call, though this is not recommended.


/* Write a character */ 
__swi(SemiSWI) void _WriteC(unsigned op, char *c);
#define WriteC(c) _WriteC (0x3,c)

void Write0 (char *string)
{ int pos = 0;
  while (string[pos] != 0)
    WriteC(&string[pos++]);
}


