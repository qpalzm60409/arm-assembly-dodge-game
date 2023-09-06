/*
 * ARM RDI 1.0 : rdi_conf.h
 * Copyright (C) 1998 Advanced Risc Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.4.8.4 $
 * Checkin $Date: 1999/05/14 10:29:17 $
 * Revising $Author: dadshead $
 */

#ifndef rdi_conf_h
#define rdi_conf_h

#ifdef PICCOLO
#ifndef RDI_VERSION
#define RDI_VERSION 150
#endif
#endif

#ifndef RDI_VERSION
#define RDI_VERSION 150
#endif

/*
 * Define the "Toolconf" tags regardless.
 * These tags start Dbg_Cnf_, for historical reasons
 */

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

/* 
 * IMPORTANT NOTE
 * ==============
 *
 * The tags that follow are suggested names for the toolconf entries
 * for debug targets.  They are not 'required names' under RDI 1.5
 *
 * For every debug target which is released, a list of toolconf entries
 * which the target is sensitive to MUST be provided in the target
 * documentation
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

#define Dbg_ConfigFlag_Reset 1
#define Dbg_ConfigFlag_LLSymsNeedPrefix 2

#define Dbg_Cnf_MICEServer    (tag_t)"Multi-ICE_Server_Location"
#define Dbg_Cnf_MICEDriver0   (tag_t)"Multi-ICE_Driver0_Name"
#define Dbg_Cnf_MICETapPos0   (tag_t)"Multi-ICE_Tap0_Position"

#if 1                           /* defined in rdi_hif.h */
typedef struct RDI_HostosInterface RDI_HostosInterface;
/* This structure allows access by the (host-independent) C-library support
   module of armulator or pisd (armos.c) to host-dependent functions for
   which there is no host-independent interface.  Its contents are unknown
   to the debugger toolbox.
   The assumption is that, in a windowed system, fputc(stderr) for example
   may not achieve the desired effect of the character appearing in some
   window.
 */
#endif

#endif /* rdi_conf_h */

#if RDI_VERSION == 100 || defined(rdi_DefineConfigBlock)
/* Use a different guard for the configblock definition, so this header
 * can be included with rdi_DefineConfigBlock defined, even if something
 * else has already included it.
 * Note: rdi_DefineConfigBlock is for "internal RDI" use
 */
#ifndef rdi_conf_configblock
#define rdi_conf_configblock

typedef struct RDI_ConfigBlock {
    int bytesex;
    int fpe;               /* Target should initialise FPE */
    long memorysize;
    unsigned long cpu_speed;/* Cpu speed (HZ) */
    int serialport;        /*) remote connection parameters */
    int seriallinespeed;   /*) (serial connection) */
    int parallelport;      /*) ditto */
    int parallellinespeed; /*) (parallel connection) */
    char *ethernettarget;  /* name of remote ethernet target */
    int processor;         /* processor the armulator is to emulate (eg ARM60) */
    int rditype;           /* armulator / remote processor */
    int heartbeat_on;      /* angel heartbeat */
    int drivertype;        /* parallel / serial / etc */
    char const *configtoload;
    char const *memconfigtoload;
    int flags;
    char const *target_name;
} RDI_ConfigBlock;


#endif /* rdi_conf_configblock */
#endif /* RDI_VERSION == 100 || ... */

/* EOF rdi_conf.h */
