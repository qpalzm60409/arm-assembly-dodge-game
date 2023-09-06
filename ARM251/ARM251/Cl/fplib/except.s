; except.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.29.2.4 $
; Checkin $Date: 1999/10/05 10:16:14 $
; Revising $Author: statham $

;==============================================================================
;Error generation - from ANSI library

        GET     fpe.s
        GET     h_errors.s

        MACRO   
        LoadFPStatus $reg, $label
        ; Return a pointer to __fp_status_flags in $reg.
        ; Does not corrupt any other register
    [ EMBEDDED_CLIB
        IMPORT  __rt_fp_status_addr, WEAK
        LDR     $reg, =__rt_fp_status_addr
        CMP     $reg, #0
        BEQ     $label
        STMFD   sp!, {r0-r3, ip, lr}
        BL      __rt_fp_status_addr
        LDMIB   sp, {r1-r3, ip, lr}
        [   "$reg" <> "R0"
        MOV     $reg, r0
        LDR     r0, [sp]
        ]
        ADD     sp, sp, #6 * 4
    |
        IMPORT  __fp_status_flags
        LDR     $reg, =__fp_status_flags
    ]
        MEND



        [ :DEF: liberror_s

        CodeArea |FPL$$Exception|

        Export_32  __fp_edom
        Export_32  __fp_erange

;  __fp_edom(ulong sign_bit, boolean huge_val);
;  __fp_erange(ulong sign_bit, boolean huge_val);
;
; set errno to EDOM/ERANGE and return sign_bit | (huge_val ? HUGE_VAL : 0)

__fp_edom EnterWithLR
        MOV     a3, #EDOM
        B       skip
__fp_erange EnterWithLR
        MOV     a3, #ERANGE
skip
        AND     a4, a1, #Sign_bit
    [ EMBEDDED_CLIB
        IMPORT  __rt_errno_addr, WEAK
        LDR     a1, =__rt_errno_addr
        CMP     a1, #0
        BEQ     %FT1
        STMDB   sp!, {a2-a4, lr}
        BL      __rt_errno_addr
        LDMIA   sp!, {a2-a4, lr}
        STR     a3, [a1]
1
    |
        LDR     a1, errno
        STR     a3, [a1]
    ]
        TEQ     a2, #0
        MOVEQ   a1, #0          ; generate +/- 0.0 in a1/a2
        LDRNE   a1, huge_val
        LDMNEIA a1, {a1, a2}    ; load HUGE_VAL into a1/a2
        ORR     a1, a1, a4      ; add in sign bit
        ReturnToLR

    [ :LNOT: EMBEDDED_CLIB
errno
        IMPORT  __errno
        DCD     __errno
    ]
 
huge_val
        IMPORT  __huge_val
        DCD     __huge_val

        ]

;==============================================================================
;Setting/returning status flags.

        [ :DEF: status_s

        CodeArea |FPL$$Exception|

        EXPORT  __fp_status_arm

; extern unsigned int __fp_status(unsigned int mask,unsigned int flags)

        CODE_32

__fp_status EnterWithLR
__fp_status_arm
        LoadFPStatus a4, no_fp_status
        LDR     a3, =&FFF8FFE0   ; bitmask for read-only bits
        BIC     a2, a2, a3       ; mask out read-only bits
        BIC     a3, a1, a3
        ; mask in a3, new bits in a2
        LDR     a1,[a4]          ; load old flags
        BIC     a3, a1, a3
        EOR     a3, a3, a2       ; clear/set/toggle flags and
        STR     a3,[a4]          ; write back.
        ReturnToLR

        [ EMBEDDED_CLIB
no_fp_status
        MOV     a1, #IOE_bit + DZE_bit + OFE_bit
        ReturnToLR
        ]

        ]

