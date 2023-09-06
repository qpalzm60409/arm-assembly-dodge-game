##
# HP-UX Makefile for Full Angel on a PID7T
#
#  Current $Revision: 1.6.4.3 $
# Revising $Author: tdouglas $
#  Checkin $Date: 1998/10/22 17:17:59 $
#
# Copyright (c) 1996, 1997, 1998 ARM Ltd.
# All Rights Reserved
#

# The following options on the make command line are supported:
#

CLXINCL=	../../../clx
PORTINCL=	../../pid
CLIBINCL=	../../../500/cl
DCCINCL=	../../dcc

# Where the Fusion IP stack includes are
FUSIONINCL=	../../../fusion/incl
# Angels interface to IP
ANGIPINCL=	../../ipstack
# Angel's interface to Fusion
ANGFUSIONINCL=	../../ipstack/fusion/arm

LATE_STARTUP=	0
THUMB_SUPPORT=  1
# MINIMAL_ANGEL=0   ## hardwired

# Set this to 1 for debugging version of Angel, 0 for Release.
DEBUG=		0

# Use this to specify where Angel's data space should be. Needs ~24Kb
RWADDR=		0x02040000

# Use this ROADDR to run Angel from SRAM; this is aliased at 0x00008000
# so specifying 2048000 means that angel will also appear at 0x00048800
ROADDR=		0x02048000

# 
ANGELVSN=	0x0001

# The default is to build without ethernet support
ETHERNET_SUPPORTED=0

# Define this only if DEBUG_METHOD is set to logterm in devconf.h
LOGTERM_DEBUGGING=1

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

# The appropriate one of these must be left uncommented.
# in order to build for the required endianess.
#
ENDIANFLAG= -li
ENDIANDEFN= -DLITTLE_ENDIAN
ENDIANSFX= l
ENDIANOBJDIR= little

# ENDIANFLAG=   -bi
# ENDIANDEFN=   -DBIG_ENDIAN
# ENDIANSFX= b
# ENDIANOBJDIR= big

# The current setup for sunos and solaris is to emulate the pc structure
# having release (_rel) and debug (_deb) directories these must be 
# edited to set the directory to reflect the build. Default is release

# OBJDIR = little_deb
# OBJDIR = big_deb
# OBJDIR = little_ether_deb
# OBJDIR = big_ether_deb
 OBJDIR = little_rel
# OBJDIR = big_rel
# OBJDIR = little_ether_rel
# OBJDIR = big_ether_rel

# These MUST MATCH the debug flags used to compile the Fusion stack,
# and this macro must only be uncommented if the Fusion stack has been
# compiled with debug flags.
#
# FUSION_DEBUG= -DDEBUG -DSKEPTIC -DKDBG -DM_HISTORY

# The default is not to build the fusion source, but to obtain the objects
# from the Supplied directory. Overwrite this on the command line by
# using FUSION_BUILD=1
FUSION_BUILD = 0

FUSIONOBJDIR= ../Supplied/little_ether
# FUSIONOBJDIR= ../Supplied/big_ether
# FUSIONOBJDIR= ../Supplied/little
# FUSIONOBJDIR= ../Supplied/big

# FUSIONOBJDIR= $(OBJDIR)
#Uncomment this if ETHERNET_SUPPORTED = 1 above (if the ethernet support is
# required

#ETHEROBJS= $(FUSIONOBJDIR)/olicom.o $(FUSIONOBJDIR)/i82595.o \
	     $(FUSIONOBJDIR)/ethernet.o $(FUSIONOBJDIR)/fusion.o
# ETHEROBJS=


#
# now make compiler and assembler arguments
#
CCENDIAN=	$(ENDIANFLAG) $(ENDIANDEFN)
ASENDIAN=	$(ENDIANFLAG)

CDEFS=	-DTARGET -DTHUMB_SUPPORT=$(THUMB_SUPPORT) -DANGELVSN=$(ANGELVSN) \
	-DROADDR=$(ROADDR) -DRWADDR=$(RWADDR) -DLOGTERM_DEBUGGING=$(LOGTERM_DEBUGGING) \
	-DMINIMAL_ANGEL=0 -DLATE_STARTUP=$(LATE_STARTUP) \
	-DDEBUG=$(DEBUG) -DASSERT_ENABLED=$(ASSERT_ENABLED) \
	-DETHERNET_SUPPORTED=$(ETHERNET_SUPPORTED)

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

# CC_DEBUG = -g+
# CC_DEBUG = -g -DCLEAR_BUFFERS
# CC_DEBUG = -g -DNO_LOG_INFO

#default for debug version is :-

# CC_DEBUG = -go
# ASM_DEBUG= -g

# default for release version is :-

CC_DEBUG= -O2
ASM_DEBUG= 

ASSERT_ENABLED= 0

# Leave this one alone - comment, uncomment and edit the two macros above
# as necessary
#
ALL_DEBUG=	$(CC_DEBUG) $(FUSION_DEBUG)

CFLAGS=	-fc -fah -apcs $(APCS) $(CPUTYPE) -c \
	-I../.. -I$(DCCINCL) -I$(PORTINCL) -I$(CLXINCL) \
	-I$(ANGIPINCL) -I$(FUSIONINCL) -I$(ANGFUSIONINCL) \
	$(CCENDIAN) $(CFL) $(CDEFS) $(ALL_DEBUG) $(CFLAG_BUILD_NUMBER) $(CFLAG_BUILD_DATE) \
  $(CFLAG_ARM_RELEASE)


AFLAGS=	-32 -apcs $(APCS) $(CPUTYPE) $(ASM_DEBUG) $(ASENDIAN) -I$(OBJDIR) \
	-I../.. -I$(PORTINCL) -I$(CLIBINCL) \
	-PD "LOGTERM_DEBUGGING SETA $(LOGTERM_DEBUGGING)" \
	-PD "ANGELVSN SETA $(ANGELVSN)" \
	-PD "DEBUG SETA $(DEBUG)" \
	-PD "LATE_STARTUP SETA $(LATE_STARTUP)" \
	-PD "ROADDR SETA $(ROADDR)" \
	-PD "THUMB_SUPPORT SETA $(THUMB_SUPPORT)" \
	-PD "ASSERT_ENABLED SETA $(ASSERT_ENABLED)" \
	-PD "MINIMAL_ANGEL SETA 0" \
	-PD "ETHERNET_SUPPORTED SETA $(ETHERNET_SUPPORTED)"

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
PORTOBJS=	$(OBJDIR)/devices.o $(OBJDIR)/pcmcia.o $(OBJDIR)/timer.o \
		$(OBJDIR)/delay.o

