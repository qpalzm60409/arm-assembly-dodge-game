##
# GNU Make / SunOS / Solaris Makefile for Minimal Angel on a PID7T
#
# To run using Sun Make, remove 'ifeq' construction, leaving definitions
# of CC_DEBUG and ASM_DEBUG appropriate for the Angel version you require.
#
# $Revision: 1.15.2.4 $
#   $Author: rivimey $
#     $Date: 1998/10/23 15:54:12 $
#
# Copyright (c) 1996, 1997, 1998 ARM Ltd.
# All Rights Reserved
#

# The following options on the make command line are supported:
#
# ENDIAN=BIG   build for big endian system (default is little endian)
#
#

CLXINCL=	../../../clx
PORTINCL=	../../pid
CLIBINCL=	../../../500/cl
DCCINCL=	../../dcc

# LATE_STARTUP= 0   ## hardwired
THUMB_SUPPORT=  1
# MINIMAL_ANGEL=1   ## hardwired

# Set this to 1 for debugging version of Angel, 0 for Release.
DEBUG=		0

# Use this to specify where Angel's data space should be.
RWADDR=		0x02040000

# Use this ROADDR to run Angel from SRAM; this is aliased at 0x00008000
# so specifying 2048000 means that angel will also appear at 0x00048800
ROADDR=		0x02048000

# 
ANGELVSN=	0x0001

# Minimal Angel is to built without ethernet support.
# ETHERNET_SUPPORTED=0   ## hardwired

# Define this only if DEBUG_METHOD is set to logterm in devconf.h
#LOGTERM_DEBUGGING=0  ## hardwired

#
# Pick one of the following modules to link first:
#
# startrom.o - Used when ROM can be unmapped from 0
#   except.o - Used when ROM is permanently mapped at 0
#
FIRST=		'startrom.o(ROMStartup)'
#FIRST=		'except.o(__Vectors)'

APCS=		3/32bit/noswst/nofp 
#APCS=		3/32bit/swst/fp 

# The PID boards typically have an ARM7TDMI, which is
# architecture 4T
CPUTYPE=	-arch 4T

# This is the default endianess - it can be overwritten on the
# make command line by ENDIAN=BIG
ENDIAN = LITTLE

LITTLE_ENDIANFLAG= -li
LITTLE_ENDIANDEFN= -DLITTLE_ENDIAN
LITTLE_ENDIANSFX= l
LITTLE_ENDIANOBJDIR= little

BIG_ENDIANFLAG=	-bi
BIG_ENDIANDEFN=	-DBIG_ENDIAN
BIG_ENDIANSFX=	b
BIG_ENDIANOBJDIR= big

ENDIANFLAG= $($(ENDIAN)_ENDIANFLAG)
ENDIANDEFN= $($(ENDIAN)_ENDIANDEFN)
ENDIANSFX= $($(ENDIAN)_ENDIANSFX)

TYPEOBJDIR_0= _rel
TYPEOBJDIR_1= _deb

OBJDIR = $($(ENDIAN)_ENDIANOBJDIR)$(TYPEOBJDIR_$(DEBUG))

#
# now make compiler and assembler arguments
#
CCENDIAN=	$(ENDIANFLAG) $(ENDIANDEFN)
ASENDIAN=	$(ENDIANFLAG)

CDEFS=	-DTARGET -DTHUMB_SUPPORT=$(THUMB_SUPPORT) -DANGELVSN=$(ANGELVSN) \
	-DROADDR=$(ROADDR) -DRWADDR=$(RWADDR) -DLOGTERM_DEBUGGING=0 \
	-DMINIMAL_ANGEL=1 -DLATE_STARTUP=0 -DETHERNET_SUPPORTED=0 \
	-DDEBUG=$(DEBUG) -DASSERT_ENABLED=$(ASSERT_ENABLED)

# Extra debugging flags: choose from:
#   NO_RXE_DATA         -- don't print received data bytes.
#   CLEAR_BUFFERS       -- clear Angel buffers on allocation/deallocation
#   NO_LOG_INFO         -- Allow LogWarning/LogError, but kill LogInfo. Saves ~35K code
#   NO_LOG_WARNING      -- Allow LogInfo/LogError, but kill LogWarning. 
#   DEBUG_FILE_TOO      -- include file names in debug info available (define format too)
#                          (includes quite a bit of extra text)
#   ASSERT_ENABLED      -- Allow Assertions with ASSERT(cond, info) macro
#   HAVE_DISASS         -- a routine disass(...) exists which disassembles an instruction
#                          (see logterm.c/disass.h for details of call). Adds ~15Kb.
#

