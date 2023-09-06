/*
 * ARM RDI : rdi_hif.h
 * Copyright (C) 1998 Advanced RISC Machines Ltd. All rights reserved.
 * Description of the RDI_HostosInterface structure.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1998/03/26 13:59:44 $
 * Revising $Author: mwilliam $
 */

#ifndef rdi_hif_h
#define rdi_hif_h

#if defined(__STDC__) || defined(ANSI)
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

typedef struct RDI_Hif_DbgArgStr RDI_Hif_DbgArg;
typedef void RDI_Hif_DbgPrint(
    RDI_Hif_DbgArg *arg, const char *format, va_list ap);
typedef void RDI_Hif_DbgPause(RDI_Hif_DbgArg *arg);

typedef struct RDI_Hif_HostosArgStr RDI_Hif_HostosArg;
typedef void RDI_Hif_WriteC(RDI_Hif_HostosArg *arg, int c);
typedef int RDI_Hif_ReadC(RDI_Hif_HostosArg *arg);
typedef int RDI_Hif_Write(RDI_Hif_HostosArg *arg, char const *buffer, int len);
typedef char *RDI_Hif_GetS(RDI_Hif_HostosArg *arg, char *buffer, int len);

typedef struct RDI_Hif_ResetArgStr RDI_Hif_ResetArg;
typedef void RDI_Hif_ResetProc(RDI_Hif_ResetArg *arg);

struct RDI_HostosInterface {
    RDI_Hif_DbgPrint *dbgprint;
    RDI_Hif_DbgPause *dbgpause;
    RDI_Hif_DbgArg *dbgarg;

    RDI_Hif_WriteC *writec;
    RDI_Hif_ReadC *readc;
    RDI_Hif_Write *write;
    RDI_Hif_GetS *gets;
    RDI_Hif_HostosArg *hostosarg;

    RDI_Hif_ResetProc *reset;
    RDI_Hif_ResetArg *resetarg;
};

#endif
