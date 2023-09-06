/*
 * ARM RDI : rdi_stat.h
 * Copyright (C) 1998 Advanced Risc Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1998/03/26 13:59:45 $
 * Revising $Author: mwilliam $
 */

#ifndef rdi_stat_h
#define rdi_stat_h

#include "armtypes.h"           /* for ARMword */

typedef struct {
  char name[32];                /* length byte, followed by string */
} RDI_CycleDesc;

typedef struct {
    ARMword handle;
    ARMword start, limit;  /* start & limit of this region */
    unsigned char width;   /* memory width 0,1,2 => 8,16,32 bit */
    unsigned char access;  /* Bit 0 => read access */
                           /* Bit 1 => write access */
                           /* Bit 2 => latched 32 bit memory */
    unsigned char d1, d2;  /* ensure padding */
             /* Access times for R/W N/S cycles */
    unsigned long Nread_ns, Nwrite_ns, Sread_ns, Swrite_ns;
} RDI_MemDescr;

typedef struct {
    ARMword Nreads,     /* Counts for R/W N/S cycles */
            Nwrites,
            Sreads,
            Swrites;
    ARMword ns,         /* Time (nsec, sec) */
            s;
} RDI_MemAccessStats;

#endif /* rdi_stat_h */
