/*
 * disass.c - single instruction disassembler
 * Copyright (C) ARM Limited, 1991-1998. All rights reserved.
 */

/*
 * RCS $Revision: 1.52.2.5 $
 * Checkin $Date: 1998/10/08 10:17:53 $
 * Revising $Author: sclarke $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "host.h"
#include "disass.h"
#include "distool.h"

/* ---------------- Local variables ---------------------- */

static char const **regnames, **fregnames;
static dis_cb *cb_proc;
static void *cb_arg;
char const *hexprefix = "0x";

void Dis_AddNote(char *notes, char const *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  if (notes[0] != 0) {
    notes += strlen(notes);
    strcpy(notes, ", ");
    notes += 2;
  }
  _vsprintf(notes, fmt, ap);
  va_end(ap);
}

void Dis_CheckValue(unsigned32 field, unsigned32 val, char const *s, char *notes) {
  if (field != val) Dis_AddNote(notes, "%s = %s%lx", s, hexprefix, field);
}

void Dis_CheckZero(unsigned32 field, char const *s, char *notes) {
  Dis_CheckValue(field, 0, s, notes);
}

char *Dis_OutF(char *o, char const *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  _vsprintf(o, fmt, ap);
  va_end(ap);
  return o + strlen(o);
}

char *Dis_OutS(const char *s, char *o) {
 /* All strings are very short: no point to any cleverness which a library
    strcpy might give us.
  */
    char ch;
    for (; (ch = *s++) != 0;)
      o = Dis_OutC(ch, o);
    return o;
}

char *Dis_cond(unsigned32 instr, char *o) {
    const char *ccnames = "EQ\0\0NE\0\0CS\0\0CC\0\0MI\0\0PL\0\0VS\0\0VC\0\0\
HI\0\0LS\0\0GE\0\0LT\0\0GT\0\0LE\0\0\0\0\0\0NV";
    return Dis_OutS(ccnames+4*(int)bits(28,31), o);
}

char *Dis_ArmReg(unsigned32 rno, int ch, char *o) {
    if (regnames == NULL)
       if (rno == 15)
          o = Dis_OutS("pc", o);
       else
          o = Dis_OutF(o, "r%ld", rno);
    else
       o = Dis_OutS(regnames[rno], o);
    if (ch != 0)
       o = Dis_OutC(ch, o);
    return o;
}

static char *FP_Reg(unsigned32 rno, int ch, char *o) {
    if (fregnames == NULL)
       o = Dis_OutF(o, "f%ld", rno);
    else
       o = Dis_OutS(fregnames[rno], o);
    if (ch != 0)
       o = Dis_OutC(ch, o);
    return o;
}

static char *shiftedreg(unsigned32 instr, char *o) {
    const char *shiftname = "LSL\0LSR\0ASR\0ROR" + 4*(int)bits(5,6);
    o = Dis_ArmReg(bits(0,3), 0, o); /* offset is a (shifted) reg */
    if (bit(4)) { /* register shift */
       o = Dis_OutF(o, ",%s ", shiftname);
       o = Dis_ArmReg(bits(8,11), 0, o);
    }
    else if (bits(5,11)!=0) { /* immediate shift */
       if (bits(5,11)==3)
          o = Dis_OutS(",RRX", o);
       else {
          o = Dis_OutF(o, ",%s ", shiftname);
          if (bits(5,11)==1 || bits(5,11)==2)
             o = Dis_OutI(32L, o);
          else
             o = Dis_OutI(bits(7,11), o);
       }
    }
    return o;
}

static char *outn(unsigned32 n, unsigned32 pos, char *o)
{
    if (!pos) o = Dis_OutC('-', o);
    if (n < 10)
        o = Dis_OutF(o, "%ld", n);
    else
        o = Dis_OutX(n, o);
    return o;
}

static char *outh(unsigned32 n, unsigned32 pos, char *o) {
    o = Dis_OutC('#', o);
    o = outn(n, pos, o);
    return o;
}

char *Dis_spacetocol9(char *start, char *o) {
  /* Ensure at least one space output */
    int k = 9 - (o - start);
    do
        o = Dis_OutC(' ', o);
    while (--k > 0);
    return o;
}

static char *t_opcode(const char *op, char *o)
{
    char *start = o;
    o = Dis_OutS(op, o);
    return Dis_spacetocol9(start, o);
}

char *Dis_ArmOpCode(unsigned32 instr, const char *op, int ch, char *o) {
    char *start = o;
    o = Dis_OutS(op, o);
    o = Dis_cond(instr, o);
    if (ch != 0) o = Dis_OutC(ch, o);
    return Dis_spacetocol9(start, o);
}

char *Dis_ArmOpCodeF(char *o, unsigned32 instr, const char *fmt, ...) {
  /* fmt is of the form "<string>$<printf-format>", where $ is
   * where to insert the condition code
   */
    va_list ap;
    char *start = o;
    const char *dollar;

    va_start(ap, fmt);

    dollar = strchr(fmt, '$');
    if (dollar == NULL) return Dis_ArmOpCode(instr, fmt, 0, o);
    memcpy(o, fmt, dollar-fmt); /* copy part of string */
    o = Dis_cond(instr, o + (dollar-fmt));

    _vsprintf(o, dollar+1, ap);
    va_end(ap);

    return Dis_spacetocol9(start, o+strlen(o));
}

