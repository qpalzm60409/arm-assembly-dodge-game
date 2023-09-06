/* multirdi.h - processor lists */
/* Copyright (C) 1996-1998 ARM Limited. All rights reserved. */

/* The real "processor list" is stored in "multirdi" rather in armrdi. */

/*
 * RCS $Revision: 1.14 $
 * Checkin $Date: 1998/05/22 16:02:26 $
 * Revising $Author: mwilliam $
 */

#ifndef multirdi_h
#define multirdi_h


#include "armdefs.h"            /* to define RDI_VERSION */

#include "rdi.h"
#include "rdi_conf.h"

#include "toolconf.h"

#if RDI_VERSION == 150
#  define RDI150(X) X
#  define RDI100(X)
#  define RDI150_(X) X,
#  define RDI100_(X)
#  define _RDI150(X) , X
#  define _RDI100(X)
#else
#  define RDI150(X)
#  define RDI100(X) X
#  define RDI150_(X)
#  define RDI100_(X) X,
#  define _RDI150(X)
#  define _RDI100(X) , X
#endif
#define RDI_HANDLE RDI150(RDI_ModuleHandle rdi_handle) RDI100(void)
#define RDI_HANDLE_ RDI150_(RDI_ModuleHandle rdi_handle)
#define RDI_AGENT_HANDLE RDI150(RDI_AgentHandle rdi_handle) RDI100(void)
#define RDI_AGENT_HANDLE_ RDI150_(RDI_AgentHandle rdi_handle)
#define IGNORE_HANDLE RDI150(IGNORE(rdi_handle))

/*
 * Need to export this.  No RDI call to get the default processor,
 * yet the GUI DLL needs to know this.
 */
extern int ARMul_DefaultProcessor(
     RDI150(RDI_AgentHandle rdi_handle) RDI100(void));



#endif

/* EOF multirdi.h */
