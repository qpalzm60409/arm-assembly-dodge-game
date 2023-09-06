1.1.2.2

ARMul_Scatter
-------------

This is a simple scatter-loaded application which runs under the ARMulator.
It simply displays the linker-generated scatter symbols on the screen.
It is not normally necessary to access these linker symbols in application 
code (only really needed in init code) - it is done here just for illustration.

Use the supplied scatter.apj to build the example with APM.
This creates an ELF image (image.axf) for loading into a debugger (ADW or armsd).
It also creates a binary ROM image (image.bin) suitable for downloading into the 
flash memory of a PID board.

Alternatively, build the (debug) images using a batch-file or make-file containing:
armasm -g boot.s -list
armasm -g regioninit.s -list
armasm -g vectors.s -list
armcc  -g -c C_main.c
armcc  -g -c C_func.c
armlink boot.o regioninit.o vectors.o C_main.o C_func.o -info total 
       -info sizes -scatter scat.txt -list out.txt -map -symb -xref 
       c:\arm250\lib\embedded\armlib_cn.32l -o scatter.axf
fromelf -nozeropad scatter.axf -bin scatter.bin