static char *ArmOutAddress(
    unsigned32 instr, unsigned32 address, int32 offset, int w, char *o, bool fromCPDT) {
    char *oldo = o;
    if (bits(16,19)==15 && bit(24) && !bit(25)) { /* pc based, pre, imm */
       if (!bit(23)) offset = -offset;
       address = address+offset+8;
       if (cb_proc != NULL)
          o = cb_proc((bit(20) ? D_LOADPCREL : D_STOREPCREL),
                      offset, address, w, cb_arg, o);
       if (oldo == o) o = Dis_OutX(address, o);
    } else {
       if (bit(24) && !bit(25) && cb_proc != NULL) /* pre, imm */
          o = cb_proc((bit(20) ? D_LOAD : D_STORE),
                      (bit(23) ? offset : -offset),
                      bits(16,19), w, cb_arg, o);
       if (oldo == o) {
          o = Dis_OutC('[', o);
          o = Dis_ArmReg(bits(16,19), (bit(24) ? 0 : ']'), o);
          o = Dis_OutC(',', o);
          if (!bit(25)) { /* offset is an immediate */
              /* there's a special case for CPDT's of the 'options' field */
              if (fromCPDT && !bit(24) && !bit(21) && bit(23)) {
                o = Dis_OutC('{', o);
                o = outn(offset/4, bit(23), o);
                o = Dis_OutC('}', o);
              } else {
                  o = outh(offset, bit(23), o);
              }
          } else {
             if (!bit(23)) o = Dis_OutC('-', o);
             o = shiftedreg(instr, o);
          }
          if (bit(24)) {
             o = Dis_OutC(']', o);
             if (bit(21)) o = Dis_OutC('!', o);
          }
       }
    }
    return o;
}

char *Dis_ArmOutAddress(unsigned32 instr, unsigned32 address, int32 offset, int w, char *o)
{
  return ArmOutAddress(instr, address, offset, w, o, FALSE);
}

static char *outregset(unsigned32 instr, char *o) {
    bool started = NO,
         string = NO;
    unsigned32 i,
               first = 0,
               last = 0;
    o = Dis_OutC('{', o);
    for (i=0; i<16; i++) {
       if (bit(i)) {
          if (!started) {
             o = Dis_ArmReg(i, 0, o);
             started=YES;
             first=last=i;
          }
          else if (i==last+1) {
             string=YES;
             last=i;
          }
          else {
             if (i>last+1 && string) {
                o = Dis_OutC((first == last-1) ? ',' : '-', o);
                o = Dis_ArmReg(last, 0, o);
                string=NO;
             }
             o = Dis_OutC(',', o); o = Dis_ArmReg(i, 0, o);
             first=last=i;
          }
       }
    }
    if (string) {
       o = Dis_OutC((first == last-1) ? ',' : '-', o);
       o = Dis_ArmReg(last, 0, o);
    }
    o = Dis_OutC('}', o);
    return o;
}

static char *generic_cpdo(int cpno, unsigned32 instr, char *o) {
    o = Dis_ArmOpCode(instr, "CDP", 0, o);
    o = Dis_OutF(o, "p%d,", cpno);
    o = Dis_OutX(bits(20,23), o); o = Dis_OutC(',', o);
    o = Dis_OutF(o, "c%ld,", bits(12,15));   /* CRd */
    o = Dis_OutF(o, "c%ld,", bits(16,19));   /* CRn */
    o = Dis_OutF(o, "c%ld,", bits(0,3));     /* CRm */
    o = Dis_OutF(o, "%ld", bits(5,7));
    return o;
}

static char *generic_cprt(int cpno, unsigned32 instr, char *o) {
    o = Dis_ArmOpCode(instr, (bit(20) ? "MRC" : "MCR"), 0, o);
    o = Dis_OutF(o, "p%d,", cpno);
    o = Dis_OutX(bits(21,23), o); o = Dis_OutC(',', o);
    o = Dis_ArmReg(bits(12,15), ',', o);
    o = Dis_OutF(o, "c%ld,", bits(16,19));   /* CRn */
    o = Dis_OutF(o, "c%ld,", bits(0,3));     /* CRm */
    o = Dis_OutF(o, "%ld", bits(5,7));
    return o;
}

static char *generic_cpdt(int cpno, unsigned32 instr, unsigned32 address, char *o, char *notes) {
    o = Dis_ArmOpCode(instr, (bit(20) ? "LDC" : "STC"), (bit(22) ? 'L' : 0), o);
    o = Dis_OutF(o, "p%d,", cpno);
    o = Dis_OutF(o, "c%ld,", bits(12,15));
    if (!bit(24) && !bit(21)) {
      if (bit(23)) {
      } else
        Dis_AddNote(notes, "Postindexed, Down, no WB");
    }
    return ArmOutAddress(instr, address, 4*bits(0,7), 0, o, TRUE);
}

