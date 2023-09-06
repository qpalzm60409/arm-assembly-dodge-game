; format_f.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.27 $
; Checkin $Date: 1998/08/03 16:31:28 $
; Revising $Author: wdijkstr $

        GET     fpe.s

        CodeArea |FPL$$Format|

SNaNInf EQU     NaNInfExp_Single - EIExp_bias + SExp_bias
SExp_max * 255

exp     RN 3

;==============================================================================

        [ :DEF: f2e_s

        CODE_32

        IMPORT  __fp_norm_op1
        EXPORT  _f2e            ; Not needed, but is an example

_f2e    EnterWithLR
        MOVS    tmp,fOP,LSL #1          ;C:=sign; Z:=exp & frac.top zero
        MOV     OP1mhi,tmp,LSL #7       ;Frac.top in bits 30:11 of mhi
        MOV     fOP,tmp,LSR #20         ;Exponent in bits 11:1
        ADDNE   fOP,fOP,#(EIExp_bias - SExp_bias):SHL:1
        MOV     OP1sue,fOP,RRX         ;Recombine sign and exponent
        ORRNE   OP1mhi,OP1mhi,#EIUnits_bit
        MOV     OP1mlo,#0

        ; Single precision exponent<<1+SExp_pos is in tmp.
        ; If 0 then this is a denormalised number.
        ; If 1fe then this is an uncommon number.
        MOVS    tmp,tmp,LSR #1+SExp_pos
        ADDEQ   lr,pc,#8        ;Skip two instructions past normalise call
        BEQ     __fp_norm_op1
        TEQ     tmp,#&ff
        ORREQ   OP1sue,OP1sue,#Uncommon_bit
        ReturnToLR

        ]

