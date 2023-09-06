/* scanf.h                                                      */
/* Copyright (C) Advanced Risc Machines Ltd., 1991             */

/*
 * RCS $Revision: 1.3 $
 * Checkin $Date: 1998/05/19 10:11:47 $
 * Revising $Author: wdijkstr $
 */

#define NOSTORE      0x1
#define LONGLONG     0x2
#define LONG         0x4
#define SHORT        0x8
#define FIELDGIVEN  0x10
#define LONGDOUBLE  0x20
#define ALLOWSIGN   0x40    /* + or - acceptable to rd_int  */
#define DOTSEEN     0x80    /* internal to rd_real */
#define NEGEXP     0x100    /* internal to rd_real */
#define NUMOK      0x200    /* ditto + rd_int */
#define NUMNEG     0x400    /* ditto + rd_int */

extern unsigned long int _strtoul(const char *nsptr, char **endptr, int base);

extern int _chval(unsigned int ch, unsigned int radix);

