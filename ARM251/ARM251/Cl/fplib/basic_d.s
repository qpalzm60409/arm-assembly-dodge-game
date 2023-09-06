; basic_d.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.15 $
; Checkin $Date: 1998/03/19 15:04:08 $
; Revising $Author: wdijkstr $

; Basic floating point functions

        GET     fpe.s

        CodeArea |FPL$$Basic|


;==============================================================================
; Compare
;
; This isn't as simple as it could be. The problem is that NaNs may cause an
; exception and always compare as FALSE if not signalling. Infinities need to
; be treated as normal numbers, although they look like NaNs.
; Furthermore +0 = -0 needs a special check.
;
; General comparison instruction flow: (this is less than)
;
;                              OP1 < 0 OR OP2 < 0
;                                       |
;               +--------Y--------------+------------N-------------+
;               |                                                  |
;       (OP1 OR OP2) NaN?                                   (OP1 OR OP2) NaN?
;               |                                                  |
;      +----N---+---Y------+                         +-----Y-------+----N-----+
;      |                   |                         |                        |
; RET OP1 < OP2      OP1 or OP2 inf/NaN?     OP1 or OP2 inf/NaN?       RET OP1 > OP2
;                            |                       |                     AND NOT 
;                     +---N--+---Y--+         +---Y--+--N----+        (OP1 = 0 AND OP2 = 0)
;                     |             |         |              |            
;              RET OP1 < OP2   (OP1 NaN?) OR (OP2 NaN?)  RET OP1 > OP2 
;                     |                   |                  |
;                     |             +--N--+--Y--> exception  |
;                     |             |                        |
;                     |     OP1 < 0 OR OP2 < 0?              |
;                     |             |                        |
;                     +-----N-------+------------Y-----------+
;
; The first layer selects between the case where both operands are positive or
; when at least one is negative. The second layer uses a quick test on the
; operands orred together to determine whether they look like a NaN. This check is
; weak: it will get about 4% or 9% 'false hits' for doubles and floats, where 
; none of the operands is a NaN. In general false hits occur for very large numbers,
; or for both numbers around 2.0 (one larger, one smaller).  
; If the operands are not categorized a NaNs, a normal unsigned compare does the
; actual work. It returns immediately if the highwords of the operands are different.
; Note that the negative case uses a compare with the operands swapped,
; as the order is reversed for negative numbers. The negative case also checks for
; -0 == 0 as a special case. In the NaN code, a more precise check is done, which
; filters out NaNs and infinities, and the normal compare follows otherwise.
; The exception handler raises a Invalid Operation exception if one of the operands
; is a NaN (ignoring the signal bit).
; There are thus 3 different checks on NaNs, with increasing accuracy: 
; 1. one of the operands looks like a NaN (but might not be one). 
; 2. one of the operands is infinite or NaN. 
; 3. one of the operands is a NaN.
;
; The compare routine can either be used as a boolean returning function (dgt, 
; dge, dlt, dle) or as a flags returning function (returning < as LO, <= as LS,
; > as HI, >= as HS). 
;
; The routine is optimised for the both operands positive which not look like
; NaNs case. It is also assumed the chance that the highwords of the operands are 
; equal is less than 50%. Timing:
; Flags: 7/9 (pos), 11/13 (false NaN), 10/12 (neg), 13/15 (false NaN) SA1.1 cycles.
; EQ/NE/HI/HS/LO/LS: 10 / 14 / 13 / 16
;==============================================================================

        MACRO
        CmpReturn $cc
        [   "$cc" = "FLAGS"
            ReturnToLR_flg
        |
            MOV     a1, #0
            MOV$cc  a1, #1
            ReturnToLR
        ]
        MEND

        MACRO
        CmpReturnNE $cc
        [   "$cc" = "FLAGS"
            ReturnToLR_flg NE
        ]
        MEND


        MACRO
$lab    DoubleCompare $cc, $NaN_lab

        ASSERT "$cc"="FLAGS":LOR:"$cc"="LO":LOR:"$cc"="LS":LOR:"$cc"="HS":LOR:"$cc"="HI":LOR:"$cc"="EQ":LOR:"$cc"="NE"
 
$lab    EnterWithLR_16
        ORRS    tmp, dOP1h, dOP2h
        BMI     $lab._negative              ; one of the operands negative? (MI)
        CMN     tmp, #0x00100000            ; check whether operands might be infinite/NaN
        BMI     $lab._check_NaN_pos
        CMP     dOP1h, dOP2h
        CmpReturnNE $cc
        CMPEQ   dOP1l, dOP2l
        CmpReturn $cc

$lab._check_NaN_pos                         ; opnd1/2 might be inf/NaNs - do more accurate check
        CMN     dOP1h, #0x00100000          ; overhead 4 cycles for false hit
        CMNPL   dOP2h, #0x00100000
        BMI     $lab._Inf_or_NaN
