/* models.h - declare all the user-definable extensions to ARMulator.
 * Copyright (c) 1996-1998 ARM Limited. All Rights Reserved.
 *
 * RCS $Revision: 1.40 $
 * Checkin $Date: 1998/07/20 09:35:01 $
 * Revising $Author: askillma $
 */

#ifndef EDA
/* SDT version below - shipped by ARM Development Systems BU */

/* These are the defaults - must be declared first! */
MEMORY(ARMul_Flat)
COPROCESSOR(ARMul_NoCopro)

/* ARMulator memory interfaces */
#ifndef NO_MMULATOR
MEMORY(ARMul_MMUlator)
#endif
MEMORY(ARMul_TracerMem)         /* Instruction/Memory tracer */
#if defined(unix) && !defined(_WINDU_SOURCE)
MEMORY(ARMul_PIE)               /* PIE card model */
MEMORY(ARMul_EBSA110)           /* Model of the EBSA-110 validation card */
#endif
MEMORY(ARMul_MapFile)           /* armsd.map file support */
#ifndef NO_SARMSD
MEMORY(StrongMMU)               /* StrongARM MMU model */
#endif
MEMORY(ARMul_BytelaneVeneer)    /* Bytelane ASIC */
MEMORY(ARMul_TrickBox)          /* Trickbox (validation) ASIC */
MEMORY(ARMul_WatchPoints)       /* Memory model that does watchpoints */
MEMORY(ARMul_Fast)              /* High-speed memory model (Fixed 2MB) */
#ifndef NO_ARM9
MEMORY(ARM940CacheMPU)
MEMORY(ARM920CacheMMU)
#endif

/* Co-Processor bus models */
COPROCESSOR(ARMul_CPBus)
/* Co-Processor models */
COPROCESSOR(ARMul_DummyMMU)

/* Basic models (extensions) */
/*
 * Basic models are initialised in the order in which they appear in
 * this file.
 */
#ifdef NEW_OS_INTERFACE
/* The "WinGlass" module has to be initialised before other models. Until
 * the O/S becomes a model, this has to be done explicitly from inside the
 * ARMulator, so this model does not appear in this header.
 */
#ifdef COMPILING_ON_WINDOWS
MODEL(ARMul_WinGlass)
#endif
#endif /* NEW_OS_INTERFACE */
MODEL(ARMul_Profiler)           /* Instruction profiler */
MODEL(ARMul_TracerModel)        /* Instruction tracer */
MODEL(ARMul_Pagetable)          /* Provides page-tables */
MODEL(ARMul_ValidateCP)         /* Vaidation CoProcessor (not OS model too) */
MODEL(ARMul_WatchPointsInit)    /* Initialise the Watchpoints model */

/* Operating System/Monitors */
#ifdef NEW_OS_INTERFACE         /* in the new world, OS's are plain models */
#ifndef NO_ANGEL
MODEL(ARMul_Angel)              /* An operating-system model */
#endif
MODEL(ARMul_Demon)              /* The old Demon debug-monitor */
MODEL(ARMul_ValidateOS)           /* Used for Validation only */
/* MODEL(PIDprint)               * this is only a new-style model */
#else
#ifndef NO_ANGEL
OSMODEL(ARMul_Angel)
#endif
OSMODEL(ARMul_Demon)
OSMODEL(ARMul_ValidateOS)
#endif /* NEW_OS_INTERFACE */

#if defined(PICCOLO) || defined(PicAlpha)
MEMORY(Piccolo)
MEMORY(Piccolo2)
#endif

#if defined(PERIPHERAL_LIB)
MEMORY(ARMul_APB)
#endif

#else
/* EDA version: CoVerification Simulators shipped by ARM EDA group */

#ifndef NO_ARM9
MEMORY(ARM940CacheMPU)
MEMORY(ARM920CacheMMU)
#endif

/* Co-Processor bus models */
/* TBD: why is this required even if no copros attached???? */
COPROCESSOR(ARMul_CPBus)

MODEL(ARMul_TracerModel)        /* Instruction tracer */
MEMORY(ARMul_TracerMem)         /* Instruction/Memory tracer */

/* Seamless CVE memory, Needs pipelined verison of ARMulator */
/* TBD: eliminate references to 3rd party products */
#ifdef CVE
MEMORY(CVEMemory)
#endif

#endif