static char *HandleGenericCoPro(
int cpno, Disass_CPOpType type, unsigned32 instr, unsigned32 address, char *o, char *notes) {
    IGNORE(notes);
    switch (type) {
    case CP_DP: return generic_cpdo(cpno, instr, o);
    case CP_RT: return generic_cprt(cpno, instr, o);
    case CP_DT: return generic_cpdt(cpno, instr, address, o, notes);
    }
    return NULL;  /* can't be reached*/
}

static char fp_dt_widthname(unsigned32 instr) {
    return "SDEP"[bit(15) + 2*bit(22)];
}

static char fp_widthname(unsigned32 instr) {
    return "SDEP"[bit(7) + 2*bit(19)];
}

static const char *fp_rounding(unsigned32 instr) {
    return "\0\0P\0M\0Z" + 2*bits(5,6);
}

static char *fp_mfield(unsigned32 instr, char *o) {
   unsigned32 r = bits(0,2);
   if (bit(3)) {
      if (r < 6)
         o = Dis_OutI(r, o);
      else
         o = Dis_OutS((r == 6 ? "#0.5" : "#10"), o);
   }
   else
      o = FP_Reg(r, 0, o);
   return o;
}

static char *fp_cpdo(unsigned32 instr, char *o, char *notes) {
    const char *opset;
    char *start = o;
    if (bit(15))  /* unary */
        opset = "\
MVF\0MNF\0ABS\0RND\0SQT\0LOG\0LGN\0EXP\0\
SIN\0COS\0TAN\0ASN\0ACS\0ATN\0URD\0NRM";
     else
        opset = "\
ADF\0MUF\0SUF\0RSF\0DVF\0RDF\0POW\0RPW\0\
RMF\0FML\0FDV\0FRD\0POL\0XX1\0XX2\0XX3";

    o = Dis_OutS(opset + 4*bits(20,23), o);
    o = Dis_cond(instr, o);
    o = Dis_OutC(fp_widthname(instr), o);
    o = Dis_OutS(fp_rounding(instr), o);
    o = Dis_spacetocol9(start, o);
    o = FP_Reg(bits(12,14), ',', o);  /* Fd */
    if (!bit(15))
        o = FP_Reg(bits(16,18), ',', o);  /* Fn */
    else if (bits(16,18) != 0)
        Dis_CheckZero(bits(16,18), "Fn", notes);
    return fp_mfield(instr, o);
}

static char *fp_cprt(unsigned32 instr, char *o, char *notes) {
    int op = (int)bits(20,23);
    IGNORE(notes);
    if (bits(12,15)==15) {
       /* ARM register = pc */ 
       if ((op & 9) != 9) return NULL;  /* Invalid: decode as generic */
       else
         op = (op>>1)-4;
       o = Dis_ArmOpCode(instr, "CMF\0\0CNF\0\0CMFE\0CNFE" + 5*op, 0, o);
       o = FP_Reg(bits(16,18), ',', o);
       return fp_mfield(instr, o);

    } else {
       char *start = o;
       if (op > 5)
         return NULL;
       o = Dis_OutS("FLT\0FIX\0WFS\0RFS\0WFC\0RFC" + 4*op, o);
       o = Dis_cond(instr, o);
       if (op == 0)
           o = Dis_OutC(fp_widthname(instr), o);    /* Field with only for FLT */
       if (op == 0 || op == 1)
           o = Dis_OutS(fp_rounding(instr), o);     /* Rounding mode for FLT/FIX */
       o = Dis_spacetocol9(start, o);
       if (op == 0) /* FLT */ {
         o = FP_Reg(bits(16,18), ',', o);
       }
       o = Dis_ArmReg(bits(12,15), 0, o);
       if (op == 1) /* FIX */ {
         o = Dis_OutC(',', o);
         o = fp_mfield(instr, o);
       }
    }
    return o;
}

static char *fp_cpdt(unsigned32 instr, unsigned32 address, char *o, char *notes) {
    if (!bit(24) && !bit(21)) Dis_AddNote(notes, "Postindexed, no WB");
    o = Dis_ArmOpCode(instr, (bit(20) ? "LDF" : "STF"), fp_dt_widthname(instr), o);
    o = FP_Reg(bits(12,14), ',', o);
    return Dis_ArmOutAddress(instr, address, 4*bits(0,7), 0, o);
}

static char *fm_cpdt(unsigned32 instr, unsigned32 address, char *o, char *notes) {
    if (!bit(24) && !bit(21)) Dis_AddNote(notes, "Postindexed, no WB");
    o = Dis_ArmOpCode(instr, (bit(20) ? "LFM" : "SFM"), 0, o);
    o = FP_Reg(bits(12,14), ',', o);
    {  int count = (int)(bit(15) + 2*bit(22));
       o = Dis_OutF(o, "%d,", count==0 ? 4: count);
    }
    o = Dis_ArmOutAddress(instr, address, 4*bits(0,7), 0, o);
    return o;
}

static char *HandleFP(
  int cpno, Disass_CPOpType type, unsigned32 instr, unsigned32 address, char *o, char *notes) {
    switch (type) {
    case CP_DP:
        if (cpno == 1)
            return fp_cpdo(instr, o, notes);
        break;
    case CP_RT:
        if (cpno == 1)
            return fp_cprt(instr, o, notes);
        break;
    case CP_DT:
        if (cpno == 1)
            return fp_cpdt(instr, address, o, notes);
        else if (cpno == 2)
            return fm_cpdt(instr, address, o, notes);
        break;
    }
    return NULL;
}

