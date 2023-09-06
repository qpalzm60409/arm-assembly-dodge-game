# Makefile for the Application Note 40 configuration of the FPASC.
#
# Copyright (C) Advanced RISC Machines Limited, 1997. All rights reserved.
#
# RCS $Revision: 1.1.2.7 $
# Checkin $Date: 1998/09/10 11:16:04 $
# Revising $Author: tdouglas $

COREDIR   = ../coresrc
VENDIR    = ../vensrc/appn40
COREFILES = arith armdefs defaults fpadefs fpaundef fpeundef interf \
            ldst macros main nans optcheck regnames rounding toplevel \
            transhw wsdefs
VENFILES  = ctxtws end globalws header options start

fpasc.o: $(VENFILES:%=$(VENDIR)/%.s) $(COREFILES:%=$(COREDIR)/%.s)
	armasm -arch 3 -apcs 3/32bit -i $(COREDIR) -i $(VENDIR) -o $@ $(VENDIR)/header.s
