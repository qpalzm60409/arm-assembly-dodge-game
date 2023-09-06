/*
 * ARM symbolic debugger toolbox: dbg_conf.h
 * Copyright (C) 1992 Advanced Risc Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.25.24.2 $
 * Checkin $Date: 1999/05/19 09:35:16 $
 * Revising $Author: dcleland $
 */

#ifndef Dbg_Conf__h
#define Dbg_Conf__h

#ifdef PICCOLO
#ifndef RDI_VERSION
#define RDI_VERSION 150
#endif
#endif

#ifndef RDI_VERSION
#define RDI_VERSION 100
#endif

/* new debugger toolbox uses a toolconf */
#include "toolconf.h"

/*
 * The debugger uses the following fields:
 *  LLSymsNeedPrefix  (Boolean)
 *  DeviceName        (String, name of device - eg. ARM, Piccolo)
 *  ByteSex           (something starting L or B, may be updated by toolkit)
 *  MemConfigToLoad   (filename)
 *  CPUSpeed          (int)
 *  ConfigToLoad      (filename)
 *
 * Specific targets use other fields, with the suggested names:
 *  FPE               \
 *  MemorySize         }  ISS
 *  Processor         /
 *  SerialPort        \
 *  SerialLineSpeed    |
 *  ParallelPort       |
 *  ParallelLineSpeed   } Dev Card
 *  EthernetTarget     |
 *  RDIType            |
 *  DriverType        /
 *  HeartbeatOn           Debug Monitor
 */

#define Dbg_Cnf_DeviceName (tag_t)"DEVICENAME"
#define Dbg_Cnf_ByteSex (tag_t)"BYTESEX"
#define Dbg_Cnf_MemConfigToLoad (tag_t)"MEMCONFIGTOLOAD"
#define Dbg_Cnf_CPUSpeed (tag_t)"CPUSPEED"
#define Dbg_Cnf_ConfigToLoad (tag_t)"CONFIGTOLOAD"
#define Dbg_Cnf_FPE (tag_t)"FPE"
#define Dbg_Cnf_MemorySize (tag_t)"MEMORYSIZE"
#define Dbg_Cnf_Processor (tag_t)"PROCESSOR"

#define Dbg_Cnf_SerialPort (tag_t)"SERIALPORT"
#define Dbg_Cnf_SerialLineSpeed (tag_t)"SERIALLINESPEED"
#define Dbg_Cnf_ParallelPort (tag_t)"PARALLELPORT"
#define Dbg_Cnf_ParallelLineSpeed (tag_t)"PARALLELLINESPEED"
#define Dbg_Cnf_EthernetTarget (tag_t)"ETHERNETTARGET"
#define Dbg_Cnf_RDIType (tag_t)"RDITYPE"
#define Dbg_Cnf_Driver (tag_t)"DRIVER"

#define Dbg_Cnf_Heartbeat (tag_t)"HEARTBEAT"
#define Dbg_Cnf_LLSymsNeedPrefix (tag_t)"LLSYMSNEEDPREFIX"
#define Dbg_Cnf_Reset (tag_t)"RESET"
#define Dbg_Cnf_TargetName (tag_t)"TARGETNAME"
#define Dbg_Cnf_ApcsFpEnabled (tag_t)"APCSFPENABLED"

/* MICE config */
/* Values moved to rdi_conf.h */
/* #define Dbg_Cnf_MICEServer    (tag_t)"MICESERVER"
#define Dbg_Cnf_MICEDbgConn   (tag_t)"MICEDBGCONN"
#define Dbg_Cnf_MICEDriver0   (tag_t)"MICEDRIVER0"
#define Dbg_Cnf_MICETapPos0   (tag_t)"MICETAPPOS0"
#define Dbg_Cnf_MICEDriver1   (tag_t)"MICEDRIVER1"
#define Dbg_Cnf_MICETapPos1   (tag_t)"MICETAPPOS1"  */

#if RDI_VERSION == 100
typedef struct Dbg_ConfigBlock {
    int bytesex;
    int fpe;               /* Target should initialise FPE */
    long memorysize;
    unsigned long cpu_speed;/* Cpu speed (HZ) */
    int serialport;        /*) remote connection parameters */
    int seriallinespeed;   /*) (serial connection) */
    int parallelport;      /*) ditto */
    int parallellinespeed; /*) (parallel connection) */
#ifndef PicAlpha
    char *ethernettarget;  /* name of remote ethernet target */
#endif
    int processor;         /* processor the armulator is to emulate (eg ARM60) */
    int rditype;           /* armulator / remote processor */
#ifndef PicAlpha
    int heartbeat_on;      /* angel heartbeat */
#endif
    int drivertype;        /* parallel / serial / etc */
    char const *configtoload;
    char const *memconfigtoload;
    int flags;
    char const *target_name;
} Dbg_ConfigBlock;


#endif

/* Move def. for these outside #ifndef PICCOLO */
#define Dbg_ConfigFlag_Reset 1
#define Dbg_ConfigFlag_LLSymsNeedPrefix 2


typedef struct Dbg_HostosInterface Dbg_HostosInterface;
/* This structure allows access by the (host-independent) C-library support
   module of armulator or pisd (armos.c) to host-dependent functions for
   which there is no host-independent interface.  Its contents are unknown
   to the debugger toolbox.
   The assumption is that, in a windowed system, fputc(stderr) for example
   may not achieve the desired effect of the character appearing in some
   window.
 */

#endif

