;;; longlong.s: support library for implementation of 64-bit integers
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.12.8.2 $
;;; Checkin $Date: 1999/10/26 12:16:26 $
;;; Revising $Author: wdijkstr $

        GET     objmacs.s

        CodeArea

 [ make = "_ll_neg" :LOR: make = "all" :LOR: make="shared-library"
_ll_neg EnterWithLR_16 
        RSBS    r0, r0, #0
        RSC     r1, r1, #0
        ReturnToLR
 ]

 [ make = "_ll_add" :LOR: make = "all" :LOR: make="shared-library"
_ll_add EnterWithLR_16 
        ADDS    r0, r0, r2
        ADC     r1, r1, r3
        ReturnToLR
 ]

 [ make = "_ll_addlu" :LOR: make = "all" :LOR: make="shared-library"
_ll_addlu EnterWithLR_16 
        ADDS    r0, r0, r2
        ADC     r1, r1, #0
        ReturnToLR
 ]

 [ make = "_ll_addls" :LOR: make = "all" :LOR: make="shared-library"
_ll_addls EnterWithLR_16 
        ADDS    r0, r0, r2
        ADC     r1, r1, r2, ASR #31
        ReturnToLR
 ]

 [ make = "_ll_adduu" :LOR: make = "all" :LOR: make="shared-library"
_ll_adduu EnterWithLR_16 
        MOV     r2, #0
        ADDS    r0, r0, r1
        ADC     r1, r2, #0
        ReturnToLR
 ]

 [ make = "_ll_addss" :LOR: make = "all" :LOR: make="shared-library"
_ll_addss EnterWithLR_16 
        MOV     r2, r0, ASR #31
        ADDS    r0, r0, r1
        ADC     r1, r2, r1, ASR #31
        ReturnToLR
 ]

 [ make = "_ll_sub" :LOR: make = "all" :LOR: make="shared-library"
_ll_sub EnterWithLR_16 
        SUBS    r0, r0, r2
        SBC     r1, r1, r3
        ReturnToLR
 ]

 [ make = "_ll_sublu" :LOR: make = "all" :LOR: make="shared-library"
_ll_sublu EnterWithLR_16 
        SUBS    r0, r0, r2
        SBC     r1, r1, #0
        ReturnToLR
 ]

 [ make = "_ll_subls" :LOR: make = "all" :LOR: make="shared-library"
_ll_subls EnterWithLR_16 
        SUBS    r0, r0, r2
        SBC     r1, r1, r2, ASR #31
        ReturnToLR
 ]

 [ make = "_ll_subuu" :LOR: make = "all" :LOR: make="shared-library"
_ll_subuu EnterWithLR_16 
        SUBS    r0, r0, r1
        SBC     r1, r1, r1
        ReturnToLR
 ]

 [ make = "_ll_subss" :LOR: make = "all" :LOR: make="shared-library"
_ll_subss EnterWithLR_16 
        MOV     r2, r0, ASR #31
        SUBS    r0, r0, r1
        SBC     r1, r2, r1, ASR #31
        ReturnToLR
 ]

 [ make = "_ll_rsb" :LOR: make = "all" :LOR: make="shared-library"
_ll_rsb EnterWithLR_16 
        RSBS    r0, r0, r2
        RSC     r1, r1, r3
        ReturnToLR
 ]

 [ make = "_ll_rsbls" :LOR: make = "all" :LOR: make="shared-library"
_ll_rsbls EnterWithLR_16 
        RSBS    r0, r0, r2
        RSC     r1, r1, r2, ASR #31
        ReturnToLR
 ]

 [ make = "_ll_rsblu" :LOR: make = "all" :LOR: make="shared-library"
_ll_rsblu EnterWithLR_16 
        RSBS    r0, r0, r2
        RSC     r1, r1, #0
        ReturnToLR
 ]

 [ make = "_ll_rsbss" :LOR: make = "all" :LOR: make="shared-library"
_ll_rsbss EnterWithLR_16 
        MOV     r2, r0, ASR #31
        RSBS    r0, r0, r1
        RSC     r1, r2, r1, ASR #31
        ReturnToLR
 ]

 [ make = "_ll_rsbuu" :LOR: make = "all" :LOR: make="shared-library"
_ll_rsbuu EnterWithLR_16 
        RSBS    r0, r0, r1
        RSC     r1, r1, r1
        ReturnToLR
 ]

 [ make = "_ll_mullu" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT _ll_mul

_ll_mullu EnterWithLR_16
        MOV     r3, #0
        B       _ll_mul
 ]

 [ make = "_ll_mulls" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT _ll_mul

_ll_mulls EnterWithLR_16
        MOV     r3, r2, ASR #31
        B       _ll_mul
 ]

 [ make = "_ll_muluu" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT  _ll_mul

_ll_muluu EnterWithLR_16 
        MOV     r3, #0
        MOV     r2, r1
        MOV     r1, #0
        B       _ll_mul
 ]

 [ make = "_ll_mulss" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT _ll_mul

_ll_mulss EnterWithLR_16 
        MOV     r3, r1, ASR #31
        MOV     r2, r1
        MOV     r1, r0, ASR #31
        B       _ll_mul
 ]

 [ make = "_ll_mul" :LOR: make = "all" :LOR: make="shared-library"

t0      RN      r4
t1      RN      r5
t2      RN      r6
        ; separate a 32 bit value into 2xunsigned 16 bit values
        ; resh and resl must be different
        MACRO
        USplit16 $rl, $rh, $x
        MOV     $rh, $x, LSR #16
        BIC     $rl, $x, $rh, LSL #16
        MEND

        ; unsigned 32x32=64 and dl,dh,x must be distinct
        ; dh.dl = x*y
        MACRO
        _UMULL  $dl, $dh, $x, $y
        ; no hardware multiplier
        ; extract y first - it may equal dl, dh or x
        USplit16        t0, t1, $y
        USplit16        t2, $dh, $x
        MUL               $dl, t0, t2       ; low x * low y
        MUL               t0, $dh, t0       ; high x * low y
        MUL               $dh, t1, $dh          ; high y * high x
        MUL               t1, t2, t1      ; low x * high y
        UAdd16  $dl, $dh, t0            ; add one middle value
        UAdd16  $dl, $dh, t1            ; add other middle value
        MEND

        ; add a 32 bit value to a 64 bit value, shifted up 16 unsigned
        MACRO
        UAdd16  $rl, $rh, $x
        ADDS      $rl, $rl, $x, LSL#16
        ADC         $rh, $rh, $x, LSR#16
        MEND

_ll_mul EnterWithLR_16
        FunctionEntry , "r4,r5,r6"
        MOV     lr, r0
        _UMULL  r0, ip, r2, lr
        MLA     r1, r2, r1, ip
        MLA     r1, r3, lr, r1
        Return  , "r4,r5,r6"
 ]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       64/64 DIVISION          ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; unsigned 64/64 = 64 with remainder 64
