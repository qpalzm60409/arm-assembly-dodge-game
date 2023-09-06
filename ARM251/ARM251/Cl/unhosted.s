;;; unhosted.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.18.4.1 $
;;; Checkin $Date: 1999/10/05 10:15:28 $
;;; Revising $Author: statham $

        GET     objmacs.s
        GET     h_errors.s

    [ THUMB
        CODE16
    ]

        MACRO
        LoadIPIfReentrant
   [ make = "shared-library"
        MOV     ip, sb          ; intra-link-unit entry
                                ; (sb gets preserved & restored only if needed)
   ]
        MEND

        CodeArea

; Support for compiler profiling options.

 [ make = "count" :LOR: make = "all" :LOR: make="shared-library"
        Function _count, leaf       ; used when profile option is enabled
        Function __rt_count, leaf

   [ THUMB
        !       1, "_count not implemented in 16 bit code"
   ]
   [ {CONFIG} = 26
        BIC     lr, lr, #&FC000003  ; remove condition code bits
   ]
        LDR     ip, [lr, #0]
        ADD     ip, ip, #1
        STR     ip, [lr, #0]
        ADD     pc, lr, #4          ; condition codes are preserved because
                                    ; nothing in this code changes them!

 ]

 [ make = "count1" :LOR: make = "all" :LOR: make="shared-library"
        Function _count1, leaf
        Function __rt_count1, leaf

   [ THUMB
        !       1, "_count1 not implemented in 16 bit code"
   ]
   [ {CONFIG} = 26
        BIC     lr, lr, #&FC000003  ; remove condition code bits
   ]
        LDR     ip, [lr, #0]
        ADD     ip, ip, #1
        STR     ip, [lr, #0]
        ADD     pc, lr, #8          ; condition codes are preserved because
                                    ; nothing in this code changes them!
 ]

 [ make = "rwcheck" :LOR: make = "ALL" :LOR: make="shared-library"
; Support for compiler option to check pointers before dereferencing.

        CODE_32

        IMPORT  |__rt_trap|, WEAK

        Function _rd1chk
        Function __rt_rd1chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     readfail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       readfail

        Function _rd2chk
        Function __rt_rd2chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     readfail
        TST     a1, #1
        BNE     readfail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       readfail

        Function _rd4chk
        Function __rt_rd4chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     readfail
        TST     a1, #3
        BNE     readfail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       readfail

        Function _wr1chk
        Function __rt_wr1chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     writefail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       writefail

        Function _wr2chk
        Function __rt_wr2chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     writefail
        TST     a1, #1
        BNE     writefail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC
        B       writefail

        Function _wr4chk
        Function __rt_wr4chk

        LoadIPIfReentrant
        CMPS    a1, #MemoryBase
        BLT     writefail
        TST     a1, #3
        BNE     writefail
        CMPS    a1, #MemoryLimit
        Return  , "", LinkNotStacked, CC

writefail
        Push    "r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15"
        ADR     r0, E_WriteFail
        B       fault

readfail
        Push    "r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15"
        ADR     r0, E_ReadFail
fault
        SUB     r14, r14, #4
        STR     r14, [sp, #15*4]
        MOV     r1, sp
        LDR     ip, =__rt_trap
        CMP     ip, #0
        [ INTERWORK:LOR:THUMB
        BXNE    ip
        |
        MOVNE   pc, ip
        ]
        DCD     0xe6000010
        ErrorBlock ReadFail, "Illegal read"
        ErrorBlock WriteFail, "Illegal write"

 ]

 [ make = "proccheck" :LOR: make = "all" :LOR: make="shared-library"

_proc_entry EnterWithLR_16
        ReturnToLR

_proc_exit EnterWithLR_16
        ReturnToLR
 ]

 [ make = "div0" :LOR: make = "all" :LOR: make="shared-library"

        IMPORT  |__rt_trap|
        IMPORT  |__rt_trap_32|

__rt_div0 EnterWithLR_16
        ; Dump all registers, then enter the abort code.
        ; r14 is a valid link.
   [ make = "shared-library"
        ; ip is our sb value
        Push    "sb,ip"                 ; caller's and our sb
        MOV     sb, ip                  ; can't access sb-relative data from ip
        LDR     ip, addr___rt_registerDump
        Pop     "sb"
   |
        LDR     ip, addr___rt_registerDump
   ]
        ; blocks of three registers only here for improved latency
        ; (performance fairly immaterial)
   [ LDM_MAX >= 15
        STMIA   ip, {r0-r14}
        ADD     ip, ip, #15*4
   |
        STMIA   ip!, {r0,r1,r2}
        STMIA   ip!, {r3,r4,r5}
        STMIA   ip!, {r6,r7,r8}
        STMIA   ip!, {r9,r10,r11}
        STMIA   ip!, {r12,sp,r14}
   ]
   [ make = "shared-library"
        Pop     "sb"
   ]
        ADR     r0, E_DivideByZero
        SUB     r1, ip, #pc*4
        SUB     r14, r14, #4
        STR     r14, [ip]
        B       __rt_trap_32

        ErrorBlock DivideByZero, "Divide by zero"
 ]

 [ make = "divtest" :LOR: make = "all" :LOR: make="shared-library"

        [ THUMB

        Function __16x$divtest
        ]

        IMPORT  __rt_div0
        Function x$divtest

__rt_divtest EnterWithLR_16
        LoadIPIfReentrant
        CMPS    a1, #0
        Return  , "", LinkNotStacked, NE
        B        __rt_div0

 ]

 [ make = "udiv10" :LOR: make = "all" :LOR: make="shared-library"
; Fast unsigned divide by 10: dividend in a1
; Returns quotient in a1, remainder in a2
;
; Calculate x / 10 as (x * 2**32/10) / 2**32.
; That is, we calculate the most significant word of the double-length
; product. In fact, we calculate an approximation which may be 1 off
; because we've ignored a carry from the least significant word we didn't
; calculate. We correct for this by insisting that the remainder < 10
; and by incrementing the quotient if it isn't.

        [ THUMB
        Function __16_kernel_udiv10, leaf
        ]
        Function _kernel_udiv10, leaf
    

__rt_udiv10 EnterWithLR_16
        SUB     a2, a1, #10
        SUB     a1, a1, a1, lsr #2
        ADD     a1, a1, a1, lsr #4
        ADD     a1, a1, a1, lsr #8
        ADD     a1, a1, a1, lsr #16
        MOV     a1, a1, lsr #3
        ADD     a3, a1, a1, asl #2
        SUBS    a2, a2, a3, asl #1
        ADDPL   a1, a1, #1
        ADDMI   a2, a2, #10
        ReturnToLR
 ]

 [ make = "sdiv10" :LOR: make = "all" :LOR: make="shared-library"
; Fast signed divide by 10: dividend in a1
; Returns quotient in a1, remainder in a2
; Quotient is truncated (rounded towards zero).

        [ THUMB
        Function __16_kernel_sdiv10, leaf
        ]
        Function _kernel_sdiv10, leaf

__rt_sdiv10 EnterWithLR_16
        MOVS    a4, a1
        RSBMI   a1, a1, #0

        SUB     a2, a1, #10         ; start of udiv10 code (verbatim)
        SUB     a1, a1, a1, lsr #2
        ADD     a1, a1, a1, lsr #4
        ADD     a1, a1, a1, lsr #8
        ADD     a1, a1, a1, lsr #16
        MOV     a1, a1, lsr #3
        ADD     a3, a1, a1, asl #2
        SUBS    a2, a2, a3, asl #1
        ADDPL   a1, a1, #1
        ADDMI   a2, a2, #10

        MOVS    a4, a4
        RSBMI   a1, a1, #0
        RSBMI   a2, a2, #0
        ReturnToLR

 ]

 [ make = "sdiv_rolled"
        [ THUMB
        IMPORT  |__16__rt_div0|

        Function __16x$divide

__rt_sdiv EnterWithLR_Thumb
        ASR     a4, a2, #31
        EOR     a2, a4
        SUB     a2, a4

        ASR     a3, a1, #31
        EOR     a1, a3
        SUB     a1, a3

        PUSH    {a3, a4}        ; Save so we can look at signs later on
        LSR     a4, a2, #1
        MOV     a3, a1
        BNE     s_loop1
        BL      |__16__rt_div0| ; Divide by zero

s_loop  LSL     a3, #1
s_loop1 CMP     a3, a4
        BLS     s_loop

        MOV     a4, #0
        CMP     a2, a3
        BCS     one_bit
zero_bit
        ADD     a4, a4
        CMP     a3, a1
        BEQ     return
s_loop2 LSR     a3, #1
        CMP     a2, a3
        BCC     zero_bit
one_bit
        ADC     a4, a4
        SUB     a2, a3
        CMP     a3, a1
        BNE     s_loop2

return
        MOV     a1, a4
        POP     {a3, a4}

        EOR     a3, a4
        EOR     a1, a3
        SUB     a1, a3

        EOR     a2, a4
        SUB     a2, a4

        ReturnToLR

        |

        IMPORT  |__rt_div0|

        Function x$divide
__rt_sdiv EnterWithLR
        LoadIPIfReentrant

; all-new signed divide entry sequence
; effectively zero a4 as top bit will be shifted out later
        ANDS    a4, a1, #&80000000
        RSBMI   a1, a1, #0
        EORS    ip, a4, a2, ASR #32
; ip bit 31 = sign of result
; ip bit 30 = sign of a2
        RSBCS   a2, a2, #0

; central part is identical code to udiv
; (without MOV a4, #0 which comes for free as part of signed entry sequence)
        MOVS    a3, a1
        BEQ     |__rt_div0|

s_loop
; justification stage shifts 1 bit at a time
        CMP     a3, a2, LSR #1
        MOVLS   a3, a3, LSL #1
; NB: LSL #1 is always OK if LS succeeds
        BLO     s_loop

s_loop2
        CMP     a2, a3
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3

        TEQ     a3, a1
        MOVNE   a3, a3, LSR #1
        BNE     s_loop2
        MOV     a1, a4

        MOVS    ip, ip, ASL #1
        RSBCS   a1, a1, #0
        RSBMI   a2, a2, #0

        ReturnToLR
        ]

 ]

 [ make = "udiv_rolled"
        [ THUMB
        IMPORT  |__16__rt_div0|

        Function __16x$udivide
__rt_udiv EnterWithLR_Thumb
        LSR     a4, a2, #1
        MOV     a3, a1
        BNE     u_loop
        BL      |__16__rt_div0|

u_loop1 LSL     a3, #1
u_loop  CMP     a3, a4
        BLS     u_loop1

        MOV     a4, #0
        CMP     a2, a3
        BCS     one_bit
zero_bit
        ADD     a4, a4
        CMP     a3, a1
        BEQ     return
u_loop2 LSR     a3, #1
        CMP     a2, a3
        BCC     zero_bit
one_bit
        ADC     a4, a4
        SUB     a2, a3
        CMP     a3, a1
        BNE     u_loop2

return
        MOV     a1, a4
        Return  , "", LinkNotStacked

        |

        IMPORT  |__rt_div0|
        Function x$udivide
__rt_udiv EnterWithLR_16

        LoadIPIfReentrant
; Unsigned divide of a2 by a1: returns quotient in a1, remainder in a2
; Destroys a3, a4

        MOV     a4, #0
        MOVS    a3, a1
        BEQ     |__rt_div0|

u_loop
; justification stage shifts 1 bit at a time
        CMP     a3, a2, LSR #1
        MOVLS   a3, a3, LSL #1
; NB: LSL #1 is always OK if LS succeeds
        BLO     u_loop

u_loop2
        CMP     a2, a3
        ADC     a4, a4, a4
        SUBCS   a2, a2, a3

; CMP sets carry so that loop-step MOV is executed if BNE u_loop2 taken
        TEQ     a3, a1
        MOVNE   a3, a3, LSR #1
        BNE     u_loop2
        MOV     a1, a4

        ReturnToLR
        ]

 ]

den         RN 0
num         RN 1
div         RN 2
sign    RN 3
tmp         RN 12

 [ make = "sdiv_unrolled8" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]

        Function x$divide
__rt_sdiv EnterWithLR_16

; Signed divide of a2 by a1: returns quotient in a1, remainder in a2
; Quotient is truncated (rounded towards zero).
; Sign of remainder = sign of dividend.
; Destroys a3, a4 and ip
; Negates dividend and divisor, then does an unsigned divide; signs
; get sorted out again at the end.
; Core code almost identical to udiv

; all-new signed divide entry sequence
; effectively zero a4 as top bit will be shifted out later

        ANDS    div, den, #1 << 31          ; high bit of div set - will be shifted out
        RSBMI   den, den, #0
        EORS    sign, div, num, ASR #32     ; CS -> negate num
        RSBCS   num, num, #0

        RSBS    tmp, den, num, LSR #3
        BLO     %F03
        RSBS    tmp, den, num, LSR #8
        BLO     %F01
        MOV     den, den, LSL #8
        ORR     div, div, #255 << 24
        RSBS    tmp, den, num, LSR #4
        BLO     %F02
        RSBS    tmp, den, num, LSR #8
        BLO     %F01
        MOV     den, den, LSL #8
        ORR     div, div, #255 << 16
        
        RSBS    tmp, den, num, LSR #8
        MOVCS   den, den, LSL #8
        ORRCS   div, div, #255 << 8        
        RSBS    tmp, den, num, LSR #4
        BLO     %F02
        RSBS    tmp, den, #0                        ; CS if den = 0
        BCS     __rt_div0
00
        MOVCS   den, den, LSR #8        
01
        RSBS    tmp, den, num, LSR #7
        SUBCS   num, num, den, LSL #7
        ADC     div, div, div
        RSBS    tmp, den, num, LSR #6
        SUBCS   num, num, den, LSL #6
        ADC     div, div, div
        RSBS    tmp, den, num, LSR #5
        SUBCS   num, num, den, LSL #5
        ADC     div, div, div
        RSBS    tmp, den, num, LSR #4
        SUBCS   num, num, den, LSL #4
        ADC     div, div, div
02
        RSBS    tmp, den, num, LSR #3
        SUBCS   num, num, den, LSL #3
        ADC     div, div, div
03        
        RSBS    tmp, den, num, LSR #2
        SUBCS   num, num, den, LSL #2
        ADC     div, div, div
        RSBS    tmp, den, num, LSR #1
        SUBCS   num, num, den, LSL #1
        ADC     div, div, div
        RSBS    tmp, den, num
        SUBCS   num, num, den
        ADCS    div, div, div                   ; CS if more bits to do
        BCS     %B00                        ; 27 cycle loop (StrongARM)

        EORS    den, div, sign, ASR #31     ; CS -> negate remainder
        ADD     den, den, sign, LSR #31     ; den = div * (sign & (1 << 31) ? -1 : 1)
        RSBCS   num, num, #0
        ReturnToLR

 ]

 [ make = "udiv_unrolled8" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]
        Function x$udivide
__rt_udiv EnterWithLR_16

        MOV     div, #0

        RSBS    tmp, den, num, LSR #3
        BLO     %F03
        RSBS    tmp, den, num, LSR #8
        BLO     %F01
        MOV     den, den, LSL #8
        ORR     div, div, #255 << 24
        RSBS    tmp, den, num, LSR #4
        BLO     %F02
        RSBS    tmp, den, num, LSR #8
        BLO     %F01
        MOV     den, den, LSL #8
        ORR     div, div, #255 << 16
        
        RSBS    tmp, den, num, LSR #8
        MOVCS   den, den, LSL #8
        ORRCS   div, div, #255 << 8        
        RSBS    tmp, den, num, LSR #4
        BLO     %F02
        RSBS    tmp, den, #0                        ; CS if den = 0
        BCS     __rt_div0
00
        MOVCS   den, den, LSR #8        
01
        RSBS    tmp, den, num, LSR #7
        SUBCS   num, num, den, LSL #7
        ADC     div, div, div
        RSBS    tmp, den, num, LSR #6
        SUBCS   num, num, den, LSL #6
        ADC     div, div, div
        RSBS    tmp, den, num, LSR #5
        SUBCS   num, num, den, LSL #5
        ADC     div, div, div
        RSBS    tmp, den, num, LSR #4
        SUBCS   num, num, den, LSL #4
        ADC     div, div, div
02
        RSBS    tmp, den, num, LSR #3
        SUBCS   num, num, den, LSL #3
        ADC     div, div, div
03        
        RSBS    tmp, den, num, LSR #2
        SUBCS   num, num, den, LSL #2
        ADC     div, div, div
        RSBS    tmp, den, num, LSR #1
        SUBCS   num, num, den, LSL #1
        ADC     div, div, div
        RSBS    tmp, den, num
        SUBCS   num, num, den
        ADCS    div, div, div                   ; CS if more bits to do
        BCS     %B00                        ; 27 cycle loop (StrongARM)

        MOV     den, div
        ReturnToLR

 ]

 [ make = "dspdiv64" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]

__rt_sdiv64by32 EnterWithLR_16

        LoadIPIfReentrant
; Constant time divide
; (64-bit) / (32-bit) = (32-bit) quotient and (32-bit) remainder
; 108 cycles + call and return

; Entry: r0 = MSW of dividend, r1 = LSW of dividend, r2 = divisor.
; Exit:  r0 = Remainder, r1 = Quotient, r2 unchanged.

        MOVS        r3, r2
        BEQ         |__rt_div0|               ; divide by zero handler
        RSBPL       r2, r2, #0                ; negate absolute value of divisor
        MOV         r3, r3, LSR #1            ; shift r1 sign down one bit
        EORS        r3, r3, r0, ASR #1        ; insert dividend sign and
        ; r1 bit 31 sign of dividend (= sign of remainder)
        ;    bit 30 sign of dividend EOR sign of divisor (= sign of quotient)
        BPL         %F01
        RSBS        r1, r1, #0                ; absolute value of dividend
        RSC         r0, r0, #0                ; absolute value of dividend
01
        ADDS        r1, r1, r1

        ADCS        r0, r2, r0, LSL #1        ; 31
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 30
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 29
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 28
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 27
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 26
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 25
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 24
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 23
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 22
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 21
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 20
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 19
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 18
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 17
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 16
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 15
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 14
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 13
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 12
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 11
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 10
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 9
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 8
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 7
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 6
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 5
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 4
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 3
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 2
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 1
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1
        ADCS        r0, r2, r0, LSL #1        ; 0
        RSBCC       r0, r2, r0
        ADCS        r1, r1, r1

        MOVS        r3, r3, ASL #1
        RSBMI       r1, r1, #0
        RSBCS       r0, r0, #0

        ReturnToLR
 ]

 [ make = "dspdiv32" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]

__rt_sdiv32by16 EnterWithLR_16

; Constant time divide
; (32-bit) / (16-bit) = (16-bit) quotient and (16-bit) remainder
; 44 cycles + call and return

; Entry: r0 = dividend, r1 = divisor.
; Exit:  r0 = remainder, r1 = quotient, r2 destroyed

        MOVS        r1, r1, LSL #16           ; shift the divisor to the top
        BEQ         |__rt_div0|               ; divide by zero handler
        MOV         r2, r1, LSR #1            ; shift r1 sign down one bit
        RSBPL       r1, r1, #0                ; negate absolute value of divisor
        EORS        r2, r2, r0, ASR #1        ; insert dividend sign and
        ; r1 bit 31 sign of dividend (= sign of remainder)
        ;    bit 30 sign of dividend EOR sign of divisor (= sign of quotient)
        RSBMI       r0, r0, #0                ; absolute value of dividend

        ADDS        r0, r1, r0, LSL #1        ; 15
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 14
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 13
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 12
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 11
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 10
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 9
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 8
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 7
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 6
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 5
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 4
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 3
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 2
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 1
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 0
        RSBCC       r0, r1, r0

        MOV         r1, r0, LSR #16           ; extract the remainder
        BIC         r0, r0, r1, LSL #16       ; and the partial quotient
        ADC         r0, r0, r0                ; insert the final quotient bit

        MOVS        r2, r2, ASL #1            ; put the signs back
        RSBMI       r0, r0, #0                ; quotient
        RSBCS       r1, r1, #0                ; remainder

        ReturnToLR
 ]

 [ make = "dspdiv32u" :LOR: make = "all" :LOR: make = "shared-library"
   [ :LNOT: (make = "all" :LOR: make = "shared-library")
        IMPORT  |__rt_div0|
   ]

__rt_udiv32by16 EnterWithLR_16

; Constant time divide
; (32-bit) / (16-bit) = (16-bit) quotient and (16-bit) remainder
; 38 cycles + call and return

; Entry: r0 = dividend, r1 = divisor.
; Exit:  r0 = remainder, r1 = quotient

        RSB         r1, r1, #0                ; negate divisor
        MOVS        r1, r1, LSL #15           ; shift the divisor to the top
        BEQ         |__rt_div0|               ; divide by zero handler

        ADDS        r0, r1, r0                ; 15
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 14
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 13
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 12
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 11
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 10
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 9
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 8
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 7
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 6
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 5
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 4
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 3
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 2
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 1
        RSBCC       r0, r1, r0
        ADCS        r0, r1, r0, LSL #1        ; 0
        RSBCC       r0, r1, r0

        ADC         r0, r0, r0                ; insert the final quotient bit
        MOV         r1, r0, LSR #16           ; extract the remainder
        BIC         r0, r0, r1, LSL #16       ; and the partial quotient

        ReturnToLR
 ]


        AdconTable

 [ make = "div0" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT  |__rt_registerDump|
addr___rt_registerDump
        &       |__rt_registerDump|
 ]
        END
