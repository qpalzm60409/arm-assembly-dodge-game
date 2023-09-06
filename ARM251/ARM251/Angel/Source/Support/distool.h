/*
 * distool.h - disassembler helper functions
 * Copyright (c) 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.6 $
 * Checkin $Date: 1998/06/05 10:22:15 $
 * Revising $Author: swallis $
 */

#include <stdio.h>
#include <string.h>

#include "host.h"

extern char const *hexprefix;

/* ---------------- Output Functions --------------------- */

#define Dis_OutC(h,o)   (*o = h, o+1)
#define Dis_UngetC(h,o) ((o[-1] == h) ? o-1 : o)
#define Dis_OutI(n,o)   Dis_OutF(o,"#%ld",n)
#define Dis_OutX(n,o)   Dis_OutF(o,"%s%lx",hexprefix,(unsigned long)n)

/* ---------------- Bit twiddlers ------------------------ */

/* The casts to uint32 in bit() and bits() are redundant, but required by
   some buggy compilers.
 */

#define bp(n) (((uint32)1L<<(n)))
#define bit(n) (((uint32)(instr & bp(n)))>>(n))
#define bits(m,n) (((uint32)(instr & (bp(n)-bp(m)+bp(n))))>>(m))
#define ror(n,b) (((uint32)(n)>>(b))|((n)<<(32-(b)))) /* n right rotated b bits */


void Dis_AddNote(char *notes, char const *fmt, ...);
void Dis_CheckValue(uint32 field, uint32 val, char const *s, char *notes);
void Dis_CheckZero(uint32 field, char const *s, char *notes);

char *Dis_OutS(char const *s, char *o);
char *Dis_OutF(char *o, char const *fmt, ...);

char *Dis_ArmOpCode(uint32 instr, const char *op, int ch, char *o);
extern char *Dis_ArmOpCodeF(char *o, unsigned32 instr, const char *fmt, ...);
  /* fmt is of the form "<string>$<printf-format>", where $ is
   * where to insert the condition code
   */
char *Dis_ArmOutAddress(uint32 instr, uint32 address, int32 offset, int w, char *o);
char *Dis_ArmReg(uint32 rno, int ch, char *o);
char *Dis_cond(unsigned32 instr, char *o);
char *Dis_spacetocol9(char *start, char *o);

/* End of distool.h */