;
; n=numerator   d=denominator (each split into high and low part)
; q=quotient    r=remainder
;
; ie n/d = q + r/d or n=q*d+r and 0<=r<d
; if d=0 then it returns q=0, r=n (so n=q*d+r !)
; registers must be distinct
; n and d are corrupted
; t is a temporary register
;
; Routine is not unrolled since the speedup isn't great.
; Can unroll if you like.

        MACRO
        UDIV_64d64_64r64 $ql,$qh,$rl,$rh,$nl,$nh,$dl,$dh,$t
        MOV     $ql,#0          ; zero the quotient
        MOV     $qh,#0
        MOV     $rh,$nh         ; set the remainder to the current value
        MOV     $rl,$nl
        TEQ     $dh,#0
        TEQEQ   $dl,#0
        BEQ     %F08            ; divide by 0
        MOVS    $t,#0           ; count number of shifts
        ; first loop gets $d as large as possible
00
        ADDS    $dl, $dl, $dl
        ADCS    $dh, $dh, $dh   ; double d
        BCS     %F01            ; overflowed
        CMP     $dh, $rh
        CMPEQ   $dl, $rl
        ADDLS   $t, $t, #1      ; done an extra shift
        BLS     %B00
        ADDS    $t, $t, #0      ; clear carry
01                              ; carry the overflow here
        MOVS    $dh, $dh, RRX   ; colour
        MOV     $dl, $dl, RRX   ; shift back down again
02                              ; now main loop
        SUBS    $nl, $rl, $dl
        SBCS    $nh, $rh, $dh   ; n = r - d and C set if r>=d
        MOVCS   $rh, $nh
        MOVCS   $rl, $nl        ; r=r-d if this goes
        ADCS    $ql, $ql, $ql
        ADC     $qh, $qh, $qh   ; shift next bit into the answer
        MOVS    $dh, $dh, LSR#1
        MOV     $dl, $dl, RRX   ; shift down d
        SUBS    $t, $t, #1
        BGE     %B02            ; do next loop (t+1) loops
08
        MEND

