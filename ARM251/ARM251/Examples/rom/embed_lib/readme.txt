1.1.2.2

Embed_lib
---------

This is an example application which makes use of sprintf() from the Embedded C 
library.  This is based on the material in the "Writing Code for ROM" chapter 
of the SDT 2.50 User Guide.

Use the supplied embed.apj project file to build the example with APM.

Alternatively, build the image using a batch-file or make-file:

a) For an ARM build use:

armcc -c print.c
armasm startup.s
armlink -o print.axf -Info Totals startup.o print.o c:\arm250\lib\embedded\armlib_cn.32l

b) For a Thumb build use:

tcc -c print.c
armasm -16 startup.s
armlink -o print.axf -Info Totals startup.o print.o c:\arm250\lib\embedded\armlib_i.16l