# Port-specific Objects included in the driver library
PORTDRIVERS=    $(OBJDIR)/st16c552.o

# General Angel Objects always included in the image
ANGELOBJS=	$(OBJDIR)/debug.o $(OBJDIR)/debughwi.o $(OBJDIR)/info.o \
		$(OBJDIR)/ctrl.o $(OBJDIR)/debugos.o $(OBJDIR)/boot.o \
		$(OBJDIR)/serlock.o $(OBJDIR)/serlasm.o \
		$(OBJDIR)/devmisc.o $(OBJDIR)/devappl.o $(OBJDIR)/devshare.o \
		$(OBJDIR)/devraw.o $(OBJDIR)/devclnt.o $(OBJDIR)/channels.o \
		$(OBJDIR)/buffers.o $(OBJDIR)/rx.o $(OBJDIR)/tx.o \
		$(OBJDIR)/crc.o $(OBJDIR)/msgbuild.o $(OBJDIR)/logging.o \
		$(OBJDIR)/params.o $(OBJDIR)/except.o $(OBJDIR)/startlib.o \
		$(OBJDIR)/sys.o $(OBJDIR)/serpkt.o $(OBJDIR)/serraw.o \
		$(OBJDIR)/interrupt.o $(OBJDIR)/suppasm.o $(OBJDIR)/prof.o \
		$(OBJDIR)/stacks.o 

# General Angel Objects which may be included in the image
ANGELDRIVERS=   $(OBJDIR)/e5_sendb.o $(OBJDIR)/rombox.o $(OBJDIR)/panicblk.o \
		$(OBJDIR)/log552.o $(OBJDIR)/logterm.o $(OBJDIR)/delay.o \
		$(ETHEROBJS) $(OBJDIR)/disass.o

DCCDRIVER=	$(OBJDIR)/dccdrv.o $(OBJDIR)/dccsup.o

DRIVERS=	$(ANGELDRIVERS) $(DCCDRIVER) $(PORTDRIVERS)

OBJS=		$(ANGELOBJS) $(PORTOBJS)


all: $(OBJDIR)/angel.rom

# Make sure the object directory exists before we try to compile things
# into it, even with multi-job gnumake runs.
$(OBJDIR)/dir_exists:
	@if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi
	touch $(OBJDIR)/dir_exists

$(OBJS) $(DRIVERS) $(OBJDIR)/startrom.o: $(OBJDIR)/dir_exists


$(OBJDIR)/angel.rom:	$(OBJS) $(OBJDIR)/startrom.o $(OBJDIR)/drivers.a
	$(LD) $(LDFLAGS) $(OBJS) $(OBJDIR)/startrom.o $(OBJDIR)/drivers.a \
		-ro-base $(ROADDR) -rw-base $(RWADDR) -symbols $@.sym.lst \
		-first $(FIRST) -o $(OBJDIR)/angel.elf
	$(FROMELF) -nozeropad $(OBJDIR)/angel.elf -bin $(OBJDIR)/angel.rom
	$(FROMELF) $(OBJDIR)/angel.elf -aifbin $(OBJDIR)/angel.sym
	sort -f $@.sym.lst > $@.alpha.lst
	sort -b +1 $@.sym.lst > $@.num.lst

$(OBJDIR)/drivers.a:	$(DRIVERS)
	$(AR) -c -o $@ $(DRIVERS)

###