; signed 64/64 with remainder 64
;
; n=numerator   d=denominator (each has a high and low part)
; q=quotient    r=remainder
; sign = an extra scratch register to store the signs in.
;
; ie n/d = q + r/d or n=q*d+r
; q is rounded towards zero and r has the same sign as n
; hence -3/2 = -1 remainder -1.
;       3/-2 = -1 remainder 1
;       -3/-2 = 1 remainder -1.
; if d=0 then it returns q=0, r=n (so n=q*d+r !)
; registers must be distinct

        MACRO
        SDIV_64d64_64r64 $ql,$qh,$rl,$rh,$nl,$nh,$dl,$dh,$t,$sign
        ANDS    $sign, $dh, #1<<31              ; get sign of d
        BPL     %F00
        RSBS    $dl, $dl, #0                    ; ensure d +ve
        RSC     $dh, $dh, #0
00
        EORS    $sign, $sign, $nh, ASR#32       ; b31=result b30=sign of n
        BCC     %F01
        RSBS    $nl, $nl, #0                    ; ensure n +ve
        RSC     $nh, $nh, #0
01
        UDIV_64d64_64r64 $ql,$qh,$rl,$rh,$nl,$nh,$dl,$dh,$t ; do the divide
        MOVS    $sign, $sign, LSL#1             ; get out sign bits
        BCC %F02
        RSBS    $ql, $ql, #0
        RSC     $qh, $qh, #0
02
        MOVS    $sign, $sign, LSL#1
        BCC %F03
        RSBS    $rl, $rl, #0                    ; negate remainder
        RSC     $rh, $rh, #0