;---------------------------------------------------------------------------

        [ :DEF: e2f_s

; Called from ARM code (no INTERWORK hacks needed).

        CODE_32

        EXPORT  __fp_e2f

__fp_e2f
        
;Passed in the result of an operation. That is the uncommon/sign are in
;OP1sue, the exponent in RNDexp, the fraction in OP1mlo/OP1mhi. Return with
;a result in dOPh/dOPl and possibly some error flags in a4

        ASSERT  Uncommon_bit = 1:SHL:(31-1)
        ASSERT  EIUnits_bit = 1:SHL:31
        BICS    tmp, OP1mhi, OP1sue, LSL #1
        BPL     _e2f_SpecialCase

        SUBS    RNDexp, RNDexp, #(EIExp_bias-SExp_bias)
        BMI     _e2f_ExpUnderflow

        ASSERT  tmp <> RNDexp
        ADDNE   tmp, RNDexp, #1
        CMPNE   tmp, #SNaNInf+1
        BGE     _e2f_ExpOverflow

        MOVS    tmp, OP1mhi, LSR #32-SFrc_len-1
;If rounding is needed then a one will have dropped out.
        BCS     _e2f_NeedsRounding

; Simple case - just merge in the exponent and sign

        BIC     tmp, tmp, #1<<SFrc_len  ; clear out the J-bit
        ORR     fOP, OP1sue, RNDexp, LSL #SExp_pos
        ORR     fOP, fOP, tmp
        MOV     a4, #0
        MOV     pc, lr

;...........................................................................

_e2f_SpecialCase

        TST     OP1sue, #Uncommon_bit
        BNE     _e2f_Uncommon

_e2f_UnitsBit

;Sign is in OP1sue's top bit. The units bit of OP1mhi is clear indicating a
;zero value (since the denorm case is handled by the uncommon bit).

        AND     dOPh, OP1sue, #Sign_bit
        MOV     a4, #0
        MOV     pc, lr

;...........................................................................

_e2f_ExpOverflow

;Sign is in OP1sue's sign bit. May still need rounding. The exponent (RNDexp)
;is out of range for a double precision number, and wasn't signalled as a NaN.

;The exponent has been re-biased.

;Might just be the "just underflowing case" (because of a quirk above). In
;that case RNDexp = 0

        TEQ     RNDexp, #0
        MOVNE   a4, #OVF_bits
        MOVNE   pc, lr

;...........................................................................

_e2f_ExpUnderflow


;Underflow. If the value can be represented as a denorm in the target
;precision, then it should be. For this to be the case the exponent needs
;to lie in range. Otherwise the result is a suitably
;signed zero. (A zero is, however, a denorm with zero mantissa.)

;The exponent (RNDexp) has been rebiased (by (EIExp_bias-SExp_bias))

        ADDS    RNDexp, RNDexp, #SFrc_len

;If the result is zero then we round according to the top bit of the
;fraction. Branch out of line to do this.

        BEQ     _e2f_ExpJustUnderflow

;If this result is negative, nothing can be done, so return a signed zero.

        ANDMI   dOPh, OP1sue, #Sign_bit
        MOVMI   dOPl, #0
        MOVMI   a4, #0
        MOVMI   pc, lr

;We now have in OP1mhi 1MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
;and in OP1mlo         MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
;These need to be converted into a denormalised number. We want the N
;left most bits (N<=23) from this 64-bit value. So we are throwing
;away all the bits from the low word, and some from the high word.
;RNDexp contains the value ([1..23]) 

        MOVS    tmp, OP1mhi, LSL RNDexp
        BMI     _e2f_UnderflowNeedsRounding

        RSB     tmp, RNDexp, #32
        ORR     fOP, OP1sue, OP1mhi, LSR tmp
        MOV     a4, #0
        MOV     pc, lr

_e2f_UnderflowNeedsRounding

;tmp contains those bits out of the high word that determine rounding.
;We add in the bits from the low word, and the sticky bits.

        ORR     tmp, Rarith, tmp, LSL #1
        ORRS    tmp, tmp, OP1mlo                ;C=1; Z=round to even

        RSB     tmp, RNDexp, #32

;Merge in the sign bit and add one (C flag). Undo the rounding if we
;need to round to even and the result

        ORR     fOP, OP1sue, OP1mhi, LSR tmp
        MOV     a4, #0
        TSTEQ   fOP, #1                         ;test for even
        ADDNE   fOP, fOP, #1
        MOV     pc, lr

;...........................................................................

_e2f_ExpJustUnderflow

;The number may just be representable as the single precision epsilon.

        ORR     tmp, OP1mlo, OP1mhi, LSL #1
        ORRS    tmp, tmp, Rarith
        ASSERT  OP1sue = fOP
        ORRNE   fOP, OP1sue, #1
        MOV     a4, #0
        MOV     pc, lr

;...........................................................................

_e2f_Uncommon

        TST     OP1sue, #Error_bit
        BNE     _e2f_Error
        MOV     a4, #0

;And infinity or a NaN. Infinity is signalled by mhi=mlo=0

        TEQ     OP1mhi, #0
        MOVNES  OP1mhi, OP1mhi, LSL #1
        MOVMI   fOP, #-1
        MOVMI   pc, lr
        TEQEQ   OP1mlo, #0
        MOVNE   a4, #IVO_bits
        MOVNE   pc, lr

;An infinity, whose sign is in the sue register.

        MOVS    OP1sue, OP1sue, LSL #1          ; C<-sign
        MOV     fOP, #&ff000000
        MOV     fOP, fOP, RRX                   ; add sign
        MOV     pc, lr

_e2f_Error

        MOV     a4, OP1sue
        MOV     pc, lr

;...........................................................................

_e2f_NeedsRounding

;The sign is in OP1sue, the exponent (in range) is in RNDexp.

        BIC     OP1mhi, OP1mhi, #EIUnits_bit

;Get the bits that are to be thrown away, except for the topmost one.

        ORR     tmp, OP1mlo, OP1mhi, LSL #SFrc_len+2
        ORRS    tmp, tmp, Rarith        ; Z<-round to even

        AND     tmp, OP1sue, #Sign_bit  ; save the real sign
        MOV     fOP, RNDexp, LSL #SExp_pos
        ORR     fOP, fOP, OP1mhi, LSR #32-SFrc_len-1

        MOV     a4, #0

        BEQ     _e2f_RoundToEven        ; go out of line for speed

        ADD     fOP, fOP, #1
        ADDS    a2, fOP, #1<<SExp_pos   ; can't be done in one add :-(
        ORR     fOP, fOP, tmp
        MOVPL   pc, lr
        MOV     a4, #OVF_bits
        MOV     pc, lr

_e2f_RoundToEven
        TSTEQ   fOP, #1
        ORREQ   fOP, fOP, tmp
        MOVEQ   pc, lr

        ADD     fOP, fOP, #1
        ADDS    a2, fOP, #1<<SExp_pos   ; can't be done in one add :-(
        ORR     fOP, fOP, tmp
        MOVPL   pc, lr
        MOV     a4, #OVF_bits
        MOV     pc, lr

        ]       

