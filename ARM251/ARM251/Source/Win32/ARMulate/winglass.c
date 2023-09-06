/* winglass.c - i/f onto "armsd_hourglass" function
 * Copyright (c) 1996 Advanced RISC Machines Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.14 $
 * Checkin $Date: 1998/04/07 13:46:48 $
 * Revising $Author: mwilliam $
 */

#include "armdefs.h"
#include "armcnf.h"
#include "rdi_hif.h"

#ifdef COMPILING_ON_WINDOWS
/*
 * armsd_hourglass is a function that must be regularly called under
 * AWD to update the Windows frontend.
 */
extern void armsd_hourglass(void);

/*
 * Intercept all HostIf calls, calling the armsd_hourglass() function
 * whenever any output is sent
 */

static void wing_dbgprint(
        RDI_Hif_DbgArg *arg, const char *format, va_list ap)
{
  RDI_HostosInterface *hif = (RDI_HostosInterface *)arg;
  hif->dbgprint(hif->dbgarg, format, ap);
  armsd_hourglass();
}

static void wing_dbgpause(RDI_Hif_DbgArg *arg)
{
  RDI_HostosInterface *hif = (RDI_HostosInterface *)arg;
  hif->dbgpause(hif->dbgarg);
  armsd_hourglass();
}

static void wing_hwritec(RDI_Hif_HostosArg *arg, int c)
{
  RDI_HostosInterface *hif = (RDI_HostosInterface *)arg;
  hif->writec(hif->hostosarg, c);
  if (c == '\n') armsd_hourglass();
}

static int wing_hreadc(RDI_Hif_HostosArg *arg)
{
  RDI_HostosInterface *hif = (RDI_HostosInterface *)arg;
  armsd_hourglass();
  return hif->readc(hif->hostosarg);
}

static int wing_hwrite(
    RDI_Hif_HostosArg *arg, char const *buffer, int n)
{
  int rv;
  RDI_HostosInterface *hif = (RDI_HostosInterface *)arg;
  rv = hif->write(hif->hostosarg, buffer, n);
  armsd_hourglass();
  return rv;
}

static char *wing_hgets(RDI_Hif_HostosArg *arg, char *buffer, int n)
{
  RDI_HostosInterface *hif = (RDI_HostosInterface *)arg;
  armsd_hourglass();
  return hif->gets(hif->hostosarg, buffer, n);
}

static void wing_hourglass(void *arg, ARMword a1, ARMword a2)
{
  IGNORE(arg); IGNORE(a1); IGNORE(a2);
  armsd_hourglass();
}

#endif

static ARMul_Error WinglassInit(ARMul_State *state, toolconf config)
{
#ifdef COMPILING_ON_WINDOWS
  unsigned long rate;
  RDI_HostosInterface *hif;
  const RDI_HostosInterface *dbg_hif;

#ifndef HOURGLASS_RATE
#  define HOURGLASS_RATE 8192   /* default rate */
#endif
  rate=ToolConf_DLookupUInt(config, ARMulCnf_Rate,HOURGLASS_RATE);

  if (ToolConf_DLookupBool(config, ARMulCnf_Verbose, FALSE)) {
    ARMul_ConsolePrint(state,"Winglass Rate:           %ld\n",rate);
  }

  ARMul_HourglassSetRate(
     state, ARMul_InstallHourglass(state,wing_hourglass,NULL), rate);

  hif = (RDI_HostosInterface *)malloc(sizeof(*hif));
  dbg_hif = ARMul_SetHostIf(state, hif);

  if (hif == NULL) return ARMul_RaiseError(state, ARMulErr_OutOfMemory);

  hif->dbgprint = (dbg_hif->dbgprint ? wing_dbgprint : NULL);
  hif->dbgpause = (dbg_hif->dbgpause ? wing_dbgpause : NULL);
  hif->dbgarg = (RDI_Hif_DbgArg *)dbg_hif;

  hif->writec = (dbg_hif->writec ? wing_hwritec : NULL);
  hif->readc = (dbg_hif->readc ? wing_hreadc : NULL);
  hif->write = (dbg_hif->write ? wing_hwrite : NULL);
  hif->gets = (dbg_hif->gets ? wing_hgets : NULL);
  hif->hostosarg = (RDI_Hif_HostosArg *)dbg_hif;

  hif->reset = dbg_hif->reset;
  hif->resetarg = dbg_hif->resetarg;

  ARMul_InstallExitHandler(state, free, hif);

#else
  IGNORE(state); IGNORE(config);
#endif

  return ARMulErr_NoError;
}


const ARMul_ModelStub ARMul_WinGlass = {
  WinglassInit,
  (tag_t)"WindowsHourglass"
  };

  
 