03
        MEND

 [ make = "_ll_udiv" :LOR: make = "all" :LOR: make="shared-library"
        EXPORT __ll_udiv1

_ll_udiv EnterWithLR_16
        FunctionEntry ,"r4,r5,r6"
        MOV     r4, r0
        MOV     r5, r1
        MOV     r6, r2
        MOV     lr, r3
__ll_udiv1
        UDIV_64d64_64r64 r0,r1,r2,r3,r4,r5,r6,lr,ip
        Return  , "r4,r5,r6"
 ]

 [ make = "_ll_sdiv" :LOR: make = "all" :LOR: make="shared-library"
        EXPORT __ll_sdiv1

_ll_sdiv EnterWithLR_16
        FunctionEntry ,"r4,r5,r6,r7"
        MOV     r4, r0
        MOV     r5, r1
        MOV     r6, r2
        MOV     r7, r3
__ll_sdiv1
        SDIV_64d64_64r64 r0,r1,r2,r3,r4,r5,r6,r7,lr,ip
        Return  , "r4,r5,r6,r7"
 ]

 [ make = "_ll_urdv" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT __ll_udiv1

_ll_urdv EnterWithLR_16
        FunctionEntry ,"r4,r5,r6"
        MOV     r4, r2
        MOV     r5, r3
        MOV     r6, r0
        MOV     lr, r1
        B       __ll_udiv1
 ]

 [ make = "_ll_srdv" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT __ll_sdiv1

_ll_srdv EnterWithLR_16
        FunctionEntry ,"r4,r5,r6,r7"
        MOV     r4, r2
        MOV     r5, r3
        MOV     r6, r0
        MOV     r7, r1
        B       __ll_sdiv1
 ]

 [ make = "_ll_not" :LOR: make = "all" :LOR: make="shared-library"
_ll_not EnterWithLR_16
        MVN     r0, r0
        MVN     r1, r1
        ReturnToLR
 ]

 [ make = "_ll_and" :LOR: make = "all" :LOR: make="shared-library"
_ll_and EnterWithLR_16
        AND     r0, r0, r2
        AND     r1, r1, r3
        ReturnToLR
 ]

 [ make = "_ll_or" :LOR: make = "all" :LOR: make="shared-library"
_ll_or EnterWithLR_16
        ORR     r0, r0, r2
        ORR     r1, r1, r3
        ReturnToLR
 ]

 [ make = "_ll_eor" :LOR: make = "all" :LOR: make="shared-library"
_ll_eor EnterWithLR_16
        EOR     r0, r0, r2
        EOR     r1, r1, r3
        ReturnToLR
 ]

 [ make = "_ll_shift_l" :LOR: make = "all" :LOR: make="shared-library"
_ll_shift_l EnterWithLR_16
        SUBS    r3, r2, #32
        BPL     %F01
        RSB     r3, r2, #32
        MOV     r1, r1, ASL r2
        ORR     r1, r1, r0, LSR r3
        MOV     r0, r0, ASL r2
        ReturnToLR

01      MOV     r1, r0, ASL r3
        MOV     r0, #0
        ReturnToLR
 ]

 [ make = "_ll_ushift_r" :LOR: make = "all" :LOR: make="shared-library"
_ll_ushift_r EnterWithLR_16
        SUBS    r3, r2, #32
        BPL     %F01
        RSB     r3, r2, #32
        MOV     r0, r0, LSR r2
        ORR     r0, r0, r1, ASL r3
        MOV     r1, r1, LSR r2
        ReturnToLR

01      MOV     r0, r1, LSR r3
        MOV     r1, #0
        ReturnToLR
 ]

 [ make = "_ll_sshift_r" :LOR: make = "all" :LOR: make="shared-library"
_ll_sshift_r EnterWithLR_16
        SUBS    r3, r2, #32
        BPL     %F01
        RSB     r3, r2, #32
        MOV     r0, r0, LSR r2
        ORR     r0, r0, r1, ASL r3
        MOV     r1, r1, ASR r2
        ReturnToLR

01      MOV     r0, r1, ASR r3
        MOV     r1, r1, ASR #31
        ReturnToLR
 ]

 [ make = "_ll_cmpeq" :LOR: make = "all" :LOR: make="shared-library"
_ll_cmpeq EnterWithLR_16
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVEQ   r0, #1
        MOVNE   r0, #0
        ReturnToLR
 ]

 [ make = "_ll_cmpne" :LOR: make = "all" :LOR: make="shared-library"
_ll_cmpne EnterWithLR_16
        SUBS    r0, r0, r2
        CMPEQ   r1, r3
        MOVNE   r0, #1
        ReturnToLR
 ]

 [ make = "_ll_ucmpgt" :LOR: make = "all" :LOR: make="shared-library"
_ll_ucmpgt EnterWithLR_16
; could be faster
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVHI   r0, #1
        MOVLS   r0, #0
        ReturnToLR
 ]

 [ make = "_ll_ucmpge" :LOR: make = "all" :LOR: make="shared-library"
_ll_ucmpge EnterWithLR_16
; could be faster
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVHS   r0, #1
        MOVLO   r0, #0
        ReturnToLR
 ]

 [ make = "_ll_ucmplt" :LOR: make = "all" :LOR: make="shared-library"
_ll_ucmplt EnterWithLR_16
; could be faster
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVLO   r0, #1
        MOVHS   r0, #0
        ReturnToLR
 ]

 [ make = "_ll_ucmple" :LOR: make = "all" :LOR: make="shared-library"
_ll_ucmple EnterWithLR_16
; could be faster
        CMP     r1, r3
        CMPEQ   r0, r2
        MOVLS   r0, #1
        MOVHI   r0, #0
        ReturnToLR
 ]

 [ make = "_ll_scmpgt" :LOR: make = "all" :LOR: make="shared-library"
_ll_scmpgt EnterWithLR_16
; could be faster
        CMP     r1, r3
        MOVGT   r0, #1
        MOVLT   r0, #0
        ReturnToLR NE
        CMP     r0, r2
        MOVHI   r0, #1
        MOVLS   r0, #0
        ReturnToLR
 ]

 [ make = "_ll_scmpge" :LOR: make = "all" :LOR: make="shared-library"
_ll_scmpge EnterWithLR_16
; could be faster
        CMP     r1, r3
        MOVGT   r0, #1
        MOVLT   r0, #0
        ReturnToLR NE
        CMP     r0, r2
        MOVHS   r0, #1
        MOVLO   r0, #0
        ReturnToLR
 ]

 [ make = "_ll_scmplt" :LOR: make = "all" :LOR: make="shared-library"
_ll_scmplt EnterWithLR_16
; could be faster
        CMP     r1, r3
        MOVLT   r0, #1
        MOVGT   r0, #0
        ReturnToLR NE
        CMP     r0, r2
        MOVLO   r0, #1
        MOVHS   r0, #0
        ReturnToLR
 ]

 [ make = "_ll_scmple" :LOR: make = "all" :LOR: make="shared-library"
_ll_scmple EnterWithLR_16
; could be faster
        CMP     r1, r3
        MOVLT   r0, #1
        MOVGT   r0, #0
        ReturnToLR NE
        CMP     r0, r2
        MOVLS   r0, #1
        MOVHI   r0, #0
        ReturnToLR
 ]


 [ make = "_ll_from_l" :LOR: make = "all" :LOR: make="shared-library"
_ll_from_l EnterWithLR_16
        MOV     r1, r0, ASR #31
        ReturnToLR
 ]

 [ make = "_ll_from_u" :LOR: make = "all" :LOR: make="shared-library"
_ll_from_u EnterWithLR_16
        MOV     r1, #0
        ReturnToLR
 ]

 [ make = "_ll_to_l" :LOR: make = "all" :LOR: make="shared-library"
_ll_to_l EnterWithLR_16
        ReturnToLR
 ]

        END


