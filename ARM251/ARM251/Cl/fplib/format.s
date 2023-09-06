; format.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.18 $
; Checkin $Date: 1998/08/03 16:31:27 $
; Revising $Author: wdijkstr $

        GET     fpe.s

        CodeArea |FPL$$Format|

SNaNInf EQU     NaNInfExp_Single - EIExp_bias + SExp_bias
DNaNInf EQU     NaNInfExp_Double - EIExp_bias + DExp_bias
DExp_max * 2047
SFrac_len * 23

exp     RN 2
sign    RN 3
tmp     RN 12

;==============================================================================
;Format conversions

        [ :DEF: d2f_s

        Export  _d2f
        IMPORT  __fp_exception
        IMPORT  __fp_return_NaN
        IMPORT  __flt_underflow

_d2f    EnterWithLR_16
        AND     sign, dOPh, #Sign_bit
        BIC     dOPh, dOPh, #Sign_bit
        ; convert double exponent to single exp by rebiasing
        SUB     dOPh, dOPh, #(DExp_bias - SExp_bias) << DExp_pos
        ; check whether exp is in range 1 .. 253 
        SUB     tmp, dOPh, #1 << DExp_pos   
        CMP     tmp, #253 << DExp_pos
        BHS     _d2f_uncommon                   ; exp <= 0 or >= 254
_d2f_round
        ORRS    tmp, sign, dOPl, LSR #29
        ADC     fOP, tmp, dOPh, LSL #3
        ReturnToLR CC
        MOVS    dOPl, dOPl, LSL #4
        ReturnToLR NE
        BIC     fOP, fOP, #1
        ReturnToLR

_d2f_uncommon                   
        ; exp out of range - check for special cases (PL = overflow)
        MOV     exp, dOPh, ASR #DExp_pos
        BPL     _d2f_ExpOverflow

_d2f_ExpUnderflow
        ; return zero or denorm via underflow exception
        CMP     dOPh, #(SExp_bias - DExp_bias) << DExp_pos
        CMPEQ   dOPl, #0
        MOVEQ   fOP, sign
        ReturnToLR EQ
        RSB     exp, exp, #SExp_len+1           ; right shift for result
        MOV     dOPh, dOPh, LSL #DExp_len
        ORR     dOPh, dOPh, dOPl, LSR #DFhi_len+1
        MOVS    dOPl, dOPl, LSL #32 - (DFhi_len+1)
        ORRNE   dOPh, dOPh, #1                  ; add guard bit
        ORR     a1, dOPh, #1 << 31              ; add leading bit
        MOV     sign, sign, LSR #31
        CMP     exp, #255                       ; max exp to flt_underflow is 255
        MOVHI   exp, #255       
        B       __flt_underflow

_d2f_ExpOverflow
        ; if exponent 254 - test for overflow during rounding
        CMP     exp, #254
        MOVEQ   tmp, dOPh, LSL #DExp_len + 1
        ORREQ   tmp, tmp, dOPl, LSR #DFhi_len
        CMNEQ   tmp, #1 << 8                    ; if dOP >= 1.FFFFFF00 E254
                                                ;   overflow occurs while rounding         
        BLO     _d2f_round                      ; no - continue (10 clk overhead)
        ; real overflow, inf or NaN 
        ADD     exp, exp, #1                
        TEQ     exp, #DExp_max - DExp_bias + SExp_bias + 1  ; check for inf/NaN
        MOVNE   fOP, sign                       ; no - overflow exception
        MOVNE   ip, #OVF_bit
        BNE     __fp_exception

_d2f_Inf_or_NaN                                 ; found inf or NaN
        ORRS    tmp, dOPl, dOPh, LSL #DExp_len + 1  ; EQ -> infinity
        MOVEQ   tmp, #0xFF000000
        ORREQ   fOP, sign, tmp, LSR #1          ; return signed infinity
        ReturnToLR EQ 
        MOVS    tmp, dOPh, LSL #DExp_len + 1    ; signalling NaN? (PL)
        LDRMI   fOP, fNaN                       ; return default quiet NaN
        ReturnToLR MI
        MOV     ip, #IVO_bit
        B       __fp_exception                  ; signal exception

fNaN    DCD &7FC00000

        ]

;------------------------------------------------------------------------------

        [ :DEF: f2d_s

        Export  _f2d
        IMPORT  __fp_exception
        IMPORT  __dflt_normalise
        IMPORT  __fp_return_NaN

_f2d    EnterWithLR_16
        ADD     tmp, fOP, #1 << SExp_pos    ; filter out inf/NaN/denorm/zero
        TST     tmp, #254 << SFrac_len
        BEQ     _f2d_uncommon
        MOV     dOPl, fOP, LSL #32 - 3
        MOVS    dOPh, fOP, ASR #3           ; widen exp field by extending sign
        ADD     dOPh, dOPh, #(DExp_bias - SExp_bias) << DExp_pos
        ReturnToLR PL
        SUB     dOPh, dOPh, #0x700 << DExp_pos  ; negative: adjust for the ASR #3
        ReturnToLR

_f2d_uncommon
        TST     tmp, #1 << SExp_pos         ; inf/NaN -> EQ, zero/denorm ->NE
        BEQ     _f2d_Inf_or_NaN
_f2d_denorm
        MOVS    dOPl, fOP, LSL #1           ; zero -> EQ
        ReturnToLR EQ                       ; dOPl zero, dOPh sign bit
        ; normalise using dflt_normalise - input exp of bit 0
        ; shift sign out a1 - apply extra shift for 8-bit constant exp
        AND     exp, fOP, #Sign_bit
        ADD     a4, exp, #(DExp_bias-SExp_bias - 24) << DExp_pos
        MOV     a1, fOP, LSL #3
        B       __dflt_normalise        

_f2d_Inf_or_NaN                             ; fOP is NaN/infinity. 
        MOVS    dOPl, fOP, LSL #SExp_len+1  ; EQ -> inf
        ORREQ   dOPh, fOP, #0x007 << DExp_pos   ; tranform float inf to double inf
        ReturnToLR EQ
        ; MI if quiet NaN - sign in fOP
        MOV     ip, #IVO_bit :OR: Double_bit
        BPL     __fp_exception              ; signal exception
        LDR     dOPh, dNaN                  ; return default quiet NaN
        MOV     dOPl, #0
        ReturnToLR

dNaN    DCD &7FF80000

        ]

;==============================================================================

        END