$lab._cmp_pos
        CMP     dOP1h, dOP2h
        CmpReturnNE $cc
        CMPEQ   dOP1l, dOP2l
        CmpReturn $cc

$lab._negative
        CMN     tmp, #0x00100000  
        BPL     $lab._check_NaN_neg         ; check whether operands might be infinite/NaN
        ORRS    tmp, dOP1l, dOP1h, LSL #1   ; check for -0 == 0
        ORREQS  tmp, dOP2l, dOP2h, LSL #1
        CMPNE   dOP2h, dOP1h
        CmpReturnNE $cc
        CMPEQ   dOP2l, dOP1l
        CmpReturn $cc

$lab._check_NaN_neg                         ; opnd1/2 might be inf/NaNs - do more accurate check
        MOV     tmp, #0x00200000            ; overhead 3 cycles for false hit    
        CMN     tmp, dOP1h, LSL #1
        CMNCC   tmp, dOP2h, LSL #1
        BCS     $lab._Inf_or_NaN
$lab._cmp_neg                               ; -0 == 0 test omitted (cannot give a false hit)
        CMP     dOP2h, dOP1h
        CmpReturnNE $cc
        CMPEQ   dOP2l, dOP1l
        CmpReturn $cc

$lab._Inf_or_NaN                            ; one of the operands is infinite or NaN
        MOV     tmp, #0x00200000
        CMN     tmp, dOP1h, LSL #1
        CMPEQ   dOP1l, #0                   ; HI -> NaN found
        CMNLS   tmp, dOP2h, LSL #1          ; no NaN, check opnd2          
        CMPEQ   dOP2l, #0
        BHI     $NaN_lab                    ; NaN found -> exception
        ORRS    tmp, dOP1h, dOP2h
        BPL     $lab._cmp_pos
        B       $lab._cmp_neg

        MEND

;==============================================================================
; Equality/inequality - result in flags

        [ :DEF: eqf_s
        
        Export _dcmpeq
        IMPORT  __fp_dcheck_NaN2

_dcmpeq DoubleCompare FLAGS, dcmpeq_NaN

dcmpeq_NaN
        MOV     ip, #IVO_bit :OR: CmpEqualFn :OR: Double_bit
        B       __fp_dcheck_NaN2

        ]

;==============================================================================
;Equality

        [ :DEF: eq_s
        
        Export _deq
        IMPORT  __fp_dcheck_NaN2

_deq    DoubleCompare EQ, deq_NaN

deq_NaN
        MOV     ip, #IVO_bit :OR: CompareFalseFn :OR: Double_bit
        B       __fp_dcheck_NaN2

        ]

;==============================================================================
;Inequality

        [ :DEF: neq_s

        Export _dneq
        IMPORT  __fp_dcheck_NaN2

_dneq   DoubleCompare NE, dneq_NaN

dneq_NaN
        MOV     ip, #IVO_bit :OR: CompareTrueFn :OR: Double_bit
        B       __fp_dcheck_NaN2

        ]

;==============================================================================
;Less Than or Equal

        [ :DEF: leqf_s

        Export _dcmple
        IMPORT  __fp_exception

_dcmple DoubleCompare FLAGS, dcmple_NaN

dcmple_NaN
        MOV     ip, #IVO_bit :OR: CmpLessFn :OR: Double_bit
        B       __fp_exception       
        
        ]

;==============================================================================
;Greater Than or Equal

        [ :DEF: geqf_s

        Export _dcmpge
        IMPORT  __fp_exception

_dcmpge DoubleCompare FLAGS, dcmpge_NaN

dcmpge_NaN
        MOV     ip, #IVO_bit :OR: CmpGreaterFn :OR: Double_bit
        B       __fp_exception       

        ]

;==============================================================================
;Less Than

        [ :DEF: ls_s

        Export _dls
        IMPORT  __dcmp_NaN

_dls    DoubleCompare LO, __dcmp_NaN

        ]

;==============================================================================
;Less Than or Equal

        [ :DEF: leq_s

        Export _dleq
        IMPORT  __dcmp_NaN

_dleq   DoubleCompare LS, __dcmp_NaN

        ]

;==============================================================================
;Greater Than

        [ :DEF: gr_s

        Export _dgr
        IMPORT  __dcmp_NaN

_dgr    DoubleCompare HI, __dcmp_NaN

        ]

;==============================================================================
;Greater Than or Equal

        [ :DEF: geq_s

        Export _dgeq
        IMPORT  __dcmp_NaN

_dgeq   DoubleCompare HS, __dcmp_NaN

        ]

;==============================================================================
;Invalid Operation checking (NaN on compares)

        [ :DEF: compare_s

        IMPORT  __fp_exception
        EXPORT  __dcmp_NaN

        CODE_32

__dcmp_NaN
        MOV     ip, #IVO_bit :OR: CompareFalseFn :OR: Double_bit
        B       __fp_exception

        ]

;==============================================================================

        END
