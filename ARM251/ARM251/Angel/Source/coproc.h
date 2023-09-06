/*
 * Angel Coprocessor descriptions.
 *
 * Derived from: RDI co-processor interface : rdi_cp.h
 * Copyright (C) 1998 Advanced Risc Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.1.2.1 $
 * Checkin $Date: 1998/10/02 18:50:53 $
 * Revising $Author: rivimey $
 */

#ifndef coproc_h
#define coproc_h

#define CP_Access_Readable   1
#define CP_Access_Writable   2 
#define CP_Access_CPDT       4 /* Register accesses via CP data transfer */
#define CP_Access_Scan_Chain 8 /* OR'ing this bit will cause the CP Register
                                 * in question to be access via the JTAG 
                                 * interface to a scan chain, only if the 
                                 * Debug agent supports this.
                                 */

typedef struct {
    unsigned short rmin, rmax;
    /* a single description can be used for a range of registers with
       the same properties *accessed via CPDT instructions*
       Because CP15 registers are specified by the CRn part of the
       MRC/MCR Instructions => they must be encoded separately and
       cannot be specified in a range.
     */
    unsigned int nbytes;   /* size of register */
    unsigned char access;  /* see above (Access_xxx) */
    union {
        /* The CPRT Part must appear first since it has more elements
         * and the C init. rules say that in a constant union initialisation
         * the value of the constant expression is assigned to the 
         * first member of the union
         */
       struct { /* CPRT instructions have much more latitude.  The bits fixed
                   by the ARM are  24..31 (condition mask & opcode)
                                   20 (direction)
                                   8..15 (cpnum, arm register)
                                   4 (CPRT not CPDO)
                   leaving 14 bits free to the coprocessor (fortunately
                   falling within two bytes).
                   These are encoded as follows:

                   Bit Pattern for MRC/MCR Instructions.
                   
                   31         23      16           7        0
                   +-----------------------------------------+
                   |Cond|1110|OC1|L|CRn | Rd |1111|OC2|1|CRm |
                   +-----------------------------------------+
                                  b1                   b0

                   b0 = Bits[0:7], b1 = Bits[16:23]
                   => b0 = CRm | ( OpCode2 << 5 )
                      b1 = CRn | ( OpCode1 << 5 )
                 */
                unsigned char read_b0, read_b1,
                              write_b0, write_b1;
              } cprt;
        struct { /* CPDT instructions do not allow the 
                    coprocessor much freedom:
                    only bit 22 ('N') and 12-15 ('CRd') are free for the
                    coprocessor to use as it sees fit.
                 */
                unsigned char nbit;
                unsigned char rdbits;
              } cpdt;
    } accessinst;
} CP_CoProRegDesc;

typedef struct CP_CoProDesc CP_CoProDesc;

struct CP_CoProDesc {
    int entries;
    CP_CoProRegDesc regdesc[1/* really nentries */];
};

#define CP_CoProDesc_Size(n) \
  (sizeof(CP_CoProDesc) + ((n)-1)*sizeof(CP_CoProRegDesc))

#endif