clean:
	-if [ -d $(OBJDIR) ]; then \
	  $(RM) $(DRIVERS) $(OBJS) \
	  		$(OBJDIR)/makelo $(OBJDIR)/drivers.a $(OBJDIR)/lolevel.s \
	  		$(OBJDIR)/suppmsg.o $(OBJDIR)/devappl.o $(OBJDIR)/angel.rom \
			$(OBJDIR)/angel.sym $(OBJDIR)/*.lst \
			$(OBJDIR)/startrom.o $(OBJDIR)/ipconfig.o ; \
	  $(RMDIR) $(OBJDIR); \
	fi
	cd $(ANGFUSIONINCL);  gnumake ENDIAN=$(ENDIAN) clean

#
# fusion.o is a partially linked combination of
# the Fusion stack and its configuration file
#
FUSION_LIB=../../ipstack/libfns35.32$(ENDIANSFX)

# Yes, the target here is correct in not having "ipstack" in it!

$(FUSION_LIB):	MAKE_FUSION_ALWAYS
	cd ../../ipstack/fusion/arm; gnumake CDEFS="$(CDEFS)" PORT_INCLUDE=../$(PORTINCL) ENDIAN=$(ENDIAN) ../../libfns35.32$(ENDIANSFX)

fusion-depend: MAKE_FUSION_ALWAYS
	cd ../../ipstack/fusion/arm; gnumake CDEFS="$(CDEFS)" PORT_INCLUDE=../$(PORTINCL) ENDIAN=$(ENDIAN) depend

MAKE_FUSION_ALWAYS:
	@echo performing build of fusion stack

$(OBJDIR)/fusion.o:	$(OBJDIR)/ipconfig.o $(FUSION_LIB)
	$(LD) -aof -o $@ $(OBJDIR)/ipconfig.o $(FUSION_LIB)

$(OBJDIR)/etherlib.32$(ENDIANSFX): $(ETHEROBJS)
	$(AR) -c -o $@ $(ETHEROBJS)


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
	cc -Ae -o $(OBJDIR)/makelo -I../.. -I$(PORTINCL) $(PORTINCL)/makelo.c $(CDEFS) $(ENDIANDEFN)
	$(OBJDIR)/makelo > $(OBJDIR)/lolevel.s

# Extra dependencies for logging.o, for all the optional headers

$(OBJDIR)/logging.o:	../../logging/e5.h
$(OBJDIR)/logging.o:	../../logging/logadp.h
$(OBJDIR)/logging.o:	../../logging/panicblk.h
$(OBJDIR)/logging.o:	../../logging/pidulate.h
$(OBJDIR)/logging.o:	../../logging/rombox.h

###

# Rules for creating dependencies, to standard output, which can then be
#  cut and pasted back into this makefile (see below).

depends: devices.dep devmisc.dep devraw.dep buffers.dep params.dep
depends: logging.dep msgbuild.dep st16c552.dep serraw.dep
depends: dccdrv.dep rombox.dep panicblk.dep stacks.dep serlock.dep
depends: debug.dep debughwi.dep info.dep ctrl.dep debugos.dep boot.dep
depends: devappl.dep devshare.dep devclnt.dep channels.dep prof.dep
depends: rx.dep tx.dep crc.dep sys.dep serpkt.dep logadp.dep
depends: logterm.dep log552.dep pcmcia.dep olicom.dep 82595.dep
depends: ipconfig.dep timer.dep ethernet.dep

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

debug.dep:	../../debug.c
	@$(CC) -Mx $(CFLAGS) $(CFLdebug) ../../debug.c | $(SORTSED)
	@echo '$$(OBJDIR)/debug.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdebug) -o $$(OBJDIR)/debug.o $?'
	@echo

debughwi.dep:	../../debughwi.c
	@$(CC) -Mx $(CFLAGS) $(CFLdebughwi) ../../debughwi.c | $(SORTSED)
	@echo '$$(OBJDIR)/debughwi.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdebughwi) -o $$(OBJDIR)/debughwi.o $?'
	@echo

info.dep:   ../../info.c
	@$(CC) -Mx $(CFLAGS) $(CFLinfo) ../../info.c | $(SORTSED)
	@echo '$$(OBJDIR)/info.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLinfo) -o $$(OBJDIR)/info.o $?'
	@echo

ctrl.dep:   ../../ctrl.c
	@$(CC) -Mx $(CFLAGS) $(CFLctrl) ../../ctrl.c | $(SORTSED)
	@echo '$$(OBJDIR)/ctrl.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLctrl) -o $$(OBJDIR)/ctrl.o $?'
	@echo

debugos.dep:	../../debugos.c
	@$(CC) -Mx $(CFLAGS) $(CFLdebugos) ../../debugos.c | $(SORTSED)
	@echo '$$(OBJDIR)/debugos.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdebugos) -o $$(OBJDIR)/debugos.o $?'
	@echo

boot.dep:	../../boot.c
	@$(CC) -Mx $(CFLAGS) $(CFLboot) ../../boot.c | $(SORTSED)
	@echo '$$(OBJDIR)/boot.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLboot) -o $$(OBJDIR)/boot.o $?'
	@echo

devappl.dep:	../../devappl.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevappl) ../../devappl.c | $(SORTSED)
	@echo '$$(OBJDIR)/devappl.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevappl) -o $$(OBJDIR)/devappl.o $?'
	@echo

devshare.dep:	../../devshare.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevshare) ../../devshare.c | $(SORTSED)
	@echo '$$(OBJDIR)/devshare.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevshare) -o $$(OBJDIR)/devshare.o $?'
	@echo

devclnt.dep:	../../devclnt.c
	@$(CC) -Mx $(CFLAGS) $(CFLdevclnt) ../../devclnt.c | $(SORTSED)
	@echo '$$(OBJDIR)/devclnt.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLdevclnt) -o $$(OBJDIR)/devclnt.o $?'
	@echo

channels.dep:	../../channels.c
	@$(CC) -Mx $(CFLAGS) $(CFLchannels) ../../channels.c | $(SORTSED)
	@echo '$$(OBJDIR)/channels.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLchannels) -o $$(OBJDIR)/channels.o $?'
	@echo

rx.dep: ../../rx.c
	@$(CC) -Mx $(CFLAGS) $(CFLrx) ../../rx.c | $(SORTSED)
	@echo '$$(OBJDIR)/rx.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLrx) -o $$(OBJDIR)/rx.o $?'
	@echo

tx.dep: ../../tx.c
	@$(CC) -Mx $(CFLAGS) $(CFLtx) ../../tx.c | $(SORTSED)
	@echo '$$(OBJDIR)/tx.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLtx) -o $$(OBJDIR)/tx.o $?'
	@echo

crc.dep:	../../crc.c
	@$(CC) -Mx $(CFLAGS) $(CFLcrc) ../../crc.c | $(SORTSED)
	@echo '$$(OBJDIR)/crc.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLcrc) -o $$(OBJDIR)/crc.o $?'
	@echo

sys.dep:	../../sys.c
	@$(CC) -Mx $(CFLAGS) $(CFLsys) ../../sys.c | $(SORTSED)
	@echo '$$(OBJDIR)/sys.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLsys) -o $$(OBJDIR)/sys.o $?'
	@echo

pcmcia.dep:	  $(PORTINCL)/pcmcia.c
	@$(CC) -Mx $(CFLAGS) $(CFLpcmcia) $(PORTINCL)/pcmcia.c | $(SORTSED)
	@echo '$$(OBJDIR)/pcmcia.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLpcmcia) -o $$(OBJDIR)/pcmcia.o $?'
	@echo

olicom.dep:	  $(PORTINCL)/olicom.c
	@$(CC) -Mx $(CFLAGS) $(CFLolicom) $(PORTINCL)/olicom.c | $(SORTSED)
	@echo '$$(OBJDIR)/olicom.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLolicom) -o $$(OBJDIR)/olicom.o $?'
	@echo

82595.dep:	  $(PORTINCL)/82595.c
	@$(CC) -Mx $(CFLAGS) $(CFL82595) -o $(OBJDIR)/i82595.o $(PORTINCL)/82595.c | $(SORTSED)
	@echo '$$(OBJDIR)/i82595.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFL82595) -o $$(OBJDIR)/i82595.o $?'
	@echo

ethernet.dep:	  $(PORTINCL)/ethernet.c
	@$(CC) -Mx $(CFLAGS) $(CFLethernet) $(PORTINCL)/ethernet.c | $(SORTSED)
	@echo '$$(OBJDIR)/ethernet.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLethernet) -o $$(OBJDIR)/ethernet.o $?'
	@echo

ipconfig.dep:	../../ipstack/ipconfig.c
	@$(CC) -Mx $(CFLAGS) $(CFLipconfig) ../../ipstack/ipconfig.c | $(SORTSED)
	@echo '$$(OBJDIR)/ipconfig.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLipconfig) -o $$(OBJDIR)/ipconfig.o $?'
	@echo

serpkt.dep:	  ../../serpkt.c
	@$(CC) -Mx $(CFLAGS) $(CFLserpkt) ../../serpkt.c | $(SORTSED)
	@echo '$$(OBJDIR)/serpkt.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLserpkt) -o $$(OBJDIR)/serpkt.o $?'
	@echo

log552.dep:	  ../../logging/log552.c
	@$(CC) -Mx $(CFLAGS) $(CFLlog552) ../../logging/log552.c | $(SORTSED)
	@echo '$$(OBJDIR)/log552.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLlog552) -o $$(OBJDIR)/log552.o $?'
	@echo

logterm.dep:	  ../../logging/logterm.c
	@$(CC) -Mx $(CFLAGS) $(CFLlogterm) ../../logging/logterm.c | $(SORTSED)
	@echo '$$(OBJDIR)/logterm.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLlogterm) -o $$(OBJDIR)/logterm.o $?'
	@echo

logadp.dep:	  ../../logging/logadp.c
	@$(CC) -Mx $(CFLAGS) $(CFLlogadp) ../../logging/logadp.c | $(SORTSED)
	@echo '$$(OBJDIR)/logadp.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLlogadp) -o $$(OBJDIR)/logadp.o $?'
	@echo

prof.dep:	../../prof.c
	@$(CC) -Mx $(CFLAGS) $(CFLprof) ../../prof.c | $(SORTSED)
	@echo '$$(OBJDIR)/prof.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLprof) -o $$(OBJDIR)/prof.o $?'
	@echo

timer.dep:	$(PORTINCL)/timer.c
	@$(CC) -Mx $(CFLAGS) $(CFLtimer) $(PORTINCL)/timer.c | $(SORTSED)
	@echo '$$(OBJDIR)/timer.o:	$(PORTINCL)/devconf.h'
	@echo '	$$(CC) $$(CFLAGS) $$(CFLtimer) -o $$(OBJDIR)/timer.o $?'
	@echo


# And now the mechanically built dependencies. 

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
$(OBJDIR)/devices.o:	../../pid/pcmcia.h
$(OBJDIR)/devices.o:	../../pid/pid.h
$(OBJDIR)/devices.o:	../../pid/st16c552.h
$(OBJDIR)/devices.o:	../../prof.h
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

$(OBJDIR)/debug.o:	../../adp.h
$(OBJDIR)/debug.o:	../../adperr.h
$(OBJDIR)/debug.o:	../../angel.h
$(OBJDIR)/debug.o:	../../arm.h
$(OBJDIR)/debug.o:	../../buffers.h
$(OBJDIR)/debug.o:	../../chandefs.h
$(OBJDIR)/debug.o:	../../channels.h
$(OBJDIR)/debug.o:	../../debug.c
$(OBJDIR)/debug.o:	../../debug.h
$(OBJDIR)/debug.o:	../../debughwi.h
$(OBJDIR)/debug.o:	../../devices.h
$(OBJDIR)/debug.o:	../../endian.h
$(OBJDIR)/debug.o:	../../logging.h
$(OBJDIR)/debug.o:	../../logtarget.h
$(OBJDIR)/debug.o:	../../msgbuild.h
$(OBJDIR)/debug.o:	../../pid/devconf.h
$(OBJDIR)/debug.o:	../../pid/pid.h
$(OBJDIR)/debug.o:	../../serlock.h
$(OBJDIR)/debug.o:	../../stacks.h
$(OBJDIR)/debug.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdebug) -o $(OBJDIR)/debug.o ../../debug.c

$(OBJDIR)/debughwi.o:	../../adp.h
$(OBJDIR)/debughwi.o:	../../adperr.h
$(OBJDIR)/debughwi.o:	../../angel.h
$(OBJDIR)/debughwi.o:	../../arm.h
$(OBJDIR)/debughwi.o:	../../buffers.h
$(OBJDIR)/debughwi.o:	../../chandefs.h
$(OBJDIR)/debughwi.o:	../../channels.h
$(OBJDIR)/debughwi.o:	../../coproc.h
$(OBJDIR)/debughwi.o:	../../debug.h
$(OBJDIR)/debughwi.o:	../../debughwi.c
$(OBJDIR)/debughwi.o:	../../debughwi.h
$(OBJDIR)/debughwi.o:	../../debugos.h
$(OBJDIR)/debughwi.o:	../../devices.h
$(OBJDIR)/debughwi.o:	../../endian.h
$(OBJDIR)/debughwi.o:	../../logging.h
$(OBJDIR)/debughwi.o:	../../logtarget.h
$(OBJDIR)/debughwi.o:	../../msgbuild.h
$(OBJDIR)/debughwi.o:	../../pid/devconf.h
$(OBJDIR)/debughwi.o:	../../pid/pid.h
$(OBJDIR)/debughwi.o:	../../stacks.h
$(OBJDIR)/debughwi.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdebughwi) -o $(OBJDIR)/debughwi.o ../../debughwi.c

$(OBJDIR)/info.o:	../../adp.h
$(OBJDIR)/info.o:	../../adperr.h
$(OBJDIR)/info.o:	../../angel.h
$(OBJDIR)/info.o:	../../arm.h
$(OBJDIR)/info.o:	../../buffers.h
$(OBJDIR)/info.o:	../../chandefs.h
$(OBJDIR)/info.o:	../../channels.h
$(OBJDIR)/info.o:	../../coproc.h
$(OBJDIR)/info.o:	../../debughwi.h
$(OBJDIR)/info.o:	../../debugos.h
$(OBJDIR)/info.o:	../../devices.h
$(OBJDIR)/info.o:	../../endian.h
$(OBJDIR)/info.o:	../../info.c
$(OBJDIR)/info.o:	../../logging.h
$(OBJDIR)/info.o:	../../logtarget.h
$(OBJDIR)/info.o:	../../msgbuild.h
$(OBJDIR)/info.o:	../../pid/devconf.h
$(OBJDIR)/info.o:	../../pid/pid.h
$(OBJDIR)/info.o:	../../stacks.h
$(OBJDIR)/info.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLinfo) -o $(OBJDIR)/info.o ../../info.c

$(OBJDIR)/ctrl.o:	../../adp.h
$(OBJDIR)/ctrl.o:	../../adperr.h
$(OBJDIR)/ctrl.o:	../../angel.h
$(OBJDIR)/ctrl.o:	../../arm.h
$(OBJDIR)/ctrl.o:	../../buffers.h
$(OBJDIR)/ctrl.o:	../../chandefs.h
$(OBJDIR)/ctrl.o:	../../channels.h
$(OBJDIR)/ctrl.o:	../../coproc.h
$(OBJDIR)/ctrl.o:	../../ctrl.c
$(OBJDIR)/ctrl.o:	../../debug.h
$(OBJDIR)/ctrl.o:	../../debughwi.h
$(OBJDIR)/ctrl.o:	../../debugos.h
$(OBJDIR)/ctrl.o:	../../devices.h
$(OBJDIR)/ctrl.o:	../../logging.h
$(OBJDIR)/ctrl.o:	../../logtarget.h
$(OBJDIR)/ctrl.o:	../../msgbuild.h
$(OBJDIR)/ctrl.o:	../../sys.h
$(OBJDIR)/ctrl.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLctrl) -o $(OBJDIR)/ctrl.o ../../ctrl.c

$(OBJDIR)/debugos.o:	../../adp.h
$(OBJDIR)/debugos.o:	../../angel.h
$(OBJDIR)/debugos.o:	../../arm.h
$(OBJDIR)/debugos.o:	../../buffers.h
$(OBJDIR)/debugos.o:	../../chandefs.h
$(OBJDIR)/debugos.o:	../../coproc.h
$(OBJDIR)/debugos.o:	../../debug.h
$(OBJDIR)/debugos.o:	../../debughwi.h
$(OBJDIR)/debugos.o:	../../debugos.c
$(OBJDIR)/debugos.o:	../../debugos.h
$(OBJDIR)/debugos.o:	../../devclnt.h
$(OBJDIR)/debugos.o:	../../devices.h
$(OBJDIR)/debugos.o:	../../endian.h
$(OBJDIR)/debugos.o:	../../logging.h
$(OBJDIR)/debugos.o:	../../logtarget.h
$(OBJDIR)/debugos.o:	../../pid/devconf.h
$(OBJDIR)/debugos.o:	../../pid/pid.h
$(OBJDIR)/debugos.o:	../../serlock.h
$(OBJDIR)/debugos.o:	../../stacks.h
$(OBJDIR)/debugos.o:	../../support.h
$(OBJDIR)/debugos.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdebugos) -o $(OBJDIR)/debugos.o ../../debugos.c

$(OBJDIR)/boot.o:	../../../clx/toolver.h
$(OBJDIR)/boot.o:	../../adp.h
$(OBJDIR)/boot.o:	../../adperr.h
$(OBJDIR)/boot.o:	../../angel.h
$(OBJDIR)/boot.o:	../../arm.h
$(OBJDIR)/boot.o:	../../boot.c
$(OBJDIR)/boot.o:	../../buffers.h
$(OBJDIR)/boot.o:	../../chandefs.h
$(OBJDIR)/boot.o:	../../channels.h
$(OBJDIR)/boot.o:	../../coproc.h
$(OBJDIR)/boot.o:	../../debugos.h
$(OBJDIR)/boot.o:	../../devclnt.h
$(OBJDIR)/boot.o:	../../devdriv.h
$(OBJDIR)/boot.o:	../../devices.h
$(OBJDIR)/boot.o:	../../endian.h
$(OBJDIR)/boot.o:	../../logging.h
$(OBJDIR)/boot.o:	../../logtarget.h
$(OBJDIR)/boot.o:	../../msgbuild.h
$(OBJDIR)/boot.o:	../../params.h
$(OBJDIR)/boot.o:	../../pid/banner.h
$(OBJDIR)/boot.o:	../../pid/devconf.h
$(OBJDIR)/boot.o:	../../pid/pid.h
$(OBJDIR)/boot.o:	../../serlock.h
$(OBJDIR)/boot.o:	../../stacks.h
$(OBJDIR)/boot.o:	../../support.h
$(OBJDIR)/boot.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLboot) -o $(OBJDIR)/boot.o ../../boot.c

$(OBJDIR)/devappl.o:	../../angel.h
$(OBJDIR)/devappl.o:	../../arm.h
$(OBJDIR)/devappl.o:	../../buffers.h
$(OBJDIR)/devappl.o:	../../chandefs.h
$(OBJDIR)/devappl.o:	../../devappl.c
$(OBJDIR)/devappl.o:	../../devappl.h
$(OBJDIR)/devappl.o:	../../devices.h
$(OBJDIR)/devappl.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdevappl) -o $(OBJDIR)/devappl.o ../../devappl.c

$(OBJDIR)/devshare.o:	../../adp.h
$(OBJDIR)/devshare.o:	../../angel.h
$(OBJDIR)/devshare.o:	../../arm.h
$(OBJDIR)/devshare.o:	../../buffers.h
$(OBJDIR)/devshare.o:	../../chandefs.h
$(OBJDIR)/devshare.o:	../../devclnt.h
$(OBJDIR)/devshare.o:	../../devdriv.h
$(OBJDIR)/devshare.o:	../../devices.h
$(OBJDIR)/devshare.o:	../../devraw.h
$(OBJDIR)/devshare.o:	../../devshare.c
$(OBJDIR)/devshare.o:	../../logging.h
$(OBJDIR)/devshare.o:	../../logtarget.h
$(OBJDIR)/devshare.o:	../../params.h
$(OBJDIR)/devshare.o:	../../pid/devconf.h
$(OBJDIR)/devshare.o:	../../pid/pid.h
$(OBJDIR)/devshare.o:	../../serlock.h
$(OBJDIR)/devshare.o:	../../stacks.h
$(OBJDIR)/devshare.o:	../../support.h
$(OBJDIR)/devshare.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdevshare) -o $(OBJDIR)/devshare.o ../../devshare.c

$(OBJDIR)/devclnt.o:	../../adp.h
$(OBJDIR)/devclnt.o:	../../angel.h
$(OBJDIR)/devclnt.o:	../../buffers.h
$(OBJDIR)/devclnt.o:	../../chandefs.h
$(OBJDIR)/devclnt.o:	../../devclnt.c
$(OBJDIR)/devclnt.o:	../../devclnt.h
$(OBJDIR)/devclnt.o:	../../devdriv.h
$(OBJDIR)/devclnt.o:	../../devices.h
$(OBJDIR)/devclnt.o:	../../logging.h
$(OBJDIR)/devclnt.o:	../../logtarget.h
$(OBJDIR)/devclnt.o:	../../params.h
$(OBJDIR)/devclnt.o:	../../pid/devconf.h
$(OBJDIR)/devclnt.o:	../../pid/pid.h
$(OBJDIR)/devclnt.o:	../../serlock.h
$(OBJDIR)/devclnt.o:	../../stacks.h
$(OBJDIR)/devclnt.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLdevclnt) -o $(OBJDIR)/devclnt.o ../../devclnt.c

$(OBJDIR)/channels.o:	../../adp.h
$(OBJDIR)/channels.o:	../../adperr.h
$(OBJDIR)/channels.o:	../../angel.h
$(OBJDIR)/channels.o:	../../buffers.h
$(OBJDIR)/channels.o:	../../chandefs.h
$(OBJDIR)/channels.o:	../../channels.c
$(OBJDIR)/channels.o:	../../channels.h
$(OBJDIR)/channels.o:	../../chanpriv.h
$(OBJDIR)/channels.o:	../../devclnt.h
$(OBJDIR)/channels.o:	../../devices.h
$(OBJDIR)/channels.o:	../../endian.h
$(OBJDIR)/channels.o:	../../logging.h
$(OBJDIR)/channels.o:	../../logtarget.h
$(OBJDIR)/channels.o:	../../msgbuild.h
$(OBJDIR)/channels.o:	../../pid/devconf.h
$(OBJDIR)/channels.o:	../../pid/pid.h
$(OBJDIR)/channels.o:	../../serlock.h
$(OBJDIR)/channels.o:	../../stacks.h
$(OBJDIR)/channels.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLchannels) -o $(OBJDIR)/channels.o ../../channels.c

$(OBJDIR)/prof.o:	../../adp.h
$(OBJDIR)/prof.o:	../../adperr.h
$(OBJDIR)/prof.o:	../../angel.h
$(OBJDIR)/prof.o:	../../arm.h
$(OBJDIR)/prof.o:	../../buffers.h
$(OBJDIR)/prof.o:	../../chandefs.h
$(OBJDIR)/prof.o:	../../channels.h
$(OBJDIR)/prof.o:	../../coproc.h
$(OBJDIR)/prof.o:	../../debug.h
$(OBJDIR)/prof.o:	../../debughwi.h
$(OBJDIR)/prof.o:	../../debugos.h
$(OBJDIR)/prof.o:	../../devices.h
$(OBJDIR)/prof.o:	../../logging.h
$(OBJDIR)/prof.o:	../../logtarget.h
$(OBJDIR)/prof.o:	../../msgbuild.h
$(OBJDIR)/prof.o:	../../pid/devconf.h
$(OBJDIR)/prof.o:	../../pid/pid.h
$(OBJDIR)/prof.o:	../../prof.c
$(OBJDIR)/prof.o:	../../prof.h
$(OBJDIR)/prof.o:	../../stacks.h
$(OBJDIR)/prof.o:	../../support.h
$(OBJDIR)/prof.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLprof) -o $(OBJDIR)/prof.o ../../prof.c

$(OBJDIR)/rx.o:	../../adp.h
$(OBJDIR)/rx.o:	../../adperr.h
$(OBJDIR)/rx.o:	../../angel.h
$(OBJDIR)/rx.o:	../../buffers.h
$(OBJDIR)/rx.o:	../../chandefs.h
$(OBJDIR)/rx.o:	../../channels.h
$(OBJDIR)/rx.o:	../../crc.h
$(OBJDIR)/rx.o:	../../devclnt.h
$(OBJDIR)/rx.o:	../../devdriv.h
$(OBJDIR)/rx.o:	../../devices.h
$(OBJDIR)/rx.o:	../../endian.h
$(OBJDIR)/rx.o:	../../logging.h
$(OBJDIR)/rx.o:	../../logtarget.h
$(OBJDIR)/rx.o:	../../params.h
$(OBJDIR)/rx.o:	../../pid/devconf.h
$(OBJDIR)/rx.o:	../../pid/pid.h
$(OBJDIR)/rx.o:	../../rx.c
$(OBJDIR)/rx.o:	../../rxtx.h
$(OBJDIR)/rx.o:	../../serlock.h
$(OBJDIR)/rx.o:	../../stacks.h
$(OBJDIR)/rx.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLrx) -o $(OBJDIR)/rx.o ../../rx.c

$(OBJDIR)/tx.o:	../../adperr.h
$(OBJDIR)/tx.o:	../../angel.h
$(OBJDIR)/tx.o:	../../buffers.h
$(OBJDIR)/tx.o:	../../chandefs.h
$(OBJDIR)/tx.o:	../../channels.h
$(OBJDIR)/tx.o:	../../crc.h
$(OBJDIR)/tx.o:	../../devclnt.h
$(OBJDIR)/tx.o:	../../devices.h
$(OBJDIR)/tx.o:	../../endian.h
$(OBJDIR)/tx.o:	../../logging.h
$(OBJDIR)/tx.o:	../../logtarget.h
$(OBJDIR)/tx.o:	../../rxtx.h
$(OBJDIR)/tx.o:	../../tx.c
$(OBJDIR)/tx.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLtx) -o $(OBJDIR)/tx.o ../../tx.c

$(OBJDIR)/crc.o:	../../crc.c
$(OBJDIR)/crc.o:	../../crc.h
$(OBJDIR)/crc.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLcrc) -o $(OBJDIR)/crc.o ../../crc.c

$(OBJDIR)/sys.o:	../../adp.h
$(OBJDIR)/sys.o:	../../adperr.h
$(OBJDIR)/sys.o:	../../angel.h
$(OBJDIR)/sys.o:	../../arm.h
$(OBJDIR)/sys.o:	../../buffers.h
$(OBJDIR)/sys.o:	../../chandefs.h
$(OBJDIR)/sys.o:	../../channels.h
$(OBJDIR)/sys.o:	../../debug.h
$(OBJDIR)/sys.o:	../../devices.h
$(OBJDIR)/sys.o:	../../endian.h
$(OBJDIR)/sys.o:	../../logging.h
$(OBJDIR)/sys.o:	../../logtarget.h
$(OBJDIR)/sys.o:	../../msgbuild.h
$(OBJDIR)/sys.o:	../../pid/devconf.h
$(OBJDIR)/sys.o:	../../pid/pid.h
$(OBJDIR)/sys.o:	../../stacks.h
$(OBJDIR)/sys.o:	../../support.h
$(OBJDIR)/sys.o:	../../sys.c
$(OBJDIR)/sys.o:	../../sys.h
$(OBJDIR)/sys.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLsys) -o $(OBJDIR)/sys.o ../../sys.c

$(OBJDIR)/serpkt.o:	../../adp.h
$(OBJDIR)/serpkt.o:	../../angel.h
$(OBJDIR)/serpkt.o:	../../buffers.h
$(OBJDIR)/serpkt.o:	../../chandefs.h
$(OBJDIR)/serpkt.o:	../../devclnt.h
$(OBJDIR)/serpkt.o:	../../devdriv.h
$(OBJDIR)/serpkt.o:	../../devices.h
$(OBJDIR)/serpkt.o:	../../logging.h
$(OBJDIR)/serpkt.o:	../../logtarget.h
$(OBJDIR)/serpkt.o:	../../params.h
$(OBJDIR)/serpkt.o:	../../pid/devconf.h
$(OBJDIR)/serpkt.o:	../../pid/pid.h
$(OBJDIR)/serpkt.o:	../../ringbuff.h
$(OBJDIR)/serpkt.o:	../../rxtx.h
$(OBJDIR)/serpkt.o:	../../serlock.h
$(OBJDIR)/serpkt.o:	../../serpkt.c
$(OBJDIR)/serpkt.o:	../../serring.h
$(OBJDIR)/serpkt.o:	../../stacks.h
$(OBJDIR)/serpkt.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLserpkt) -o $(OBJDIR)/serpkt.o ../../serpkt.c

$(OBJDIR)/logadp.o:	../../adperr.h
$(OBJDIR)/logadp.o:	../../angel.h
$(OBJDIR)/logadp.o:	../../buffers.h
$(OBJDIR)/logadp.o:	../../chandefs.h
$(OBJDIR)/logadp.o:	../../channels.h
$(OBJDIR)/logadp.o:	../../debug.h
$(OBJDIR)/logadp.o:	../../devices.h
$(OBJDIR)/logadp.o:	../../logging.h
$(OBJDIR)/logadp.o:	../../logging/logadp.c
$(OBJDIR)/logadp.o:	../../logging/logadp.h
$(OBJDIR)/logadp.o:	../../logpriv.h
$(OBJDIR)/logadp.o:	../../logtarget.h
$(OBJDIR)/logadp.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLlogadp) -o $(OBJDIR)/logadp.o ../../logging/logadp.c

$(OBJDIR)/logterm.o:	../../logging/logterm.c
$(OBJDIR)/logterm.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLlogterm) -o $(OBJDIR)/logterm.o ../../logging/logterm.c

$(OBJDIR)/log552.o:	../../../clx/toolver.h
$(OBJDIR)/log552.o:	../../adp.h
$(OBJDIR)/log552.o:	../../adperr.h
$(OBJDIR)/log552.o:	../../angel.h
$(OBJDIR)/log552.o:	../../buffers.h
$(OBJDIR)/log552.o:	../../chandefs.h
$(OBJDIR)/log552.o:	../../channels.h
$(OBJDIR)/log552.o:	../../debug.h
$(OBJDIR)/log552.o:	../../devclnt.h
$(OBJDIR)/log552.o:	../../devdriv.h
$(OBJDIR)/log552.o:	../../devices.h
$(OBJDIR)/log552.o:	../../logging.h
$(OBJDIR)/log552.o:	../../logging/log552.c
$(OBJDIR)/log552.o:	../../logging/log552.h
$(OBJDIR)/log552.o:	../../logpriv.h
$(OBJDIR)/log552.o:	../../logtarget.h
$(OBJDIR)/log552.o:	../../params.h
$(OBJDIR)/log552.o:	../../pid/banner.h
$(OBJDIR)/log552.o:	../../pid/devconf.h
$(OBJDIR)/log552.o:	../../pid/pid.h
$(OBJDIR)/log552.o:	../../pid/st16c552.h
$(OBJDIR)/log552.o:	../../ringbuff.h
$(OBJDIR)/log552.o:	../../rxtx.h
$(OBJDIR)/log552.o:	../../serlock.h
$(OBJDIR)/log552.o:	../../serring.h
$(OBJDIR)/log552.o:	../../stacks.h
$(OBJDIR)/log552.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLlog552) -o $(OBJDIR)/log552.o ../../logging/log552.c

$(OBJDIR)/pcmcia.o:	../../angel.h
$(OBJDIR)/pcmcia.o:	../../logging.h
$(OBJDIR)/pcmcia.o:	../../logtarget.h
$(OBJDIR)/pcmcia.o:	../../pid/devconf.h
$(OBJDIR)/pcmcia.o:	../../pid/olicom.h
$(OBJDIR)/pcmcia.o:	../../pid/pcmcia.c
$(OBJDIR)/pcmcia.o:	../../pid/pcmcia.h
$(OBJDIR)/pcmcia.o:	../../pid/pid.h
$(OBJDIR)/pcmcia.o:	../../stacks.h
$(OBJDIR)/pcmcia.o:	../../support.h
$(OBJDIR)/pcmcia.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLpcmcia) -o $(OBJDIR)/pcmcia.o ../../pid/pcmcia.c

$(OBJDIR)/olicom.o:	../../../fusion/incl/adbnrc.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/arp.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/ccdep.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/debug.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/dsu.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/enet.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/flags.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/flip.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/fnsproto.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/fr.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/frctlm.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/icmp.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/intra.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/ip.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/krent.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/m.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/memory.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/modulo.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/nerrno.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/netdev.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/netioc.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/nsel.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/os.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/panic.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/pr.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/psu.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/q.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/select.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/so.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/socket.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/son.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/std.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/tcp.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/timer.h
$(OBJDIR)/olicom.o:	../../../fusion/incl/twq.h
$(OBJDIR)/olicom.o:	../../angel.h
$(OBJDIR)/olicom.o:	../../arm.h
$(OBJDIR)/olicom.o:	../../ipstack/fusion/arm/config.h
$(OBJDIR)/olicom.o:	../../ipstack/ipconfig.h
$(OBJDIR)/olicom.o:	../../logging.h
$(OBJDIR)/olicom.o:	../../logtarget.h
$(OBJDIR)/olicom.o:	../../pid/82595.h
$(OBJDIR)/olicom.o:	../../pid/devconf.h
$(OBJDIR)/olicom.o:	../../pid/olicom.c
$(OBJDIR)/olicom.o:	../../pid/olicom.h
$(OBJDIR)/olicom.o:	../../pid/pcmcia.h
$(OBJDIR)/olicom.o:	../../pid/pid.h
$(OBJDIR)/olicom.o:	../../serlock.h
$(OBJDIR)/olicom.o:	../../stacks.h
$(OBJDIR)/olicom.o:	../../support.h
$(OBJDIR)/olicom.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLolicom) -o $(OBJDIR)/olicom.o ../../pid/olicom.c

$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/adbnrc.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/arp.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/ccdep.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/debug.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/dsu.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/enet.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/flags.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/flip.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/fnsproto.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/fr.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/frctlm.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/icmp.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/intra.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/ip.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/krent.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/m.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/memory.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/modulo.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/nerrno.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/netdev.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/netioc.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/nsel.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/os.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/panic.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/pr.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/psu.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/q.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/select.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/so.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/socket.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/son.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/std.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/tcp.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/timer.h
$(OBJDIR)/little_rel/i82595.o:	../../../fusion/incl/twq.h
$(OBJDIR)/little_rel/i82595.o:	../../angel.h
$(OBJDIR)/little_rel/i82595.o:	../../ipstack/fusion/arm/config.h
$(OBJDIR)/little_rel/i82595.o:	../../ipstack/ipconfig.h
$(OBJDIR)/little_rel/i82595.o:	../../logging.h
$(OBJDIR)/little_rel/i82595.o:	../../logtarget.h
$(OBJDIR)/little_rel/i82595.o:	../../pid/82595.c
$(OBJDIR)/little_rel/i82595.o:	../../pid/82595.h
$(OBJDIR)/little_rel/i82595.o:	../../pid/olicom.h
$(OBJDIR)/little_rel/i82595.o:	../../pid/pcmcia.h
$(OBJDIR)/little_rel/i82595.o:	../../pid/pid.h
$(OBJDIR)/little_rel/i82595.o:	../../support.h
$(OBJDIR)/i82595.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFL82595) -o $(OBJDIR)/i82595.o ../../pid/82595.c

$(OBJDIR)/ipconfig.o:	../../adp.h
$(OBJDIR)/ipconfig.o:	../../angel.h
$(OBJDIR)/ipconfig.o:	../../buffers.h
$(OBJDIR)/ipconfig.o:	../../chandefs.h
$(OBJDIR)/ipconfig.o:	../../devclnt.h
$(OBJDIR)/ipconfig.o:	../../devdriv.h
$(OBJDIR)/ipconfig.o:	../../devices.h
$(OBJDIR)/ipconfig.o:	../../ethernet.h
$(OBJDIR)/ipconfig.o:	../../ipstack/ipconfig.c
$(OBJDIR)/ipconfig.o:	../../ipstack/ipconfig.h
$(OBJDIR)/ipconfig.o:	../../params.h
$(OBJDIR)/ipconfig.o:	../../pid/devconf.h
$(OBJDIR)/ipconfig.o:	../../pid/pid.h
$(OBJDIR)/ipconfig.o:	../../serlock.h
$(OBJDIR)/ipconfig.o:	../../stacks.h
$(OBJDIR)/ipconfig.o:	../../support.h
$(OBJDIR)/ipconfig.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLipconfig) -o $(OBJDIR)/ipconfig.o ../../ipstack/ipconfig.c

$(OBJDIR)/timer.o:	../../angel.h
$(OBJDIR)/timer.o:	../../pid/devconf.h
$(OBJDIR)/timer.o:	../../pid/pid.h
$(OBJDIR)/timer.o:	../../pid/timer.c
$(OBJDIR)/timer.o:	../../prof.h
$(OBJDIR)/timer.o:	../../serlock.h
$(OBJDIR)/timer.o:	../../stacks.h
$(OBJDIR)/timer.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLtimer) -o $(OBJDIR)/timer.o ../../pid/timer.c

$(OBJDIR)/ethernet.o:	../../../fusion/incl/adbnrc.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/arp.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/boot.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/ccdep.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/debug.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/dsu.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/enet.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/flags.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/flip.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/fnsproto.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/fr.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/frctlm.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/icmp.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/in.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/intra.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/ip.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/krent.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/m.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/memory.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/mib.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/modulo.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/nerrno.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/netdev.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/netioc.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/nsel.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/os.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/panic.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/pr.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/psu.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/q.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/select.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/so.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/socket.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/son.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/std.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/tcp.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/timer.h
$(OBJDIR)/ethernet.o:	../../../fusion/incl/twq.h
$(OBJDIR)/ethernet.o:	../../adp.h
$(OBJDIR)/ethernet.o:	../../adperr.h
$(OBJDIR)/ethernet.o:	../../angel.h
$(OBJDIR)/ethernet.o:	../../buffers.h
$(OBJDIR)/ethernet.o:	../../chandefs.h
$(OBJDIR)/ethernet.o:	../../channels.h
$(OBJDIR)/ethernet.o:	../../devclnt.h
$(OBJDIR)/ethernet.o:	../../devdriv.h
$(OBJDIR)/ethernet.o:	../../devices.h
$(OBJDIR)/ethernet.o:	../../endian.h
$(OBJDIR)/ethernet.o:	../../ethernet.h
$(OBJDIR)/ethernet.o:	../../ipstack/fusion/arm/config.h
$(OBJDIR)/ethernet.o:	../../ipstack/ipconfig.h
$(OBJDIR)/ethernet.o:	../../logging.h
$(OBJDIR)/ethernet.o:	../../logtarget.h
$(OBJDIR)/ethernet.o:	../../params.h
$(OBJDIR)/ethernet.o:	../../pid/devconf.h
$(OBJDIR)/ethernet.o:	../../pid/ethernet.c
$(OBJDIR)/ethernet.o:	../../pid/pid.h
$(OBJDIR)/ethernet.o:	../../serlock.h
$(OBJDIR)/ethernet.o:	../../stacks.h
$(OBJDIR)/ethernet.o:	../../support.h
$(OBJDIR)/ethernet.o:	../../pid/devconf.h
	$(CC) $(CFLAGS) $(CFLethernet) -o $(OBJDIR)/ethernet.o ../../pid/ethernet.c

