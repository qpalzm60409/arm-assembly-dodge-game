1.1.2.4

init
----

This is an example application initialisation file.  This is based on the 
material in the "Writing Code for ROM" chapter of the SDT 2.50 User Guide.

Use the supplied init.apj project file to build the example with APM.

Alternatively, build the image using a batch-file or make-file:

a) For an ARM ROM-at-zero build use:

armcc -c ex.c
armasm -PD "ROM_AT_ADDRESS_ZERO SETL {TRUE}" init.s
armlink -o rom0.axf -RO 0x0 -RW 0x10000000 -First init.o(Init) -Map -Info Sizes init.o ex.o
fromelf -nozeropad rom0.axf -bin rom0.bin

b) For an ARM RAM-at-zero build use:

armcc -c ex.c
armasm init.s
armlink -o ram0.axf -RO 0xf0000000 -RW 0x10000000 -First init.o(Init) -Map -Info Sizes init.o ex.o
fromelf -nozeropad ram0.axf -bin ram0.bin

c) For a Thumb ROM-at-zero build use:

tcc -c ex.c
armasm -PD "THUMB SETL {TRUE}" -PD "ROM_AT_ADDRESS_ZERO SETL {TRUE}" init.s
armlink -o trom0.axf -RO 0x0 -RW 0x10000000 -First init.o(Init) -Map -Info Sizes init.o ex.o
fromelf -nozeropad trom0.axf -bin trom0.bin

d) For a Thumb RAM-at-zero build use:

tcc -c ex.c
armasm -PD "THUMB SETL {TRUE}" init.s
armlink -o tram0.axf -RO 0xf0000000 -RW 0x10000000 -First init.o(Init) -Map -Info Sizes init.o ex.o
fromelf -nozeropad tram0.axf -bin tram0.bin