void disass_setregnames(char const **rn, char const **fn) {
    regnames = rn; fregnames = fn;
}

void disass_sethexprefix(char const *p) {
    hexprefix = p;
}

unsigned32 disass_16(unsigned32 instr, unsigned32 instr2, unsigned32 address, char *o, void *cba, dis_cb *cbp)
{
    int32 Rd, Rm, Rn, Ro;
    int32 imm5, imm8, imm11;
    int32 L, B;

    Rd = bits(0, 2);
    Rm = bits(3, 5);
    Rn = bits(6, 8);
    Ro = bits(8, 10);
#define imm3 Rn
    imm11 = bits(0, 10);
    imm8 = bits(0, 7);
    imm5 = bits(6, 10);
    L = bit(11);
#define SP (L)
#define H (L)
    B = bit(10);
#define S B
#define I B

    switch (bits(11, 15)) {
        case 3:
            if (bit(9) == 0 && I && imm3 == 0) {
              o = t_opcode("MOV", o);
              o = Dis_ArmReg(Rd, ',', o);
              o = Dis_ArmReg(Rm, 0, o);
              break;
            }
            o = t_opcode(bit(9) ? "SUB" : "ADD", o);
            o = Dis_ArmReg(Rd, ',', o);
            if (Rd != Rm) o = Dis_ArmReg(Rm, ',', o);
            o = I ? outh(imm3, 1, o) : Dis_ArmReg(Rn, 0 , o);
            break;
        case 10:
        case 11:
            o = t_opcode("STR\0*STRH\0STRB\0LDSB\0LDR\0*LDRH\0LDRB\0LDSH" + bits(9, 11) * 5, o);
            o = Dis_ArmReg(Rd, ',', o);
            o = Dis_OutC('[', o);
            o = Dis_ArmReg(Rm, ',', o);
            o = Dis_ArmReg(Rn, ']', o);
            break;
        case 12:
        case 13:
            imm5 <<= 1;
        case 16:
        case 17:
            imm5 <<= 1;
        case 14:
        case 15:
            o = t_opcode("STR\0*LDR\0*STRB\0LDRB\0STRH\0LDRH\0" + (bits(11, 15) - 12) * 5, o);
            o = Dis_ArmReg(Rd, ',', o);
            o = Dis_OutC('[', o);
            o = Dis_ArmReg(Rm, ',', o);
            o = outh(imm5, 1, o);
            o = Dis_OutC(']', o);
            break;
        case 0:
        case 1:
        case 2:
            o = t_opcode("LSL\0LSR\0ASR" + bits(11, 12) * 4, o);
            o = Dis_ArmReg(Rd, ',', o);
            if (Rd != Rm) o = Dis_ArmReg(Rm, ',', o);
            if (bits(11, 12) > 0 && imm5 == 0) imm5 = 32; /* LSR/ASR 0 -> LSR/ASR 32 */
            o = Dis_OutF(o, "#%ld", imm5);
            break;
        case 8: {
            int32 op;

            op = bits(6, 10);
            if (op < 16) {
            o = t_opcode("AND\0EOR\0LSL\0LSR\0ASR\0ADC\0SBC\0ROR\0TST\0NEG\0CMP\0CMN\0ORR\0MUL\0BIC\0MVN" + op * 4, o);
            } else {
              if (op & 2) Rd += 8;
              if (op & 1) Rm += 8;
              switch(op) {
                case 17:
                case 18:
                case 19:
                  o = t_opcode("ADD", o);
                  break;
                case 21:
                case 22:
                case 23:
                  o = t_opcode("CMP", o);
                  break;
                case 25:
                case 26:
                case 27:
                  o = t_opcode("MOV", o);
                  break;
                case 16:
                case 20:
                case 24:
                  o = t_opcode("Undefined", o);
                  o = Dis_OutC(0, o);
                  return 2;
                case 28:
                case 29:
                  o = t_opcode("BX", o);
                  o = Dis_ArmReg(Rm, 0, o);
                  o = Dis_OutC(0, o);
                  return 2;
                case 30:
                case 31:
                  o = t_opcode("BLX", o);
                  o = Dis_ArmReg(Rm, 0, o);
                  o = Dis_OutC(0, o);
                  return 2;
              }
            }
            o = Dis_ArmReg(Rd, ',', o);
            o = Dis_ArmReg(Rm, 0, o);
            break;
        }
        case 4:
        case 5:
        case 6:
        case 7:
            o = t_opcode("MOV\0CMP\0ADD\0SUB" + bits(11, 12) * 4, o);
            o = Dis_ArmReg(Ro, ',', o);
            o = outh(imm8, 1, o);
            break;
        case 18:
        case 19: {
            char *oldo;

            o = t_opcode("STR\0LDR" + L * 4, o);
            o = Dis_ArmReg(Ro, ',', o);
            imm8 <<= 2;
            oldo = o;
            if (cbp) o = cbp(L ? D_LOAD : D_STORE, imm8, 13, 4, cba, o);
            if (o == oldo) {
                o = Dis_OutC('[', o);
                o = Dis_ArmReg(13, ',', o);
                o = outh(imm8, 1, o);
                o = Dis_OutC(']', o);
            }
            break;
        }
        case 28: {
            char *oldo;

            o = t_opcode("B", o);
            imm11 = (imm11 << 21) / (1 << 20);
            oldo = o;
            if (cbp) o = cbp(D_BORBL, imm11, address + imm11 + 4, 0, cba, o);
            if (o == oldo) o = Dis_OutX(address + imm11 + 4, o);
            break;
        }
        case 22:
        case 23:
            if (!bit(10)) {
                if (bits(8, 11) != 0) {
                    o = t_opcode("Undefined", o);
                } else {
                    imm8 = (imm8 & 0x7f) << 2;
                    o = t_opcode(bit(7) ? "SUB" : "ADD", o);
                    o = Dis_ArmReg(13, ',', o);
                    o = outh(imm8, 1, o);
                }
            } else {
                if (bit(9)) {
                    o = t_opcode("Undefined", o);
                } else {
                    instr &= 0x1ff;
                    if (instr & 0x100) {
                        instr &= ~0x100;
                        if (L)
                            instr |= 0x8000;
                        else
                            instr |= 0x4000;
                    }
                    o = t_opcode("PUSH\0POP" + L * 5, o);
                    o = outregset(instr, o);
                }
            }
            break;
        case 9: {
            char *oldo;

            o = t_opcode("LDR", o);
            o = Dis_ArmReg(Ro, ',', o);
            imm8 <<= 2;
            oldo = o;
            address = (address + 4) & ~3;
            if (cbp) o = cbp(D_LOADPCREL, imm8, address + imm8, 0, cba, o);
            if (o == oldo) o = Dis_OutX(address + imm8, o);
            break;
        }
        case 24:
        case 25:
            instr &= 0xff;
            o = t_opcode("STMIA\0LDMIA" + L * 6, o);
            o = Dis_ArmReg(Ro, '!', o);
            o = Dis_OutC(',', o);
            o = outregset(instr, o);
            break;
        case 20:
        case 21: {
            char *oldo;

            o = t_opcode("ADR\0ADD" + SP * 4, o);
            o = Dis_ArmReg(Ro, ',', o);
            imm8 <<= 2;
            if (!SP) {
                oldo = o;
                address = (address + 4) & ~3;
                if (cbp) o = cbp(D_ADDPCREL, imm8, address + imm8, 0, cba, o);
                if (o == oldo) o = Dis_OutX(address + imm8, o);
            } else {
                o = Dis_ArmReg(13, ',', o);
                o = outh(imm8, 1, o);
            }
            break;
        }
        case 26:
        case 27: {
            char *oldo;
            int32 op;

            op = bits(8, 11);
            if (op == 15) {
                o = t_opcode("SWI", o);
                oldo = o;
                if (cbp) cbp(D_SWI, imm8, 0, 0, cba, o);
                if (o == oldo) o = Dis_OutX(imm8, o);
            } else {
                o = Dis_ArmOpCode(op << 28, "B", 0, o);
                imm8 = (imm8 << 24) / (1 << 23);
                oldo = o;
                if (cbp) o = cbp(D_BORBL, imm8, address + imm8 + 4, 0, cba, o);
                if (o == oldo) o = Dis_OutX(address + imm8 + 4, o);
            }
            break;
        }
        case 30: {
            int32 offset;
            char *oldo;

            if ((instr2 & 0xe800) == 0xe800) {
                if (instr2 & 0x1000)
                   o = t_opcode("BL", o);
                else
                   o = t_opcode("BLX", o);
                offset = instr2 & 0x7ff;
                offset = (((imm11 << 11) | offset) << 10) / (1 << 9);
                oldo = o;
                if (cbp) o = cbp(D_BORBL, offset, address + offset + 4, 0, cba, o);
                if (o == oldo) o = Dis_OutX(address + offset + 4, o);
                o = Dis_OutC(0, o);
                return 4;
            } else {
                o = t_opcode("???", o);
            }
            break;
        }
        case 29:
        case 31:
            o = t_opcode("???", o);
            break;
        default:
            o = t_opcode("Undefined", o);
            break;
    }
    o = Dis_OutC(0, o);
    return 2;
}

