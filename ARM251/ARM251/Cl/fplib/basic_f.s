; basic_f.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.14 $
; Checkin $Date: 1998/03/19 15:04:08 $
; Revising $Author: wdijkstr $

; Basic floating point functions

        GET     fpe.s

        CodeArea |FPL$$Basic|


;This code is very similar to the "double" versions. The documentation isn't
;extensively repeated. Refer to basic_d.s for further documentation.


;==============================================================================
; Compare.
;
; Timing:
; Flags: 7 (pos), 11 (false NaN), 9 (neg), 13 (false NaN) SA1.1 cycles
; Others: 9 / 13 / 11 / 15
;==============================================================================


        MACRO
        CmpReturn $cc

        [ "$cc" = "FLAGS"
            ReturnToLR_flg
        |
            MOV     a1, #0
            MOV$cc  a1, #1
            ReturnToLR
        ]

        MEND


        MACRO
$lab    FloatCompare $cc, $NaN_lab

        ASSERT "$cc"="FLAGS":LOR:"$cc"="LO":LOR:"$cc"="LS":LOR:"$cc"="HS":LOR:"$cc"="HI":LOR:"$cc"="EQ":LOR:"$cc"="NE"

$lab    EnterWithLR_16
        ORRS    tmp, fOP1, fOP2         ; separate into opnd1/2 both positive, or one negative
        BMI     $lab._negative
        CMN     tmp, #1 << 23           ; check whether operands might be infinite/NaN
        BMI     $lab._NaN_check_pos
        CMP     fOP1, fOP2
        CmpReturn $cc
        
$lab._NaN_check_pos                     ; opnd1/2 might be inf/NaNs - now do the real check
        CMN     fOP1, #1 << 23          ; these get about 9% false hits - overhead 4 cycles
        CMNPL   fOP2, #1 << 23
        BMI     $lab._Inf_or_NaN_pos
$lab._cmp_pos
        CMP     fOP1, fOP2
        CmpReturn $cc

$lab._Inf_or_NaN_pos                    ; at least one operand infinite or NaN - filter out infinities
        MOV     tmp, #1 << 24
        CMN     tmp, fOP1, LSL #1
        CMNLS   tmp, fOP2, LSL #1
        BLS     $lab._cmp_pos           ; no NaN - continue compare
        B       $NaN_lab

$lab._negative                          ; at least one negative operand
        CMN     tmp, #1 << 23
        BPL     $lab._NaN_check_neg
        MOVS    tmp, tmp, LSL #1        ; check -0 == 0 (CS & EQ -> HS and LS)
        CMPNE   fOP2, fOP1
        CmpReturn $cc

$lab._NaN_check_neg                     ; opnd1/2 might be inf/NaNs - now do the real check
        MOV     tmp, #1 << 24           ; these get about 9% false hits - overhead 4 cycles
        CMN     tmp, fOP1, LSL #1
        CMNLS   tmp, fOP2, LSL #1
        BHI     $NaN_lab
        CMP     fOP2, fOP1              ; -0 == 0 check not needed...
        CmpReturn $cc

        MEND

;==============================================================================
; Equality - result in flags

        [ :DEF: eqf_s

        Export _fcmpeq
        IMPORT __fp_fcheck_NaN2

_fcmpeq FloatCompare FLAGS, fcmpeq_NaN

fcmpeq_NaN
        MOV     ip, #IVO_bit :OR: CmpEqualFn
        B       __fp_fcheck_NaN2

        ]

;==============================================================================
; Equality

        [ :DEF: eq_s

        Export _feq
        IMPORT __fp_fcheck_NaN2

_feq    FloatCompare EQ, feq_NaN

feq_NaN
        MOV     ip, #IVO_bit :OR: CompareFalseFn
        B       __fp_fcheck_NaN2

        ]

;==============================================================================
;Inequality

        [ :DEF: neq_s

        Export _fneq
        IMPORT __fp_fcheck_NaN2

_fneq   FloatCompare NE, fneq_NaN

fneq_NaN
        MOV     ip, #IVO_bit :OR: CompareTrueFn
        B       __fp_fcheck_NaN2

        ]

;==============================================================================
; Less Than or Equal - result in flags

        [ :DEF: leqf_s

        Export _fcmple
        IMPORT __fp_exception

_fcmple FloatCompare FLAGS, fcmple_NaN
       
fcmple_NaN
        MOV     ip, #IVO_bit :OR: CmpLessFn
        B       __fp_exception

        ]

;==============================================================================
;Greater Than or Equal - result in flags

        [ :DEF: geqf_s

        Export _fcmpge
        IMPORT __fp_exception

_fcmpge FloatCompare FLAGS, fcmpge_NaN
        
fcmpge_NaN
        MOV     ip, #IVO_bit :OR: CmpGreaterFn
        B       __fp_exception

        ]

;==============================================================================
;Less Than

        [ :DEF: ls_s

        Export _fls
        IMPORT __fcmp_NaN

_fls    FloatCompare LO, __fcmp_NaN

        ]

;==============================================================================
;Less Than or Equal

        [ :DEF: leq_s

        Export _fleq
        IMPORT __fcmp_NaN

_fleq   FloatCompare LS, __fcmp_NaN

        ]

;==============================================================================
;Greater Than

        [ :DEF: gr_s

        Export _fgr
        IMPORT __fcmp_NaN

_fgr    FloatCompare HI, __fcmp_NaN

        ]

;==============================================================================
;Greater Than or Equal

        [ :DEF: geq_s

        Export _fgeq
        IMPORT __fcmp_NaN

_fgeq   FloatCompare HS, __fcmp_NaN

        ]


;==============================================================================
;Invalid Operation checking (NaN on compares)
;Called from 32-bit (ARM) code

        [ :DEF: compare_s

        EXPORT __fcmp_NaN
        IMPORT __fp_exception

        CODE_32

__fcmp_NaN
        MOV     ip, #IVO_bit :OR: CompareFalseFn
        B       __fp_exception 

        ]

;==============================================================================

        END