ifeq ($(DEBUG),1)
    # CC_DEBUG = -g
    # CC_DEBUG = -g -DCLEAR_BUFFERS
    # CC_DEBUG = -go -DHAVE_DISASS
    CC_DEBUG = -go
    ASM_DEBUG= -g
else
    CC_DEBUG= -O2
    ASM_DEBUG=
endif

ASSERT_ENABLED= 0

# Leave this one alone - comment, uncomment and edit the two macros above
# as necessary
#
ALL_DEBUG=	$(CC_DEBUG) $(FUSION_DEBUG)

CFLAGS= -fc -fah -apcs $(APCS) $(CPUTYPE) -c \
	-I../.. -I$(DCCINCL) -I$(PORTINCL) -I$(CLXINCL) \
	$(CCENDIAN) $(CFL) $(CDEFS) $(ALL_DEBUG)

AFLAGS=	-32 -apcs $(APCS) $(CPUTYPE) $(ASM_DEBUG) $(ASENDIAN) -I$(OBJDIR) \
	-I../.. -I$(PORTINCL) -I$(CLIBINCL) \
	-PD "LOGTERM_DEBUGGING SETA 0" \
	-PD "ANGELVSN SETA $(ANGELVSN)" \
	-PD "DEBUG SETA $(DEBUG)" \
	-PD "LATE_STARTUP SETA 0" \
	-PD "ROADDR SETA $(ROADDR)" \
	-PD "THUMB_SUPPORT SETA $(THUMB_SUPPORT)" \
	-PD "ASSERT_ENABLED SETA $(ASSERT_ENABLED)" \
	-PD "MINIMAL_ANGEL SETA 1" \
	-PD "ETHERNET_SUPPORTED SETA 0"

LFLAGS=		-symbols $@.sym.lst

#
# set ARMTOOLS to point to the tools you wish to
# use (otherwise the default ones will be used)
#
# Note that tasm is used since this means the makefile does not have to
# be modified between enabling and disabling Thumb support
#
ARMTOOLS=
CC=		$(ARMTOOLS)armcc
AS=		$(ARMTOOLS)tasm
LD=		$(ARMTOOLS)armlink
AR=		$(ARMTOOLS)armlib
FROMELF=	$(ARMTOOLS)fromelf
RM=		rm -f
RMDIR=		rmdir

# Port-specific Objects always included in the image
PORTOBJS=	$(OBJDIR)/devices.o

# Port-specific Objects included in the driver library
PORTDRIVERS=    $(OBJDIR)/st16c552.o

# General Angel Objects always included in the image
ANGELOBJS=	$(OBJDIR)/serlasm.o $(OBJDIR)/devraw.o $(OBJDIR)/startlib.o\
		$(OBJDIR)/devmisc.o $(OBJDIR)/buffers.o $(OBJDIR)/params.o \
		$(OBJDIR)/logging.o $(OBJDIR)/except.o $(OBJDIR)/serraw.o \
		$(OBJDIR)/stacks.o $(OBJDIR)/serlock.o $(OBJDIR)/interrupt.o \
		$(OBJDIR)/suppasm.o

# General Angel Objects always included in the image
DRIVEROBJS=     $(OBJDIR)/e5_sendb.o $(OBJDIR)/rombox.o $(OBJDIR)/panicblk.o \
		$(OBJDIR)/log552.o $(OBJDIR)/logterm.o $(OBJDIR)/delay.o \
		$(OBJDIR)/disass.o

DCCDRIVER=	$(OBJDIR)/dccdrv.o $(OBJDIR)/dccsup.o

DRIVERS=	$(ANGELDRIVERS) $(DCCDRIVER) $(PORTDRIVERS)

OBJS=		$(ANGELOBJS) $(PORTOBJS)


all:	$(OBJDIR)/angmin.lib $(OBJDIR)/minexample.rom

# Make sure the object directory exists before we try to compile things
# into it, even with multi-job gnumake runs.
$(OBJDIR)/dir_exists:
	@if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi
	touch $(OBJDIR)/dir_exists

$(OBJS) $(DRIVERS) $(OBJDIR)/minexample.o $(OBJDIR)/lolevel.s: $(OBJDIR)/dir_exists
$(OBJDIR)/suppmsg.o $(OBJDIR)/rtstand.o $(OBJDIR)/startrom.o: $(OBJDIR)/dir_exists