#undef imm3
#undef SP
#undef S
#undef H
#undef H1
#undef H2

unsigned32 disass(unsigned32 instr, unsigned32 address, char *o, void *cba, dis_cb *cbp)
{
    disass_32or26(instr, address, o, cba, cbp, 0);
    return 4;
}

typedef struct CoProRec CoProRec;

struct CoProRec {
    CoProRec *next;
    Disass_CoProProc *f;
};

#define NextCoProRec NULL

#ifdef PICCOLO
#include "picdis.h"
static CoProRec PicCoProRec = { NextCoProRec, piccolo_DisassCP };
#undef NextCoProRec
#define NextCoProRec (&PicCoProRec)
#endif

static CoProRec FPCoProRec = { NextCoProRec, HandleFP };
static CoProRec *copros = &FPCoProRec;

static char *HandleCoPro(
  Disass_CPOpType type, unsigned32 instr, unsigned32 address, char *o, char *notes) {
    CoProRec *p = copros;
    int cpno = bits(8, 11);
    for (; p != NULL; p = p->next) {
        char *res = p->f(cpno, type, instr, address, o, notes);
        if (res != NULL) return res;
    }
    return HandleGenericCoPro(cpno, type, instr, address, o, notes);
}

#ifndef NO_MALLOC
void disass_addcopro(Disass_CoProProc *f) {
    CoProRec *p = copros;
    for (; p != NULL; p = p->next)
        if (p->f == f)
            return;
    p = (CoProRec *)malloc(sizeof(CoProRec));
    p->next = copros; p->f = f;
    copros = p;
}
#endif