;...........................................................................

        [ :DEF: fflt_s

        Export  _fflt
        Export  _ffltu
        EXPORT __fflt_normalise

_ffltu  EnterWithLR_16
        MOV     exp, #(SExp_bias+1) << SExp_pos
        B       __fflt_normalise

_fflt   EnterWithLR_16
        ANDS    exp, a1, #Sign_bit
        RSBNE   a1, a1, #0
        ORR     exp, exp, #(SExp_bias+1) << SExp_pos
        ; fallthrough

__fflt_normalise        
        MOVS    a2, a1, LSR #16
        ADDNE   exp, exp, #16 << SExp_pos
        MOVEQS  a1, a1, LSL #16
        ReturnToLR EQ                   ; a1 was zero - return +0
        TST     a1, #0xFF000000
        ADDNE   exp, exp, #8 << SExp_pos
        MOVEQ   a1, a1, LSL #8
        TST     a1, #0xF0000000
        ADDNE   exp, exp, #4 << SExp_pos
        MOVEQ   a1, a1, LSL #4
        TST     a1, #0xC0000000
        ADDNE   exp, exp, #2 << SExp_pos
        MOVEQS  a1, a1, LSL #2
        ; MI if high bit set, PL if not
        ADDMI   exp, exp, #1 << SExp_pos
        MOVPL   a1, a1, LSL #1
        MOVS    a2, a1, LSL #25             ; CS -> round, EQ round to even
        ADC     a1, exp, a1, ASR #8         ; combine, subtract one from exponent
        ReturnToLR CC
        ReturnToLR NE
        BIC     a1, a1, #1
        ReturnToLR
 
        ]

;...........................................................................

        [ :DEF: ffltll_s

shift   RN 2

_ll_uto_f EnterWithLR_16
        MOV     exp, #SExp_bias << SExp_pos
        B       fltll_normalise

_ll_sto_f EnterWithLR_16
        ANDS    exp, lh, #Sign_bit
        BPL     %F0
        RSBS    ll, ll, #0
        RSC     lh, lh, #0
0       ORR     exp, exp, #SExp_bias << SExp_pos
        ; fallthrough

fltll_normalise
        ADD     exp, exp, #30 << SExp_pos
        MOVS    shift, lh               ; shift = 32 if high word nonzero
        MOVNE   shift, #32
        MOVEQS  lh, ll
        ReturnToLR EQ                   ; (lh,ll) was zero - return +0
        MOVS    tmp, lh, LSR #16
        SUBEQ   shift, shift, #16
        MOVEQS  lh, lh, LSL #16
        TST     lh, #255 << 24
        SUBEQ   shift, shift, #8
        MOVEQ   lh, lh, LSL #8
        TST     lh, #15 << 28
        SUBEQ   shift, shift, #4
        MOVEQ   lh, lh, LSL #4
        TST     lh, #3 << 30
        SUBEQ   shift, shift, #2
        MOVEQS  lh, lh, LSL #2
        MOVPL   lh, lh, LSL #1
        SUBPL   shift, shift, #1

        ORR     lh, lh, ll, LSR shift   ; if shift negative then lh unaltered 
        ADD     exp, exp, shift, LSL #23
        MOVS    tmp, lh, LSL #25        ; CS -> round, EQ -> round to even
        MOV     tmp, ll                 ; save ll!
        ADC     fOP, exp, lh, LSR #8
        ReturnToLR CC
        ReturnToLR NE
        RSB     shift, shift, #32
        MOVS    tmp, tmp, LSL shift
        BICEQ   fOP, fOP, #1
        ReturnToLR

        ]

;===========================================================================

        [ :DEF: ffix_s

        Export  _ffix
        IMPORT  __fp_exception

_ffix   EnterWithLR_16
        MOVS    exp, fOP, ASR #SFrc_len
        MOV     a3, fOP, LSL #SExp_len
        ORR     a3, a3, #1 << 31
        BMI     ffix_negative
        RSBS    exp, exp, #31 + SExp_bias
        MOVHI   a1, a3, LSR exp
        ReturnToLR HI                   ; 9 clk
        B       ffix_ivo

ffix_negative
        AND     exp, exp, #255
        RSBS    exp, exp, #31 + SExp_bias
        MOVHS   a1, a3, LSR exp
        RSBHI   a1, a1, #0              ; negative result
        ReturnToLR HI                   ; 11 clk
        TEQEQ   a1, #0x80000000         ; MinInt special case
        ReturnToLR EQ
        
ffix_ivo        ; sign in a1  
        CMP     exp, #(31 + SExp_bias) - SExp_max    ; EQ -> inf or NaN 
        MOV     ip, #IVO_bit :OR: FixFn
        BNE     __fp_exception
        MOVS    a2, a1, LSL #SExp_len+1 ; NE -> NaN
        MOVNE   ip, #IVO_bit :OR: FixZeroFn
        B       __fp_exception

        ]
        