$(OBJDIR)/angmin.lib:	$(OBJS) $(OBJDIR)/startrom.o $(OBJDIR)/drivers.a
	$(LD) -aof -o $@ $(OBJS) $(OBJDIR)/drivers.a

$(OBJDIR)/drivers.a:	$(DRIVERS)
	$(AR) -c -o $@ $(DRIVERS)

###

clean:
	-if [ -d $(OBJDIR) ]; then \
	  $(RM) $(DRIVERS) $(OBJS) \
	  		$(OBJDIR)/makelo $(OBJDIR)/drivers.a $(OBJDIR)/lolevel.s \
	  		$(OBJDIR)/supplib.o $(OBJDIR)/startrom.o $(OBJDIR)/minexample.o \
			$(OBJDIR)/rtstand.o $(OBJDIR)/suppmsg.o $(OBJDIR)/angmin.lib \
			$(OBJDIR)/minexample.rom $(OBJDIR)/*.sym $(OBJDIR)/*.lst ; \
	  $(RMDIR) $(OBJDIR); \
	fi


###

#
# Rules for linking example applications
#


# Use this version if you need the code in rtstand.s to link with.
#
#$(OBJDIR)/minexample.rom:  $(OBJDIR)/minexample.o $(OBJDIR)/suppmsg.o $(OBJDIR)/rtstand.o $(OBJDIR)/angmin.lib
#	$(LD) -o $(OBJDIR)/minexample.elf $(OBJDIR)/minexample.o $(OBJDIR)/startrom.o \
#		$(OBJDIR)/suppmsg.o $(OBJDIR)/rtstand.o $(OBJDIR)/angmin.lib \
#		-ro-base $(ROADDR) -rw-base $(RWADDR) -first 'startrom.o(ROMStartup)' \
#		-symbols $@.sym.lst -entry $(ROADDR)
#	$(FROMELF) -nozeropad $(OBJDIR)/minexample.elf -bin $(OBJDIR)/minexample.rom
#	$(FROMELF) $(OBJDIR)/minexample.elf -aifbin $(OBJDIR)/minexample.sym
#	sort -f $@.sym.lst > $@.alpha.lst
#	sort -b +1 $@.sym.lst > $@.num.lst

# this version works for self-contained modules.

$(OBJDIR)/minexample.rom:  $(OBJDIR)/minexample.o $(OBJDIR)/suppmsg.o $(OBJDIR)/angmin.lib
	$(LD) -o $(OBJDIR)/minexample.elf $(OBJDIR)/minexample.o $(OBJDIR)/startrom.o \
		$(OBJDIR)/angmin.lib \
		-ro-base $(ROADDR) -rw-base $(RWADDR) -first 'startrom.o(ROMStartup)' \
		-symbols $@.sym.lst -entry $(ROADDR)
	$(FROMELF) -nozeropad $(OBJDIR)/minexample.elf -bin $(OBJDIR)/minexample.rom
	$(FROMELF) $(OBJDIR)/minexample.elf -aifbin $(OBJDIR)/minexample.sym
	sort -f $@.sym.lst > $@.alpha.lst
	sort -b +1 $@.sym.lst > $@.num.lst

$(OBJDIR)/minexample.o:	../../examples/minexample.c
	$(CC) $(CFLAGS) $(CFLminexample) -o $@ ../../examples/minexample.c

$(OBJDIR)/suppmsg.o:	../../msgbuild.c
	$(CC) $(CFLAGS) $(CFLsuppmsg) -DMSG_UTILS_ONLY -o $(OBJDIR)/suppmsg.o ../../msgbuild.c

$(OBJDIR)/rtstand.o:	../../examples/rtstand.s
	$(AS) ../../examples/rtstand.s $(AFLAGS) -o $@

# And here are the assmebler dependencies - probably not 100% correct

$(OBJDIR)/startrom.o:	../../startrom.s ../../listopts.s $(OBJDIR)/lolevel.s \
			../../macros.s $(PORTINCL)/target.s
	$(AS) ../../startrom.s $(AFLAGS) -o $@

$(OBJDIR)/startlib.o:	../../startlib.s ../../listopts.s $(OBJDIR)/lolevel.s \
			../../macros.s $(PORTINCL)/target.s
	$(AS) ../../startlib.s $(AFLAGS) -o $@

$(OBJDIR)/except.o:	../../except.s ../../listopts.s $(OBJDIR)/lolevel.s \
			../../taskmacs.s ../../macros.s $(PORTINCL)/target.s
	$(AS) ../../except.s $(AFLAGS) -o $@

#$(OBJDIR)/stackrom.o:	../../stackchk.s ../../taskmacs.s ../../listopts.s \
#			$(OBJDIR)/lolevel.s ../../macros.s $(PORTINCL)/target.s
#	$(AS) ../../stackchk.s $(AFLAGS) -o $@

$(OBJDIR)/interrupt.o:	../../interrupt.s ../../taskmacs.s ../../listopts.s \
			$(OBJDIR)/lolevel.s ../../macros.s $(PORTINCL)/target.s
	$(AS) ../../interrupt.s $(AFLAGS) -o $@

$(OBJDIR)/suppasm.o:	../../suppasm.s ../../taskmacs.s ../../listopts.s \
			$(OBJDIR)/lolevel.s ../../macros.s $(PORTINCL)/target.s
	$(AS) ../../suppasm.s $(AFLAGS) -o $@

#$(OBJDIR)/stacklib.o:	../../stackchk.s ../../listopts.s $(OBJDIR)/lolevel.s \
#			../../macros.s ../../taskmacs.s $(PORTINCL)/target.s
#	$(AS) ../../stackchk.s $(AFLAGS) -PD 'LINKING_WITH_CLIB SETA 1' -o $@

$(OBJDIR)/serlasm.o:	../../serlasm.s ../../listopts.s $(OBJDIR)/lolevel.s \
			../../taskmacs.s ../../macros.s
	$(AS) ../../serlasm.s $(AFLAGS) -o $@

$(OBJDIR)/delay.o:	$(PORTINCL)/delay.s ../../listopts.s $(OBJDIR)/lolevel.s
	$(AS)	$(PORTINCL)/delay.s $(AFLAGS) -o $@

$(OBJDIR)/dccsup.o:	../../dcc/dccsup.s $(PORTINCL)/devconf.h
	$(AS)	../../dcc/dccsup.s $(AFLAGS) -o $@

$(OBJDIR)/e5_sendb.o:	../../logging/e5_sendb.s ../../listopts.s $(OBJDIR)/lolevel.s
	$(AS)	../../logging/e5_sendb.s $(AFLAGS) -o $@

$(OBJDIR)/e5_bc.o:		../../logging/e5_bc.s
	$(AS)	../../logging/e5_bc.s $(AFLAGS) -o $@

$(OBJDIR)/ladpasm.o:		../../logging/ladpasm.s $(PORTINCL)/devconf.h
	$(AS)	../../logging/ladpasm.s $(AFLAGS) -o $@


$(OBJDIR)/disass.o:	$(CLXINCL)/disass.c $(CLXINCL)/disass.h
	$(CC) $(CFLAGS) $(CFLdisass) -DNO_MALLOC -o $(OBJDIR)/disass.o $(CLXINCL)/disass.c

$(OBJDIR)/lolevel.s:	$(PORTINCL)/makelo.c ../../angel.h ../../adp.h ../../chandefs.h \
	  ../../devices.h ../../buffers.h ../../serlock.h $(PORTINCL)/devconf.h \
	  $(PORTINCL)/pid.h ../../stacks.h ../../serial.h ../../devdriv.h \
	  ../../devclnt.h ../../params.h ../../support.h ../../arm.h Makefile
	gcc -o $(OBJDIR)/makelo -I../.. -I$(PORTINCL) $(PORTINCL)/makelo.c \
			$(CDEFS) $(ENDIANDEFN)
	$(OBJDIR)/makelo > $(OBJDIR)/lolevel.s

# Extra dependencies for logging.o, for all the optional headers

$(OBJDIR)/logging.o:	../../logging/e5.h
$(OBJDIR)/logging.o:	../../logging/logserial.h
$(OBJDIR)/logging.o:	../../logging/log552.h
$(OBJDIR)/logging.o:	../../logging/logterm.h
$(OBJDIR)/logging.o:	../../logging/panicblk.h
$(OBJDIR)/logging.o:	../../logging/pidulate.h
$(OBJDIR)/logging.o:	../../logging/rombox.h

###

# Rules for creating dependencies, to standard output, which can then be
#  cut and pasted back into this makefile above.

depends: devices.dep devmisc.dep devraw.dep buffers.dep params.dep
depends: logging.dep msgbuild.dep st16c552.dep serraw.dep
depends: dccdrv.dep rombox.dep panicblk.dep stacks.dep serlock.dep

SORTSED=sort -u | sed -e 's/^/$$(OBJDIR)\//'


devices.dep:	$(PORTINCL)/devices.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevices) $(PORTINCL)/devices.c | $(SORTSED)
	@echo '$$(OBJDIR)/devices.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevices) -o $$(OBJDIR)/devices.o $?'
	@echo

devmisc.dep:	../../devmisc.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevmisc) ../../devmisc.c | $(SORTSED)
	@echo '$$(OBJDIR)/devmisc.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevmisc) -o $$(OBJDIR)/devmisc.o $?'
	@echo

devraw.dep:	../../devraw.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevraw) ../../devraw.c | $(SORTSED)
	@echo '$$(OBJDIR)/devraw.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevraw) -o $$(OBJDIR)/devraw.o $?'
	@echo

buffers.dep:	../../buffers.c
	@$(CC) -Mx $(CFLAGS) $(CFLbuffers) ../../buffers.c | $(SORTSED)
	@echo '$$(OBJDIR)/buffers.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLbuffers) -o $$(OBJDIR)/buffers.o $?'
	@echo

params.dep:	../../params.c
	@$(CC) -Mx $(CFLAGS) $(CFLparams) ../../params.c | $(SORTSED)
	@echo '$$(OBJDIR)/params.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLparams) -o $$(OBJDIR)/params.o $?'
	@echo

stacks.dep:	../../stacks.c
	@$(CC) -Mx $(CFLAGS) $(CFLstacks) ../../stacks.c | $(SORTSED)
	@echo '$$(OBJDIR)/stacks.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLstacks) -o $$(OBJDIR)/stacks.o $?'
	@echo

logging.dep:	../../logging.c
	@$(CC) -Mx $(CFLAGS) $(CFLlogging) -I../../logging ../../logging.c | $(SORTSED)
	@echo '$$(OBJDIR)/logging.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLlogging) -I../../logging -o $$(OBJDIR)/logging.o $?'
	@echo

serlock.dep:	../../serlock.c
	@$(CC) -Mx $(CFLAGS) $(CFLserlock) ../../serlock.c | $(SORTSED)
	@echo '$$(OBJDIR)/serlock.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLserlock) -o $$(OBJDIR)/serlock.o $?'
	@echo

msgbuild.dep:	../../msgbuild.c
	@$(CC) -Mx $(CFLAGS) $(CFLmsgbuild) ../../msgbuild.c | $(SORTSED)
	@echo '$$(OBJDIR)/msgbuild.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLmsgbuild) -o $$(OBJDIR)/msgbuild.o $?'
	@echo

st16c552.dep:	  $(PORTINCL)/st16c552.c
	@$(CC) -Mx $(CFLAGS) $(CFLst16c552) $(PORTINCL)/st16c552.c | $(SORTSED)
	@echo '$$(OBJDIR)/st16c552.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLst16c552) -o $$(OBJDIR)/st16c552.o $?'
	@echo

serraw.dep:	  ../../serraw.c
	@$(CC) -Mx $(CFLAGS) $(CFLserraw) ../../serraw.c | $(SORTSED)
	@echo '$$(OBJDIR)/serraw.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLserraw) -o $$(OBJDIR)/serraw.o $?'
	@echo

dccdrv.dep:	  ../../dcc/dccdrv.c
	@$(CC) -Mx $(CFLAGS) $(CFLdccdrv) ../../dcc/dccdrv.c | $(SORTSED)
	@echo '$$(OBJDIR)/dccdrv.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdccdrv) -o $$(OBJDIR)/dccdrv.o $?'
	@echo

rombox.dep:	  ../../logging/rombox.c
	@$(CC) -Mx $(CFLAGS) $(CFLrombox) ../../logging/rombox.c | $(SORTSED)
	@echo '$$(OBJDIR)/rombox.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLrombox) -o $$(OBJDIR)/rombox.o $?'
	@echo

panicblk.dep:	  ../../logging/panicblk.c
	@$(CC) -Mx $(CFLAGS) $(CFLpanicblk) ../../logging/panicblk.c | $(SORTSED)
	@echo '$$(OBJDIR)/panicblk.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLpanicblk) -o $$(OBJDIR)/panicblk.o $?'
	@echo

# And now the mechanically built dependencies

$(OBJDIR)/devices.o:	../../adp.h
$(OBJDIR)/devices.o:	../../angel.h
$(OBJDIR)/devices.o:	../../buffers.h
$(OBJDIR)/devices.o:	../../chandefs.h
$(OBJDIR)/devices.o:	../../devclnt.h
$(OBJDIR)/devices.o:	../../devdriv.h
$(OBJDIR)/devices.o:	../../devices.h
$(OBJDIR)/devices.o:	../../params.h
$(OBJDIR)/devices.o:	../../pid/devconf.h
$(OBJDIR)/devices.o:	../../pid/devices.c
$(OBJDIR)/devices.o:	../../pid/pid.h
$(OBJDIR)/devices.o:	../../pid/st16c552.h
$(OBJDIR)/devices.o:	../../ringbuff.h
$(OBJDIR)/devices.o:	../../rxtx.h
$(OBJDIR)/devices.o:	../../serlock.h
$(OBJDIR)/devices.o:	../../serring.h
$(OBJDIR)/devices.o:	../../stacks.h
$(OBJDIR)/devices.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdevices) -o $(OBJDIR)/devices.o ../../pid/devices.c

$(OBJDIR)/devmisc.o:	../../adp.h
$(OBJDIR)/devmisc.o:	../../angel.h
$(OBJDIR)/devmisc.o:	../../buffers.h
$(OBJDIR)/devmisc.o:	../../chandefs.h
$(OBJDIR)/devmisc.o:	../../devclnt.h
$(OBJDIR)/devmisc.o:	../../devdriv.h
$(OBJDIR)/devmisc.o:	../../devices.h
$(OBJDIR)/devmisc.o:	../../devmisc.c
$(OBJDIR)/devmisc.o:	../../logging.h
$(OBJDIR)/devmisc.o:	../../logtarget.h
$(OBJDIR)/devmisc.o:	../../params.h
$(OBJDIR)/devmisc.o:	../../pid/devconf.h
$(OBJDIR)/devmisc.o:	../../pid/pid.h
$(OBJDIR)/devmisc.o:	../../serlock.h
$(OBJDIR)/devmisc.o:	../../stacks.h
$(OBJDIR)/devmisc.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdevmisc) -o $(OBJDIR)/devmisc.o ../../devmisc.c

$(OBJDIR)/devraw.o:	../../adp.h
$(OBJDIR)/devraw.o:	../../angel.h
$(OBJDIR)/devraw.o:	../../buffers.h
$(OBJDIR)/devraw.o:	../../chandefs.h
$(OBJDIR)/devraw.o:	../../devclnt.h
$(OBJDIR)/devraw.o:	../../devdriv.h
$(OBJDIR)/devraw.o:	../../devices.h
$(OBJDIR)/devraw.o:	../../devraw.c
$(OBJDIR)/devraw.o:	../../devraw.h
$(OBJDIR)/devraw.o:	../../logging.h
$(OBJDIR)/devraw.o:	../../logtarget.h
$(OBJDIR)/devraw.o:	../../params.h
$(OBJDIR)/devraw.o:	../../pid/devconf.h
$(OBJDIR)/devraw.o:	../../pid/pid.h
$(OBJDIR)/devraw.o:	../../serlock.h
$(OBJDIR)/devraw.o:	../../stacks.h
$(OBJDIR)/devraw.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdevraw) -o $(OBJDIR)/devraw.o ../../devraw.c

$(OBJDIR)/buffers.o:	../../adp.h
$(OBJDIR)/buffers.o:	../../angel.h
$(OBJDIR)/buffers.o:	../../buffers.c
$(OBJDIR)/buffers.o:	../../buffers.h
$(OBJDIR)/buffers.o:	../../chandefs.h
$(OBJDIR)/buffers.o:	../../devices.h
$(OBJDIR)/buffers.o:	../../logging.h
$(OBJDIR)/buffers.o:	../../logtarget.h
$(OBJDIR)/buffers.o:	../../pid/devconf.h
$(OBJDIR)/buffers.o:	../../pid/pid.h
$(OBJDIR)/buffers.o:	../../serlock.h
$(OBJDIR)/buffers.o:	../../stacks.h
$(OBJDIR)/buffers.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLbuffers) -o $(OBJDIR)/buffers.o ../../buffers.c

$(OBJDIR)/params.o:	../../adp.h
$(OBJDIR)/params.o:	../../angel.h
$(OBJDIR)/params.o:	../../chandefs.h
$(OBJDIR)/params.o:	../../endian.h
$(OBJDIR)/params.o:	../../logging.h
$(OBJDIR)/params.o:	../../logtarget.h
$(OBJDIR)/params.o:	../../params.c
$(OBJDIR)/params.o:	../../params.h
$(OBJDIR)/params.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLparams) -o $(OBJDIR)/params.o ../../params.c

$(OBJDIR)/logging.o:	../../adp.h
$(OBJDIR)/logging.o:	../../angel.h
$(OBJDIR)/logging.o:	../../chandefs.h
$(OBJDIR)/logging.o:	../../logging.c
$(OBJDIR)/logging.o:	../../logging.h
$(OBJDIR)/logging.o:	../../logpriv.h
$(OBJDIR)/logging.o:	../../logtarget.h
$(OBJDIR)/logging.o:	../../pid/devconf.h
$(OBJDIR)/logging.o:	../../pid/pid.h
$(OBJDIR)/logging.o:	../../stacks.h
$(OBJDIR)/logging.o:	../../sys.h
$(OBJDIR)/logging.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLlogging) -I../../logging -o $(OBJDIR)/logging.o ../../logging.c

$(OBJDIR)/msgbuild.o:	../../adperr.h
$(OBJDIR)/msgbuild.o:	../../angel.h
$(OBJDIR)/msgbuild.o:	../../buffers.h
$(OBJDIR)/msgbuild.o:	../../chandefs.h
$(OBJDIR)/msgbuild.o:	../../channels.h
$(OBJDIR)/msgbuild.o:	../../devices.h
$(OBJDIR)/msgbuild.o:	../../endian.h
$(OBJDIR)/msgbuild.o:	../../msgbuild.c
$(OBJDIR)/msgbuild.o:	../../msgbuild.h
$(OBJDIR)/msgbuild.o:	../../pid/devconf.h
$(OBJDIR)/msgbuild.o:	../../pid/pid.h
$(OBJDIR)/msgbuild.o:	../../stacks.h
$(OBJDIR)/msgbuild.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLmsgbuild) -o $(OBJDIR)/msgbuild.o ../../msgbuild.c

$(OBJDIR)/st16c552.o:	../../adp.h
$(OBJDIR)/st16c552.o:	../../angel.h
$(OBJDIR)/st16c552.o:	../../buffers.h
$(OBJDIR)/st16c552.o:	../../chandefs.h
$(OBJDIR)/st16c552.o:	../../devclnt.h
$(OBJDIR)/st16c552.o:	../../devdriv.h
$(OBJDIR)/st16c552.o:	../../devices.h
$(OBJDIR)/st16c552.o:	../../logging.h
$(OBJDIR)/st16c552.o:	../../logtarget.h
$(OBJDIR)/st16c552.o:	../../params.h
$(OBJDIR)/st16c552.o:	../../pid/devconf.h
$(OBJDIR)/st16c552.o:	../../pid/pid.h
$(OBJDIR)/st16c552.o:	../../pid/st16c552.c
$(OBJDIR)/st16c552.o:	../../pid/st16c552.h
$(OBJDIR)/st16c552.o:	../../ringbuff.h
$(OBJDIR)/st16c552.o:	../../rxtx.h
$(OBJDIR)/st16c552.o:	../../serial.h
$(OBJDIR)/st16c552.o:	../../serlock.h
$(OBJDIR)/st16c552.o:	../../serring.h
$(OBJDIR)/st16c552.o:	../../stacks.h
$(OBJDIR)/st16c552.o:	../../support.h
$(OBJDIR)/st16c552.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLst16c552) -o $(OBJDIR)/st16c552.o ../../pid/st16c552.c

$(OBJDIR)/serraw.o:	../../adp.h
$(OBJDIR)/serraw.o:	../../angel.h
$(OBJDIR)/serraw.o:	../../buffers.h
$(OBJDIR)/serraw.o:	../../chandefs.h
$(OBJDIR)/serraw.o:	../../devappl.h
$(OBJDIR)/serraw.o:	../../devclnt.h
$(OBJDIR)/serraw.o:	../../devdriv.h
$(OBJDIR)/serraw.o:	../../devices.h
$(OBJDIR)/serraw.o:	../../devraw.h
$(OBJDIR)/serraw.o:	../../logging.h
$(OBJDIR)/serraw.o:	../../logtarget.h
$(OBJDIR)/serraw.o:	../../params.h
$(OBJDIR)/serraw.o:	../../pid/devconf.h
$(OBJDIR)/serraw.o:	../../pid/pid.h
$(OBJDIR)/serraw.o:	../../ringbuff.h
$(OBJDIR)/serraw.o:	../../rxtx.h
$(OBJDIR)/serraw.o:	../../serlock.h
$(OBJDIR)/serraw.o:	../../serraw.c
$(OBJDIR)/serraw.o:	../../serring.h
$(OBJDIR)/serraw.o:	../../stacks.h
$(OBJDIR)/serraw.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLserraw) -o $(OBJDIR)/serraw.o ../../serraw.c

$(OBJDIR)/dccdrv.o:	../../adp.h
$(OBJDIR)/dccdrv.o:	../../angel.h
$(OBJDIR)/dccdrv.o:	../../buffers.h
$(OBJDIR)/dccdrv.o:	../../chandefs.h
$(OBJDIR)/dccdrv.o:	../../dcc/dccdrv.c
$(OBJDIR)/dccdrv.o:	../../dcc/dccdrv.h
$(OBJDIR)/dccdrv.o:	../../devclnt.h
$(OBJDIR)/dccdrv.o:	../../devdriv.h
$(OBJDIR)/dccdrv.o:	../../devices.h
$(OBJDIR)/dccdrv.o:	../../logging.h
$(OBJDIR)/dccdrv.o:	../../logtarget.h
$(OBJDIR)/dccdrv.o:	../../params.h
$(OBJDIR)/dccdrv.o:	../../pid/devconf.h
$(OBJDIR)/dccdrv.o:	../../pid/pid.h
$(OBJDIR)/dccdrv.o:	../../ringbuff.h
$(OBJDIR)/dccdrv.o:	../../rxtx.h
$(OBJDIR)/dccdrv.o:	../../serlock.h
$(OBJDIR)/dccdrv.o:	../../serring.h
$(OBJDIR)/dccdrv.o:	../../stacks.h
$(OBJDIR)/dccdrv.o:	../../support.h
$(OBJDIR)/dccdrv.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdccdrv) -o $(OBJDIR)/dccdrv.o ../../dcc/dccdrv.c

$(OBJDIR)/rombox.o:	../../logging/rombox.c
$(OBJDIR)/rombox.o:	../../logging/rombox.h
$(OBJDIR)/rombox.o:	../../pid/devconf.h
$(OBJDIR)/rombox.o:	../../pid/pid.h
$(OBJDIR)/rombox.o:	../../stacks.h
$(OBJDIR)/rombox.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLrombox) -o $(OBJDIR)/rombox.o ../../logging/rombox.c

$(OBJDIR)/panicblk.o:	../../logging.h
$(OBJDIR)/panicblk.o:	../../logging/panicblk.c
$(OBJDIR)/panicblk.o:	../../logging/panicblk.h
$(OBJDIR)/panicblk.o:	../../logtarget.h
$(OBJDIR)/panicblk.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLpanicblk) -o $(OBJDIR)/panicblk.o ../../logging/panicblk.c

$(OBJDIR)/stacks.o:	../../pid/devconf.h
$(OBJDIR)/stacks.o:	../../pid/pid.h
$(OBJDIR)/stacks.o:	../../stacks.c
$(OBJDIR)/stacks.o:	../../stacks.h
$(OBJDIR)/stacks.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLstacks) -o $(OBJDIR)/stacks.o ../../stacks.c

$(OBJDIR)/serlock.o:	../../adp.h
$(OBJDIR)/serlock.o:	../../angel.h
$(OBJDIR)/serlock.o:	../../arm.h
$(OBJDIR)/serlock.o:	../../buffers.h
$(OBJDIR)/serlock.o:	../../chandefs.h
$(OBJDIR)/serlock.o:	../../devclnt.h
$(OBJDIR)/serlock.o:	../../devdriv.h
$(OBJDIR)/serlock.o:	../../devices.h
$(OBJDIR)/serlock.o:	../../logging.h
$(OBJDIR)/serlock.o:	../../logtarget.h
$(OBJDIR)/serlock.o:	../../params.h
$(OBJDIR)/serlock.o:	../../pid/devconf.h
$(OBJDIR)/serlock.o:	../../pid/pid.h
$(OBJDIR)/serlock.o:	../../serlock.c
$(OBJDIR)/serlock.o:	../../serlock.h
$(OBJDIR)/serlock.o:	../../stacks.h
$(OBJDIR)/serlock.o:	../../support.h
$(OBJDIR)/serlock.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLserlock) -o $(OBJDIR)/serlock.o ../../serlock.c