#ifndef NO_MALLOC
void disass_deletecopro(Disass_CoProProc *f) {
    CoProRec **pp = &copros, *p;
    for (; (p = *pp) != NULL; pp = &p->next)
        if (p->f == f) {
            *pp = p->next;
            free(p);
            return;
        }
}
#endif

static char *DPOp2(unsigned32 instr, unsigned32 address, char *o) {
    if (bit(25)) {            /* rhs is immediate */
      int op = (int)bits(21,24);
      int shift = 2*(int)bits(8,11);
      int32 operand = ror(bits(0,7), shift);
      char *oldo = o;
      if ((op == 4 || op == 2) && /* ADD or SUB  */
          bits(16,19) == 15 &&        /* rn = pc */
          cb_proc != NULL)
        o = cb_proc((op == 4 ? D_ADDPCREL : D_SUBPCREL),
                    operand, address+8, 0, cb_arg, o);

      if (o == oldo) {
          o = outh(operand, 1, o);
          if ((op == 4 || op == 2) && /* ADD or SUB */
              bits(16,19) == 15) {       /* rn = pc */
              o = Dis_OutS(" ; ", o);
              o = outh(address+8 + ((op == 4) ? operand : -operand),
                       1, o);
          }
      }
    }
    else {                   /* rhs is a register */
      o = shiftedreg(instr, o);
    }
    return o;
}