;==============================================================================
;Error checking - from fplib code

        [ :DEF: fcheck_s

        CodeArea |FPL$$Exception|

        CODE_32

        EXPORT __fp_fcheck_NaN2
        EXPORT __fp_fcheck_NaN2s
        EXPORT __fp_dcheck_NaN2
        EXPORT __fp_dcheck_NaN2s
        EXPORT __fp_return_NaN
        EXPORT __fp_return_Inf
        EXPORT __flt_underflow
        EXPORT __flt_overflow
        IMPORT __fp_exception


; Check fOP1 and fOP2 for signalling NaN, IP contains exception flags.

__fp_fcheck_NaN2s
        TEQ     fOP1, fOP2
        BIC     fOP1, fOP1, #1 << 31
        BIC     fOP2, fOP2, #1 << 31
        ORRMI   fOP1, fOP1, #1 << 31
        ORRMI   fOP2, fOP2, #1 << 31
__fp_fcheck_NaN2    
        MOV     a4, #0x01000000
        CMN     a4, fOP1, LSL #1
        BLS     fcheck_opnd2_NaN
fcheck_opnd1_NaN
        TST     fOP1, #fSignalBit
        BEQ     __fp_exception
        CMN     a4, fOP2, LSL #1
        BLS     __fp_return_NaN
fcheck_opnd2_NaN
        MOV     fOP1, fOP2
        TST     fOP1, #fSignalBit
        BNE     __fp_return_NaN
        B       __fp_exception

; Check dOP1 and dOP2 for signalling NaN, IP contains exception flags.

__fp_dcheck_NaN2s
        TEQ     dOP1h, dOP2h
        BIC     dOP1h, dOP1h, #1 << 31
        BIC     dOP2h, dOP2h, #1 << 31
        ORRMI   dOP1h, dOP1h, #1 << 31
        ORRMI   dOP2h, dOP2h, #1 << 31
__fp_dcheck_NaN2
        STMFD   sp!, {ip}
        MOV     tmp, #0x00200000
        CMN     tmp, dOP1h, LSL #1
        CMPEQ   dOP1l, #0
        BLS     dcheck_opnd2_NaN
dcheck_opnd1_NaN
        TST     dOP1h, #dSignalBit
        LDMEQFD sp!, {ip}
        BEQ     __fp_exception
        CMN     tmp, dOP2h, LSL #1
        CMPEQ   dOP2l, #0
        LDMLSFD sp!, {ip}
        BLS     __fp_return_NaN
dcheck_opnd2_NaN
        LDMFD   sp!, {ip}
        MOV     dOP1h, dOP2h
        MOV     dOP1l, dOP2l
        TST     dOP1h, #dSignalBit
        BNE     __fp_return_NaN
        B       __fp_exception

; Return NaN in fOP / dOP, except for non-float returning functions. 
; IP contains exception flags.

__fp_return_NaN 
        ; test for special cases
        AND     a4, ip, #Fn_mask
        CMP     a4, #CompareFalseFn
        CMPNE   a4, #FixZeroFn
        BEQ     return_zero
        CMP     a4, #CompareTrueFn
        BEQ     return_one
        CMP     a4, #CmpLessFn
        BEQ     return_HI
        CMP     a4, #CmpGreaterFn
        BEQ     return_LO
        CMP     a4, #CmpEqualFn
        BEQ     return_NE
        CMP     a4, #FixFn
        BEQ     return_smaxint
        CMP     a4, #FixuFn
        BEQ     return_umaxint
        ReturnToLR

return_HI
        MOV     a1, #1
        CMP     a1, #0
        ReturnToLR_flg
       
return_LO             
        MOV     a1, #0
        CMP     a1, #1
        ReturnToLR_flg

return_NE             
        MOVS    a1, #1
        ReturnToLR_flg

return_one
        MOV     a1, #1
        ReturnToLR

return_zero
        MOV     a1, #0
        MOV     a2, #0
        ReturnToLR

return_smaxint
        MOV     a3, a1
        TST     ip, #LongLong_bit
        MOVEQ   a1, #0x7fffffff
        MOVNE   a1, #0xffffffff
        MOVNE   a2, #0x7fffffff
        TST     a3, #Sign_bit
        MVNNE   a1, a1
        MVNNE   a2, a2        
        ReturnToLR

return_umaxint
        MOV     a1, #0xffffffff
        MOV     a2, #0xffffffff
        TST     a3, #Sign_bit
        MVNNE   a1, a1
        MVNNE   a2, a2        
        ReturnToLR

__fp_return_Inf
        ; no special cases
        ReturnToLR


__flt_overflow
        MOV     ip, #OVF_bit
        B       __fp_exception


        ;  r3 - sign in bit 0 (negative if set)
        ;  r0 - underflowed number with leading bit set, round and guard bits
        ;  r2 - shift count for a (0 - exp)

res     RN 0
val     RN 0
sign    RN 3
exp     RN 2
flags   RN 1

__flt_underflow
        LoadFPStatus ip, no_fp_status

        MOV     sign, sign, LSL #31
        LDR     flags, [ip]
        TST     flags, #UFE_bit
        BNE     fp_underflow
        TST     flags, #ND_bit
        MOVNE   res, sign
        ReturnToLR NE
        ORRS    sign, sign, val, LSR exp
        BCS     fp_underflow_carry
        AND     exp, exp, #255
        RSBS    exp, exp, #32
        MOVLO   exp, #0                 ; clamp exp
        MOVS    exp, val, LSL exp
        MOV     res, sign
        ReturnToLR EQ
        ORR     flags, flags, #UFC_bit
        STR     flags, [ip]
        ReturnToLR

        [ EMBEDDED_CLIB
no_fp_status
        ReturnToLR
        ]

fp_underflow_carry
        ORR     flags, flags, #UFC_bit
        STR     flags, [ip]
        RSB     exp, exp, #33
        MOVS    exp, val, LSL exp       ; will set carry
        ADC     res, sign, #0
        BICEQ   res, res, #1
        ReturnToLR
         
fp_underflow
        MOV     ip, #UNF_bit :OR: ArithFN
        B       __fp_exception                     

        ]