;---------------------------------------------------------------------------

        [ :DEF: ffixu_s

        Export  _ffixu
        IMPORT  __fp_exception

_ffixu  EnterWithLR_16
        MOVS    exp, fOP, ASR #SFrc_len
        BMI     _ffixu_negative
        MOV     a3, fOP, LSL #SExp_len
        ORR     a3, a3, #1 << 31
        RSBS    exp, exp, #31 + SExp_bias
        MOVHS   a1, a3, LSR exp
        ReturnToLR HS
        B       _ffixu_ivo  

_ffixu_negative     
        SUBS    exp, exp, #0xFFFFFF00 + SExp_bias ; exp < 127?
        MOVLO   a1, #0                  ; yes -> -0.999 .. -0.0 fix to 0
        ReturnToLR LO
        RSB     exp, exp, #31

_ffixu_ivo      ; sign in a1
        CMP     exp, #(31 + SExp_bias) - SExp_max    ; EQ -> inf or NaN 
        MOV     ip, #IVO_bit :OR: FixuFn
        BNE     __fp_exception
        MOVS    a2, a1, LSL #SExp_len+1 ; NE -> NaN
        MOVNE   ip, #IVO_bit :OR: FixZeroFn
        B       __fp_exception

        ]

;---------------------------------------------------------------------------

        [ :DEF: ll_sfrom_f_s

        IMPORT  __fp_exception

_ll_sfrom_f EnterWithLR_16
        MOVS    exp, fOP, ASR #SFrc_len
        MOV     a3, fOP, LSL #SExp_len
        ORR     a3, a3, #1 << 31
        BMI     ffixll_negative
        RSBS    exp, exp, #63 + SExp_bias
        BLS     ffixll_ivo
        SUBS    tmp, exp, #32
        MOVHS   ll, a3, LSR tmp
        MOV     lh, a3, LSR exp
        ReturnToLR HS                   ; 13 clk
        RSB     exp, exp, #32
        MOV     ll, a3, LSL exp
        ReturnToLR                      ; 16 clk

ffixll_negative
        AND     exp, exp, #255
        RSBS    exp, exp, #63 + SExp_bias
        BLS     ffixll_ivoneg
1       SUBS    tmp, exp, #32
        MOVHS   ll, a3, LSR tmp
        MOV     lh, a3, LSR exp
        RSBLO   exp, exp, #32
        MOVLO   ll, a3, LSL exp
        RSBS    ll, ll, #0
        RSC     lh, lh, #0
        ReturnToLR                      ; 19 clk

ffixll_ivoneg
        TEQEQ   a3, #0x80000000         ; MinInt special case
        BEQ     %B1
ffixll_ivo
        ; sign of result in a1
        CMP     exp, #(63 + SExp_bias) - SExp_max    ; EQ -> inf or NaN 
        MOV     ip, #IVO_bit :OR: FixFn :OR: LongLong_bit
        BNE     __fp_exception
        MOVS    a2, fOP, LSL #SExp_len+1    ; NE -> NaN
        MOVNE   ip, #IVO_bit :OR: FixZeroFn :OR: LongLong_bit
        B       __fp_exception

        ]

;---------------------------------------------------------------------------

        [ :DEF: ll_ufrom_f_s

        IMPORT  __fp_exception

_ll_ufrom_f EnterWithLR_16
        MOVS    exp, fOP, ASR #SFrc_len
        MOV     a3, fOP, LSL #SExp_len
        ORR     a3, a3, #1 << 31
        BMI     ffixull_negative
        RSBS    exp, exp, #63 + SExp_bias
        BLO     ffixull_ivo
        SUBS    tmp, exp, #32
        MOVHS   ll, a3, LSR tmp
        MOV     lh, a3, LSR exp
        ReturnToLR HS               ; 13 clk
        RSB     exp, exp, #32
        MOV     ll, a3, LSL exp
        ReturnToLR                  ; 16 clk

ffixull_negative
        SUBS    exp, exp, #0xFFFFFF00 + SExp_bias ; exp < 127?
        MOVLO   ll, #0                  ; yes -> -0.999 .. -0.0 fix to 0
        MOVLO   lh, #0
        ReturnToLR LO
        RSB     exp, exp, #63

ffixull_ivo
        CMP     exp, #(63 + SExp_bias) - SExp_max    ; EQ -> inf or NaN 
        MOV     ip, #IVO_bit :OR: FixuFn :OR: LongLong_bit
        BNE     __fp_exception
        MOVS    a2, a1, LSL #SExp_len+1 ; NE -> NaN
        MOVNE   ip, #IVO_bit :OR: FixZeroFn :OR: LongLong_bit
        B       __fp_exception

        ]

;===========================================================================

        END