unsigned32 disass_32or26(unsigned32 instr, unsigned32 address, char *o, void *cba, dis_cb *cbp, int mode_32bit) {
    char notes[64];
    notes[0] = 0;
    cb_proc = cbp; cb_arg = cba;
    switch (bits(24,27)) {

    case 0:
       if (bits(4,7) == 9) {
         /* Arithmetic extension space */
         if (bits(22,23) == 0) {
           o = Dis_ArmOpCode(instr, (bit(21) ? "MLA" : "MUL"), (bit(20) ? 'S' : 0), o);
           o = Dis_ArmReg(bits(16,19), ',', o);
           o = Dis_ArmReg(bits(0,3), ',', o);
           o = Dis_ArmReg(bits(8,11), 0, o);
           if (bit(21)) {
             o = Dis_OutC(',', o);
             o = Dis_ArmReg(bits(12,15), 0, o);
           }
           break;
         } else if (bit(23)==1) {
           /* Long Multiply */
           o=Dis_ArmOpCode(instr, bit(21) ? (bit(22) ? "SMLAL" : "UMLAL")
                                       : (bit(22) ? "SMULL" : "UMULL"),
                        bit(20) ? 'S' : 0, o);
           o = Dis_ArmReg(bits(12,15), ',', o);
           o = Dis_ArmReg(bits(16,19), ',', o);
           o = Dis_ArmReg(bits(0,3), ',', o);
           o = Dis_ArmReg(bits(8,11), 0, o);
           break;
         } else {
           Dis_AddNote(notes, "Bad arithmetic extension op = %ld", bits(20,23));
           /* And fall through to disassemble as data-processing */
         }
       }
       /* Drop through */
    case 1: case 2: case 3:
       if (bits(26,27) == 0 && bits(23,24) == 2 && !bit(20) &&
           (bit(25) || !bit(7) || !bit(4))) {
         /* Control extension space */
         if (!bit(25) && bits(4,7) == 1 && bits(8,19) == 0xfff &&
             bits(21,22) == 1) {
           o = Dis_ArmOpCode(instr, "BX", 0, o);
           o = Dis_ArmReg(bits(0, 3), 0, o);
           break;
         }

         if (!bit(25) && bits(4,7) == 3 && bits(8,19) == 0xfff &&
             bits(21,22) == 1) {
           o = Dis_ArmOpCode(instr, "BLX", 0, o);
           o = Dis_ArmReg(bits(0, 3), 0, o);
           break;
         }

         if ((bits(20,27) == 0x16) && bits(4,7) == 1){
           o = Dis_ArmOpCode(instr, "CLZ", 0, o);
           o = Dis_ArmReg(bits(12, 15), ',', o);
           o = Dis_ArmReg(bits(0, 3), 0, o);
           break;
         }

         if (bits(4,11) == 0x05 && bits(23,27) == 2 && !bit(20)) {
           /* El Segundo saturated arithmetic */
           char *satname = NULL;
           bool swap_sources = FALSE;
           uint32 src1, src2;

           switch(bits(21,22)) {
           case 0: satname = "QADD";                       break;
           case 1: satname = "QSUB";                       break;
           case 2: satname = "QDADD"; swap_sources = TRUE; break;
           case 3: satname = "QDSUB"; swap_sources = TRUE; break;
           }
           o = Dis_ArmOpCode(instr, satname, 0, o);
           o = Dis_ArmReg(bits(12, 15), ',', o);
           src1 = bits(16, 19);
           src2 = bits(0, 3);
           if (swap_sources)
           {
               uint32 tmpsrc = src1; src1 = src2; src2 = tmpsrc;
           }
           o = Dis_ArmReg(src1, ',', o);
           o = Dis_ArmReg(src2, 0, o);
           break;
         }

         if (bits(23, 27) == 2 && !bit(20) && !bit(4) && bit(7)) {
           /* El Segundo narrow multiply instructions */
           char *mname = NULL;
           enum { fourth_reg_none,
                  fourth_reg_is_rn,
                  fourth_reg_is_rdlo
                } fourth_reg = fourth_reg_none;
           char mulname[10];
           bool select_rm = TRUE;

           switch(bits(21,22)) {
           case 0:
             mname = "SMLA";
             fourth_reg = fourth_reg_is_rn;
             break;
           case 1:
             select_rm = FALSE;
             if (bit(5)) {
               mname = "SMULW";
             } else {
               mname = "SMLAW";
               fourth_reg = fourth_reg_is_rn;
             }
             break;
           case 2:
             mname = "SMLAL";
             fourth_reg = fourth_reg_is_rdlo;
             break;
           case 3:
             mname = "SMUL";
             break;
           }
           strcpy(mulname, mname);
           if (select_rm)
             strcat(mulname, bit(5) ? "T" : "B");
           strcat(mulname, bit(6) ? "T" : "B");
           o = Dis_ArmOpCode(instr, mulname, 0, o);
           if (fourth_reg == fourth_reg_is_rdlo)
             o = Dis_ArmReg(bits(12, 15), ',', o);
           o = Dis_ArmReg(bits(16, 19), ',', o);
           o = Dis_ArmReg(bits(0, 3), ',', o);
           o = Dis_ArmReg(bits(8, 11), ","[fourth_reg != fourth_reg_is_rn], o);
           if (fourth_reg == fourth_reg_is_rn)
             o = Dis_ArmReg(bits(12, 15), 0, o);
           break;
         }

         if (!bit(25) && !bit(21)) {
           o = Dis_ArmOpCode(instr, "MRS", 0, o);
           o = Dis_ArmReg(bits(12,15), ',', o);
           o = Dis_OutS(!bit(22) ? "CPSR" : "SPSR", o);
           Dis_CheckZero(bits(0,11), "0-11", notes);
           Dis_CheckValue(bits(16,19), 15, "Rn", notes);
           break;
         }
         if (bit(21)) {
           const char *rname = !bit(22) ? "CPSR" : "SPSR";
           int rn = (int)bits(16, 19);
           char flags[8];
           char *f = flags;
           *f++ = '_';
           if (rn & 1) *f++ = 'c';
           if (rn & 2) *f++ = 'x';
           if (rn & 4) *f++ = 's';
           if (rn & 8) *f++ = 'f';
           if (rn == 0) Dis_AddNote(notes, "field-mask = 0");
           *f++ = ',';
           *f = '\0';
           o = Dis_ArmOpCode(instr, "MSR", 0, o);
           o = Dis_OutS(rname, o);
           o = Dis_OutS(flags, o);
           Dis_CheckValue(bits(12,15), 15, "Rd", notes);
           if (!bit(25))
             Dis_CheckZero(bits(4,11), "4-11", notes);
           o = DPOp2(instr, address, o);
           break;
         }
         Dis_AddNote(notes, "Bad control extension op");
       }
       if (bits(25,27) == 0 && bit(7) && bit(4)
           && (bit(24) || bits(5,6))) {
         /* Load-store extension space */
         if (bits(23,24) == 2 && bits(20,21) == 0 && bits(4,11) == 9) {
           /* Swap */
           o = Dis_ArmOpCode(instr, "SWP", (bit(22) ? 'B' : 0), o);
           o = Dis_ArmReg(bits(12,15), ',', o);
           o = Dis_ArmReg(bits(0,3), ',', o);
           o = Dis_OutC('[', o); o = Dis_ArmReg(bits(16,19), ']', o);
           break;
         } 
         if (bit(20) ? bits(5,6) != 0 : bits(5,6) == 1) {
           char *start = o;
           o = Dis_OutS(bit(20) ? "LDR" : "STR", o);
           o = Dis_cond(instr, o);
           if (bit(6)) {
             o = Dis_OutC('S', o);
             o = Dis_OutC(bit(5) ? 'H' : 'B', o);
           } else
             o = Dis_OutC('H', o);
           o = Dis_spacetocol9(start, o);
           o = Dis_ArmReg(bits(12,15), ',', o);
           o = Dis_OutC('[', o);
           o = Dis_ArmReg(bits(16,19), 0, o);
           if (bit(24)) o = Dis_OutC(',', o); else o = Dis_OutC(']', o), o = Dis_OutC(',', o);
           if (bit(22)) {
             o = outh(bits(0, 3) + (bits(8,11)<<4), bit(23), o);
           } else {
             if (!bit(23)) o = Dis_OutC('-',o);
             o = Dis_ArmReg(bits(0,3),0,o);
           }
           if (bit(24)) {
             o = Dis_OutC(']', o);
             if (bit(21)) o = Dis_OutC('!', o);
           } else if (bit(21))
             Dis_AddNote(notes, "Post-indexed, W=1");

           break;
         }
         Dis_AddNote(notes, "Bad load/store extension op");
       }
       if (instr == 0xe1a00000L) {
          o = Dis_ArmOpCode(instr, "NOP", 0, o);
          break;
       }
       { /* data processing */
          int op = (int)bits(21,24);
          const char *opnames = "AND\0EOR\0SUB\0RSB\0ADD\0ADC\0SBC\0RSC\0\
TST\0TEQ\0CMP\0CMN\0ORR\0MOV\0BIC\0MVN";
          unsigned32 rd = bits(12,15);
          int ch = (!bit(20)) ? 0 :
             (op>=8 && op<12) ? (rd==15 ? 'P' : 0) :
                                 'S';
          o = Dis_ArmOpCode(instr, opnames+4*op, ch, o);
          if (op >= 8 && op < 12) {    /* TST TEQ CMP CMN */
            if (rd != 15)
              Dis_CheckZero(rd, "Rd", notes); 
          } else {
             /* print the dest reg */
             o = Dis_ArmReg(rd, ',', o);
          }
          if (op == 13 || op == 15) {         /* MOV MVN */
            Dis_CheckZero(bits(16,19), "Rn", notes);
          } else {
            o = Dis_ArmReg(bits(16,19), ',', o);
          }
          o = DPOp2(instr, address, o);
       }
       break;

    case 0xa: case 0xb:
       if (bits(25,31) == 0x7d) {
          char *start = o ;
          o = Dis_OutS("BLX", o);
          o = Dis_spacetocol9(start, o);
          }
       else
          o = Dis_ArmOpCode(instr, (bit(24) ? "BL" : "B"), 0, o);
       {  int32 offset = (((int32)bits(0,23))<<8)>>6; /* sign extend and * 4 */
          char *oldo = o;
          if (bits(24,31) == 0xfb)
             offset |= bit(24) >> 23 ;
          address += offset + 8;
          if (!mode_32bit) address &= 0x3ffffffL;
          if (cb_proc != NULL)
             o = cb_proc(D_BORBL, offset, address, 0, cb_arg, o);
          if (o == oldo) o = Dis_OutX(address, o);
       }
       break;

    case 6: case 7:
       /* Cope with the case where register shift register is specified
        * as this is an undefined instruction rather than an LDR or STR
        */
         if (bit(4)) {
           o=Dis_OutS("Undefined Instruction",o);
           break;
         }
       /* Drop through to alwasy LDR / STR case */
    case 4: case 5:
       {  char *start = o;
          o = Dis_OutS(bit(20) ? "LDR" : "STR", o);
          o = Dis_cond(instr, o);
          if (bit(22)) o = Dis_OutC('B', o);
          if (!bit(24) && bit(21))  /* post, writeback */
             o = Dis_OutC('T', o);
          o = Dis_spacetocol9(start, o);
          o = Dis_ArmReg(bits(12,15), ',', o);
          o = Dis_ArmOutAddress(instr, address, bits(0,11), (bit(22) ? 1 : 4), o);
          break;
       }

    case 8: case 9:
       {  char *start = o;
          o = Dis_OutS(bit(20) ? "LDM" : "STM", o);
          o = Dis_cond(instr, o);
          o = Dis_OutS("DA\0\0IA\0\0DB\0\0IB" + 4*(int)bits(23,24), o);
          o = Dis_spacetocol9(start, o);
          o = Dis_ArmReg(bits(16,19), 0, o);
          if (bit(21)) o = Dis_OutC('!', o);
          o = Dis_OutC(',', o);
          o = outregset(instr, o);
          if (bit(22)) o = Dis_OutC('^', o);
          break;
       }

    case 0xf:
       o = Dis_ArmOpCode(instr, "SWI", 0, o);
       {  char *oldo = o;
          int32 swino = bits(0,23);
          if (cb_proc != NULL)
             o = cb_proc(D_SWI, swino, 0, 0, cb_arg, o);
          if (o == oldo) o = Dis_OutX(swino, o);
       }
       break;

    case 0xe:
       o = HandleCoPro(bit(4)==0 ? CP_DP : CP_RT, instr, address, o, notes);
       break;

    case 0xc: case 0xd:
       o = HandleCoPro(CP_DT, instr, address, o, notes);
       break;

    default:
       o = Dis_OutS("EQUD    ", o);
       o = Dis_OutX(instr, o);
    }
    if (notes[0] != 0) o = Dis_OutF(o, " ; ? %s", notes);
    o = Dis_OutC('\0', o);
    return 4;
}