;==============================================================================
;Error generation - from fplib code

        [ :DEF: except_s

        CodeArea |FPL$$Exception|

; SWI Names

        CODE_32

        EXPORT  __fp_veneer_error
        EXPORT  __fp_nonveneer_error
        EXPORT  __fp_exception
        IMPORT  __fp_return_NaN
        IMPORT  __fp_return_Inf


__fp_veneer_error           ; a4 contains the exception flags
        VPull
__fp_nonveneer_error        ; a4 contains the exception flags OBSOLETE
        MOV     ip, a4
        ; fallthrough

; fp_exception is called when an IEEE exception has occurred 

; a1 contains the sign of the NaN to be returned if the exception is disabled
; ip contains the exception flags (see fpe.s for a list)

__fp_exception
        ; fix to use correct __fp_status for softfp and hardfp
        IMPORT  __fp_status_arm
        STMFD   sp!, {a1, ip, lr}
        MOV     a1, ip, LSL #32-Except_pos-Except_len
        MOV     a1, a1, LSR #32 - Except_len
        MOV     a2, a1
        BL      __fp_status_arm
        LDMFD   sp!, {a3, ip, lr}
        MOV     a2, a1      ; previous setting
        MOV     a1, a3      ; sign of result

        TST     ip,#OVF_bit
        BNE     overflow
        TST     ip,#DVZ_bit
        BNE     divide_by_zero
        TST     ip, #UNF_bit
        BNE     underflow
        ;TST     ip,#IVO_bit
        ;BNE     invalid_operation

invalid_operation
        TST     a2,#IOE_bit                     ; check enabled
        BEQ     return_NaN
        ADR     a1,E_FP_IVO
        B       GenerateError
overflow
        TST     a2,#OFE_bit                     ; check enabled
        BEQ     return_Inf
        ADR     a1,E_FP_OFL
        B       GenerateError
underflow                                       ; already checked by _flt/dbl_underflow
        ADR     a1,E_FP_UFL
        B       GenerateError
divide_by_zero
        TST     a2,#DZE_bit                     ; check enabled
        BEQ     return_Inf
        ADR     a1,E_FP_DVZ
        ;B      GenerateError

GenerateError
    [ EMBEDDED_CLIB
        STMDB   sp!, {sp-pc}
        STMDB   sp!, {r0-r12}
        SUB     lr, lr, #4
        STR     lr, [sp, #15*4]
        MOV     r1, sp
    |
        LDR     r1,ErrBlock
        SUB     lr,lr,#4
        STR     lr,[r1,#15*4]
        MOV     lr,#&de00
        ORR     lr,lr,#&00ad
        ORR     lr,lr,lr,LSL #16
        STMIA   r1,{r0-r14}

        B       trap

ErrBlock
        IMPORT  __fp_errblock
        DCD     __fp_errblock

trap
    ]
        ; fallthrough
        IMPORT  __rt_trap, WEAK
        LDR     ip, =__rt_trap
        CMP     ip, #0
    [ INTERWORK :LOR: THUMB
        BXNE    ip
    |
        MOVNE   pc, ip
    ]
        DCD     0xe6000010

        ErrorBlock FP_IVO, "Floating Point Exception: Invalid Operation"
        ErrorBlock FP_OFL, "Floating Point Exception: Overflow"
        ErrorBlock FP_UFL, "Floating Point Exception: Underflow"
        ErrorBlock FP_DVZ, "Floating Point Exception: Divide By Zero"

return_Inf
        AND     a3,a1,#Sign_bit
        TST     ip,#Double_bit
        ADRNE   a1,prototype_double_Inf
        LDMNEIA a1,{a1,a2}
        LDREQ   a1,prototype_single_Inf
        ORR     a1,a1,a3
        B       __fp_return_Inf

return_NaN
        AND     a3, a1, #Sign_bit
        TST     ip, #Double_bit
        ADRNE   a1,prototype_double_NaN
        LDMNEIA a1,{a1,a2}
        LDREQ   a1,prototype_single_NaN
        ORR     a1, a1, a3
        B       __fp_return_NaN

prototype_double_Inf
        DCD     &7ff00000,&00000000
prototype_single_Inf
        DCD     &7f800000
prototype_double_NaN
        DCD     &7ff80000,&00000000
prototype_single_NaN
        DCD     &7fc00000

        ]

;------------------------------------------------------------------------------

        [ :DEF: fpdata_s

        AREA    |C$$data|,DATA

        EXPORT  __fp_status_flags
__fp_status_flags
        ; default - all supported flags enabled.
        DCD     (0x40 << 24) :OR: OFE_bit :OR: DZE_bit :OR: IOE_bit

        EXPORT  __fp_errblock
__fp_errblock
        DCD     0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0

        ]

;==============================================================================

        END
