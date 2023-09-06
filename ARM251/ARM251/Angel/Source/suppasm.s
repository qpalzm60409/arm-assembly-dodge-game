        TTL     Angel assembler support routines                    > suppasm.s
        ; ---------------------------------------------------------------------
        ; This source files holds the general assembler routines
        ; needed by Angel.
        ;
        ; $Revision: 1.26.2.2 $
        ;   $Author: rivimey $
        ;     $Date: 1998/10/15 17:13:00 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995, 1997, 1998.
        ; All Rights Reserved
        ;
        ; ---------------------------------------------------------------------

        GET     listopts.s              ; standard listing control
        GET     lolevel.s               ; generic ARM definitions
        GET     macros.s                ; standard assembler support
        GET     target.s                ; target specific definitions
        GET     taskmacs.s              ; exception/task mgmt support

        ; ---------------------------------------------------------------------

        AREA    |C$$Code$$LibrarySupport|,CODE,READONLY
        KEEP
        
        IMPORT  Angel_StackBase

        ; Angel is designed *NOT* to require any of the standard ANSI
        ; 'C' library, since it lives beneath the library.  This means
        ; that we need to provide our own versions of certain standard
        ; routines:

        EXPORT __rt_memcpy
        GBLS  SLA                       ; shift towards low address end
        GBLS  SHA                       ; shift towards high address end
 [ {ENDIAN} = "big"
SLA     SETS "LSL"
SHA     SETS "LSR"
 |                                      ; assume little-endian
SLA     SETS "LSR"
SHA     SETS "LSL"
 ]

 [ {TRUE}       ; new, fast memcpy

        GET objmacs.s

src     RN    a2
dst     RN    a1
n       RN    a3
tmp1    RN    a4
tmp3    RN    ip

        Function __rt_memcpy, leaf

        CMP     src, dst
        BLO     CopyDown
        Return  , "", LinkNotStacked, EQ ; dst == src, no move required

        FunctionEntry , "dst"           ; Must return original dst.
        SUBS    n, n, #4                ; need at least 4 bytes
        BLT     Up_TrailingBytes        ; < 4 bytes to go

        ; word align the dst - first find out how many bytes
        ; must be stored to do this.  If the number is 0
        ; check the src too.

        ANDS    tmp3, dst, #3           ; eq means aligned!
        BNE     Up_AlignDst
        ANDS    tmp3, src, #3
        BNE     Up_SrcUnaligned         ; more difficult!

        ; here when source and destination are both aligned.
        ; number of bytes to transfer is (n+4), n is >= 0.

Up_SrcDstAligned
        SUBS    n, n, #12-4             ; 12 bytes or more?
        BLT     Up_TrailingWords
        ; We only have three registers to play with.  It is
        ; worth gaining more only if the number of bytes to
        ; transfer is greater than 12+8*<registers stacked>
        ; We need to stack 8 (4+4) registers to gain 8 temporaries,
        ; so look for >=44 bytes.  Since we would save 8*4 = 32
        ; bytes at a time we actually compare with 64.

        SUBS    n, n, #32-12            ; n+32 to go.
        BLT     %F1

        STMFD   sp!, {v1}

        ; loop loading 4 registers per time, twice (32 bytes)

0       LDMIA   src!, {tmp1, v1, tmp3, lr}
        STMIA   dst!, {tmp1, v1, tmp3, lr}
        LDMIA   src!, {tmp1, v1, tmp3, lr}
        STMIA   dst!, {tmp1, v1, tmp3, lr}
        SUBS    n, n, #32
        BGE     %B0
        ; see if we can handle another 8

        CMN     n, #16
        LDMGEIA src!, {tmp1, v1, tmp3, lr}
        STMGEIA dst!, {tmp1, v1, tmp3, lr}
        SUBGE   n, n, #16

        ; Reload the registers - note that we still have (n+32)
        ; bytes to go, and that this is <16.

        LDMFD   sp!, {v1}

        ; Here when there are fewer than 16 bytes to go.

1       ADDS    n, n, #32-12               ; (n-12) to go

        ; Ok - do three words at a time.

2       LDMGEIA src!, {tmp1, tmp3, lr}
        STMGEIA dst!, {tmp1, tmp3, lr}
        SUBGES  n, n, #12
        BGE     %B2
        ; (n-12) bytes to go - 0, 1 or 2 words.  Check
        ; which.

Up_TrailingWords
        ADDS    n, n, #12-4             ; (n-4) to go
        BLT     Up_TrailingBytes        ; < 4 bytes to go
        SUBS    n, n, #4
        LDRLT   tmp1, [src], #4
        STRLT   tmp1, [dst], #4
        LDMGEIA src!, {tmp1, tmp3}
        STMGEIA dst!, {tmp1, tmp3}
        SUBGE   n, n, #4

        ; Here with less than 4 bytes to go

Up_TrailingBytes
        ADDS    n, n, #4
        Return  , "a1", , EQ            ; 0 bytes
        CMP     n, #2                   ; 1, 2 or 3 bytes
        LDRB    tmp1, [src], #1
        STRB    tmp1, [dst], #1
        LDRGEB  tmp1, [src], #1
        STRGEB  tmp1, [dst], #1
        LDRGTB  tmp1, [src], #1
        STRGTB  tmp1, [dst], #1
        Return  , "a1"                  ; recover old dst value

;------------------------------------------------------------

; word align dst - tmp3 contains current destination
; alignment.  We can store at least 4 bytes here.

Up_AlignDst
        RSB     tmp3, tmp3, #4          ; 1-3 bytes to go
        CMP     tmp3, #2
        LDRB    tmp1, [src], #1
        STRB    tmp1, [dst], #1
        LDRGEB  tmp1, [src], #1
        STRGEB  tmp1, [dst], #1
        LDRGTB  tmp1, [src], #1
        STRGTB  tmp1, [dst], #1
        SUBS    n, n, tmp3              ; check number to go
        BLT     Up_TrailingBytes        ; less than 4 bytes
        ANDS    tmp3, src, #3
        BEQ     Up_SrcDstAligned        ; coaligned case

        ; The source is not coaligned with the destination,
        ; the destination IS currently word aligned.

Up_SrcUnaligned
        BIC     src, src, #3            ; tmp3 holds extra!
        LDR     lr, [src], #4           ; 1-3 useful bytes
        CMP     tmp3, #2
        BGT     Up_OneByte              ; one byte in tmp1
        BEQ     Up_TwoBytes             ; two bytes in tmp1

; The next three source bytes are in tmp1, one byte must
; come from the next source word.  At least four bytes
; more must be stored.  Check first to see if there are a
; sufficient number of bytes to go to justify using stm/ldm
; instructions.

Up_ThreeBytes
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no                    ; 1
        SUB     n, n, #16-4             ; (n+16) bytes to go    ; 1

        ; save some work registers.  The point at which this
        ; is done is based on the ldm/stm time being = (n+3)+(n/4)S

        STMFD   sp!, {v1, v2}                                  ; 14   ????

        ; loop doing 16 bytes at a time.  There are currently
        ; three useful bytes in lr.

0       MOV     tmp1, lr, $SLA #8        ; first three bytes     ; 1
        LDMIA   src!, {v1, v2, tmp3, lr}                         ; 12/13
        ORR     tmp1, tmp1, v1, $SHA #24         ; word 1        ; 1
        MOV     v1, v1, $SLA #8                                  ; ...
        ORR     v1, v1, v2, $SHA #24             ; word 2        ; 2 (1+1)
        MOV     v2, v2, $SLA #8
        ORR     v2, v2, tmp3, $SHA #24           ; word 3        ; 2
        MOV     tmp3, tmp3, $SLA #8
        ORR     tmp3, tmp3, lr, $SHA #24         ; word 4        ; 2
        STMIA   dst!, {tmp1, v1, v2, tmp3}                       ; 12/13
        SUBS    n, n, #16                                        ; 1
        BGE     %B0                                              ; 4 / 1

        ; loop timing (depends on alignment) for n loops:-

        ;       pre:    17
        ;               ((45/46/47)n - 3) for 32n bytes
        ;       post:   13/14
        ;       total:  (45/46/47)n+(27/28)
        ;       32 bytes:       72-75
        ;       64 bytes:       117-122
        ;       96 bytes:       162-169

        ; Reload registers

        LDMFD   sp!, {v1, v2}                                   ; 12/13 ????

        ADDS    n, n, #16-4              ; check for at least 4
        BLT     %F2                      ; < 4 bytes

1       MOV     tmp3, lr, $SLA #8        ; first three bytes     ; 1
        LDR     lr, [src], #4            ; next four bytes       ; 4
        ORR     tmp3, tmp3, lr, $SHA #24                         ; 1
        STR     tmp3, [dst], #4                                  ; 4
        SUBS    n, n, #4                                         ; 1
        BGE     %B1                      ; tmp1 contains three bytes 1 / 4

        ; Loop timing:

        ;               15n-3   for 4n bytes
        ;       32:     117
        ;       64:     237

        ; Less than four bytes to go - readjust the src
        ; address.

2       SUB     src, src, #3
        B       Up_TrailingBytes

; The next two source bytes are in tmp1, two bytes must
; come from the next source word.  At least four bytes
; more must be stored.

Up_TwoBytes
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2}

        ; loop doing 32 bytes at a time.  There are currently
        ; two useful bytes in lr.

0       MOV     tmp1, lr, $SLA #16       ; first two bytes
        LDMIA   src!, {v1, v2, tmp3, lr}
        ORR     tmp1, tmp1, v1, $SHA #16 ; word 1
        MOV     v1, v1, $SLA #16
        ORR     v1, v1, v2, $SHA #16     ; word 2
        MOV     v2, v2, $SLA #16
        ORR     v2, v2, tmp3, $SHA #16   ; word 3
        MOV     tmp3, tmp3, $SLA #16
        ORR     tmp3, tmp3, lr, $SHA #16 ; word 4
        STMIA   dst!, {tmp1, v1, v2, tmp3}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2}

        ADDS    n, n, #16-4              ; check number of bytes
        BLT     %F2
1       MOV     tmp3, lr, $SLA #16       ; first two bytes
        LDR     lr, [src], #4            ; next four bytes
        ORR     tmp3, tmp3, lr, $SHA #16
        STR     tmp3, [dst], #4
        SUBS    n, n, #4
        BGE     %B1                      ; tmp1 contains two bytes

        ; Less than four bytes to go - readjust the src
        ; address.

2       SUB     src, src, #2
        B       Up_TrailingBytes

; The next source byte is in tmp1, three bytes must
; come from the next source word.  At least four bytes
; more must be stored.

Up_OneByte
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2}

        ; loop doing 32 bytes at a time.  There is currently
        ; one useful byte in lr

0       MOV     tmp1, lr, $SLA #24       ; first byte
        LDMIA   src!, {v1, v2, tmp3, lr}
        ORR     tmp1, tmp1, v1, $SHA #8  ; word 1
        MOV     v1, v1, $SLA #24
        ORR     v1, v1, v2, $SHA #8      ; word 2
        MOV     v2, v2, $SLA #24
        ORR     v2, v2, tmp3, $SHA #8    ; word 3
        MOV     tmp3, tmp3, $SLA #24
        ORR     tmp3, tmp3, lr, $SHA #8  ; word 4
        STMIA   dst!, {tmp1, v1, v2, tmp3}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2}

        ADDS    n, n, #16-4              ; check number of bytes
        BLT     %F2
1       MOV     tmp3, lr, $SLA #24       ; first byte
        LDR     lr, [src], #4            ; next four bytes
        ORR     tmp3, tmp3, lr, $SHA #8
        STR     tmp3, [dst], #4
        SUBS    n, n, #4
        BGE     %B1                      ; tmp1 contains one byte

        ; Less than four bytes to go - one already in tmp3.

2       SUB     src, src, #1
        B       Up_TrailingBytes

;======================================================================
; Copy down code
; ==============

;       This is exactly the same as the copy up code -
;       but it copies in the opposite direction.

CopyDown
        ADD     src, src, n             ; points beyond end
        ADD     dst, dst, n

        SUBS    n, n, #4                ; need at least 4 bytes
        BLT     Down_TrailingBytes      ; < 4 bytes to go

        ; word align the dst - first find out how many bytes
        ; must be stored to do this.  If the number is 0
        ; check the src too.

        ANDS    tmp3, dst, #3           ; eq means aligned!
        BNE     Down_AlignDst
        ANDS    tmp3, src, #3
        BNE     Down_SrcUnaligned       ; more difficult!

        ; here when source and destination are both aligned.
        ; number of bytes to transfer is (n+4), n is >= 0.

Down_SrcDstAligned
        SUBS    n, n, #12-4             ; 12 bytes or more?
        BLT     Down_TrailingWords
        ; We only have three registers to play with.  It is
        ; worth gaining more only if the number of bytes to
        ; transfer is greater than 12+8*<registers stacked>
        ; We need to stack 8 (4+4) registers to gain 8 temporaries,
        ; so look for >=44 bytes.  Since we would save 8*4 = 32
        ; bytes at a time we actually compare with 64.

        STMFD   sp!, {v1, lr}
        SUBS    n, n, #32-12            ; n+32 to go.
        BLT     %F1

        ; loop loading 4 registers per time, twice (32 bytes)

0       LDMDB   src!, {tmp1, v1, tmp3, lr}
        STMDB   dst!, {tmp1, v1, tmp3, lr}
        LDMDB   src!, {tmp1, v1, tmp3, lr}
        STMDB   dst!, {tmp1, v1, tmp3, lr}
        SUBS    n, n, #32
        BGE     %B0
        ; see if we can handle another 8

1       CMN     n, #16
        LDMGEDB src!, {tmp1, v1, tmp3, lr}
        STMGEDB dst!, {tmp1, v1, tmp3, lr}
        SUBGE   n, n, #16

        ; Here when there are fewer than 16 bytes to go.

        ADDS    n, n, #32-12            ; (n-12) to go

        ; Ok - do three words at a time.

        LDMGEDB src!, {tmp1, tmp3, lr}
        STMGEDB dst!, {tmp1, tmp3, lr}
        SUBGE   n, n, #12
        LDMFD   sp!, {v1, lr}
        ; (n-12) bytes to go - 0, 1 or 2 words.  Check
        ; which.

Down_TrailingWords
        ADDS    n, n, #12-4             ; (n-4) to go
        BLT     Down_TrailingBytes      ; < 4 bytes to go
        SUBS    n, n, #4
        LDRLT   tmp1, [src, #-4]!
        STRLT   tmp1, [dst, #-4]!
        LDMGEDB src!, {tmp1, tmp3}
        STMGEDB dst!, {tmp1, tmp3}
        SUBGE   n, n, #4

        ; Here with less than 4 bytes to go

Down_TrailingBytes
        ADDS    n, n, #4
        Return  , "", LinkNotStacked, EQ ; 0 bytes
        CMP     n, #2                    ; 1, 2 or 3 bytes
        LDRB    tmp1, [src, #-1]!
        STRB    tmp1, [dst, #-1]!
        LDRGEB  tmp1, [src, #-1]!
        STRGEB  tmp1, [dst, #-1]!
        LDRGTB  tmp1, [src, #-1]!        ; dst is now original dst
        STRGTB  tmp1, [dst, #-1]!
        Return  , "", LinkNotStacked

;------------------------------------------------------------

; word align dst - tmp3 contains current destination
; alignment.  We can store at least 4 bytes here.  We are
; going downwards - so tmp3 is the actual number of bytes
; to store.

Down_AlignDst
        CMP     tmp3, #2
        LDRB    tmp1, [src, #-1]!
        STRB    tmp1, [dst, #-1]!
        LDRGEB  tmp1, [src, #-1]!
        STRGEB  tmp1, [dst, #-1]!
        LDRGTB  tmp1, [src, #-1]!
        STRGTB  tmp1, [dst, #-1]!
        SUBS    n, n, tmp3              ; check number to go
        BLT     Down_TrailingBytes      ; less than 4 bytes
        ANDS    tmp3, src, #3
        BEQ     Down_SrcDstAligned      ; coaligned case

        ; The source is not coaligned with the destination,
        ; the destination IS currently word aligned.

Down_SrcUnaligned
        BIC     src, src, #3            ; tmp3 holds extra!
        LDR     tmp1, [src]             ; 1-3 useful bytes
        CMP     tmp3, #2
        BLT     Down_OneByte            ; one byte in tmp1
        BEQ     Down_TwoBytes           ; two bytes in tmp1

; The last three source bytes are in tmp1, one byte must
; come from the previous source word.  At least four bytes
; more must be stored.  Check first to see if there are a
; sufficient number of bytes to go to justify using stm/ldm
; instructions.

Down_ThreeBytes
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2, lr}

        ; loop doing 32 bytes at a time.  There are currently
        ; three useful bytes in tmp1 (a4).

0       MOV     lr, tmp1, $SHA #8        ; last three bytes
        LDMDB   src!, {tmp1, v1, v2, tmp3}
        ORR     lr, lr, tmp3, $SLA #24   ; word 4
        MOV     tmp3, tmp3, $SHA #8
        ORR     tmp3, tmp3, v2, $SLA #24 ; word 3
        MOV     v2, v2, $SHA #8
        ORR     v2, v2, v1, $SLA #24     ; word 2
        MOV     v1, v1, $SHA #8
        ORR     v1, v1, tmp1, $SLA #24   ; word 1
        STMDB   dst!, {v1, v2, tmp3, lr}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2, lr}

        ADDS    n, n, #16-4              ; check for at least 4
        BLT     %F2                      ; < 4 bytes

1       MOV     tmp3, tmp1, $SHA #8      ; last three bytes
        LDR     tmp1, [src, #-4]!        ; previous four bytes
        ORR     tmp3, tmp3, tmp1, $SLA #24
        STR     tmp3, [dst, #-4]!
        SUBS    n, n, #4
        BGE     %B1                      ; tmp1 contains three bytes

        ; Less than four bytes to go - readjust the src
        ; address.

2       ADD     src, src, #3
        B       Down_TrailingBytes

; The last two source bytes are in tmp1, two bytes must
; come from the previous source word.  At least four bytes
; more must be stored.

Down_TwoBytes
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2, lr}

        ; loop doing 32 bytes at a time.  There are currently
        ; two useful bytes in tmp1 (a4).

0       MOV     lr, tmp1, $SHA #16       ; last two bytes
        LDMDB   src!, {tmp1, v1, v2, tmp3}
        ORR     lr, lr, tmp3, $SLA #16   ; word 4
        MOV     tmp3, tmp3, $SHA #16
        ORR     tmp3, tmp3, v2, $SLA #16 ; word 3
        MOV     v2, v2, $SHA #16
        ORR     v2, v2, v1, $SLA #16     ; word 2
        MOV     v1, v1, $SHA #16
        ORR     v1, v1, tmp1, $SLA #16   ; word 1
        STMDB   dst!, {v1, v2, tmp3, lr}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2, lr}

        ADDS    n, n, #16-4              ; check for at least 4
        BLT     %F2                      ; < 4 bytes

1       MOV     tmp3, tmp1, $SHA #16     ; last two bytes
        LDR     tmp1, [src, #-4]!        ; previous four bytes
        ORR     tmp3, tmp3, tmp1, $SLA #16
        STR     tmp3, [dst, #-4]!
        SUBS    n, n, #4
        BGE     %B1                      ; tmp1 contains two bytes

        ; Less than four bytes to go - readjust the src
        ; address.

2       ADD     src, src, #2
        B       Down_TrailingBytes

; The last source byte is in tmp1, three bytes must
; come from the previous source word.  At least four bytes
; more must be stored.

Down_OneByte
        CMP     n, #16-4                ; at least 16 bytes?
        BLT     %F1                     ; no
        SUB     n, n, #16-4             ; (n+16) bytes to go

        ; form a stack frame and save registers

        STMFD   sp!, {v1, v2, lr}

        ; loop doing 32 bytes at a time.  There is currently
        ; one useful byte in tmp1 (a4).

0       MOV     lr, tmp1, $SHA #24       ; last byte
        LDMDB   src!, {tmp1, v1, v2, tmp3}
        ORR     lr, lr, tmp3, $SLA #8    ; word 4
        MOV     tmp3, tmp3, $SHA #24
        ORR     tmp3, tmp3, v2, $SLA #8  ; word 3
        MOV     v2, v2, $SHA #24
        ORR     v2, v2, v1, $SLA #8      ; word 2
        MOV     v1, v1, $SHA #24
        ORR     v1, v1, tmp1, $SLA #8    ; word 1
        STMDB   dst!, {v1, v2, tmp3, lr}
        SUBS    n, n, #16
        BGE     %B0
        ; Reload registers

        LDMFD   sp!, {v1, v2, lr}

        ADDS    n, n, #16-4               ; check for at least 4
        BLT     %F2                       ; < 4 bytes

1       MOV     tmp3, tmp1, $SHA #24      ; last byte
        LDR     tmp1, [src, #-4]!         ; previous four bytes
        ORR     tmp3, tmp3, tmp1, $SLA #8
        STR     tmp3, [dst, #-4]!
        SUBS    n, n, #4
        BGE     %B1                       ; tmp1 contains one byte

        ; Less than four bytes to go - one already in tmp3.

2       ADD     src, src, #1
        B       Down_TrailingBytes

 |      ; old, slow memcpy

__rt_memcpy
        ; a1 = dst address
        ; a2 = src address
        ; a3 = byte count.
        ;
        ; A very slow version of memcpy. Its only merit is that it is quite
        ; small.
        ;
        CMP     a3,#0
        MOVEQ   pc,lr
        CMP     a2,a1
        BLO     CopyDown
        MOVEQ   pc,lr

        MOV     ip,a1
CopyUpLoop
        LDRB    a4,[a2],#1
        STRB    a4,[ip],#1
        SUBS    a3,a3,#1
        BNE     CopyUpLoop

memcpyExit
        MOV     pc,lr

CopyDown
        ADD     a2,a2,a3
        ADD     a1,a1,a3
CopyDownLoop
        LDRB    a4,[a2,#-1]!
        STRB    a4,[a1,#-1]!
        SUBS    a3,a3,#1
        BNE     CopyDownLoop
        BAL     memcpyExit

 ]

        EXPORT __rt_udiv
__rt_udiv
        ; Signed divide of a2 by a1, returns quotient in a1, remainder in a2
        ; Unsigned divide of a2 by a1: returns quotient in a1, remainder in a2
        ; Destroys a3, a4 and ip
        MOVS    a3,a1
        BEQ     __division_by_zero
        MOV     a4,#0
        MOV     ip,#0x80000000
        CMP     a2,# ip
        MOVLO   ip,a2
01
        CMP     ip,a3,ASL #0
        BLS     u_shifted0mod8
        CMP     ip,a3,ASL #1
        BLS     u_shifted1mod8
        CMP     ip,a3,ASL #2
        BLS     u_shifted2mod8
        CMP     ip,a3,ASL #3
        BLS     u_shifted3mod8
        CMP     ip,a3,ASL #4
        BLS     u_shifted4mod8
        CMP     ip,a3,ASL #5
        BLS     u_shifted5mod8
        CMP     ip,a3,ASL #6
        BLS     u_shifted6mod8
        CMP     ip,a3,ASL #7
        MOVHI   a3,a3,ASL #8
        BHI     %BT01
02
u_shifted7mod8
        CMP     a2,a3,ASL #7
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #7
u_shifted6mod8
        CMP     a2,a3,ASL #6
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #6
u_shifted5mod8
        CMP     a2,a3,ASL #5
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #5
u_shifted4mod8
        CMP     a2,a3,ASL #4
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #4
u_shifted3mod8
        CMP     a2,a3,ASL #3
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #3
u_shifted2mod8
        CMP     a2,a3,ASL #2
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #2
u_shifted1mod8
        CMP     a2,a3,ASL #1
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #1
u_shifted0mod8
        CMP     a2,a3,ASL #0
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #0
        CMP     a1,a3,LSR #1
        MOVLS   a3,a3,LSR #8
        BLS     %BT02
        MOV     a1,a4
        MOV     pc,lr

__division_by_zero
        ; Currently we do not deal with division by zero in a nice way
        MOV     a1,#0
        MOV     a2,#0
        MOV     pc,lr

        EXPORT  __rt_udiv10
__rt_udiv10     ROUT
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
        MOV     pc, lr

        EXPORT  __rt_memcmp
__rt_memcmp     ROUT
        MOV     a4, a1
        MOVS    a1, a3
        MOVEQ   pc, lr

0       LDRB    a1, [a2], #1
        LDRB    ip, [a4], #1
        SUBS    a1, a1, ip
        MOVNE   pc, lr

        SUBS    a3, a3, #1
        BNE     %0
        MOV     pc, lr

        EXPORT  __rt_strlen
__rt_strlen     ROUT
        MOV     a2, a1
        MOV     a1, #0

0       LDRB    a3, [a2], #1
        CMP     a3, #0
        MOVEQ   pc, lr

        ADD     a1, a1, #1
        B       %0
        
;;      The following routines are currently only needed for the Fusion
;;      TCP/IP stack and Ethernet drivers, so don't include them if we
;;      don't have to

  IF    ETHERNET_SUPPORTED /= 0

        EXPORT  __rt_divtest
__rt_divtest    ROUT
        CMPS    a1, #0
        MOVNE   pc, lr

;;
;;      Fall through to...
;;

__rt_div0       ROUT
        MOV     a1, #angel_SWIreason_ReportException
        LDR     a2, =ADP_Stopped_DivisionByZero
        SWI     angel_SWI_ARM

        ; if we ever continue from this SWI, then report this and die
        FatalError "Divide by zero\n"

  ENDIF ; ETHERNET_SUPPORTED /= 0
  
;;
;; __rt_memXXX routines - small but inefficient
;;
        EXPORT  __rt_memset
__rt_memset     ROUT
        ADD     a3, a3, #1
        MOV     a4, a1

0       SUBS    a3, a3, #1
        MOVEQ   pc, lr
        STRB    a2, [a4], #1
        B       %0

        EXPORT  __rt_strcmp
__rt_strcmp     ROUT
        MOV     ip, a1

0       LDRB    a3, [ip], #1
        LDRB    a4, [a2], #1

        SUBS    a1, a3, a4
        MOVNE   pc, lr

        CMP     a3, #0
        CMPNE   a4, #0
        BNE     %0
        MOV     pc, lr

        EXPORT  __rt_strncmp
__rt_strncmp    ROUT
        MOV     ip, a1
        MOVS    a1, a3
        MOVEQ   pc, lr

        STR     lr, [sp, #-4]!

0       LDRB    lr, [ip], #1
        LDRB    a4, [a2], #1

        SUBS    a1, lr, a4
        LDRNE   pc, [sp], #4

        CMP     lr, #0
        CMPNE   a4, #0
        SUBNES  a3, a3, #1
        BNE     %0
        LDR     pc, [sp], #4

        EXPORT  __rt_strcpy
__rt_strcpy     ROUT
        MOV     ip, a1

0       LDRB    a3, [a2], #1
        STRB    a3, [ip], #1
        CMP     a3, #0
        BNE     %0

        MOV     pc, lr

        EXPORT  __rt_strcat
__rt_strcat     ROUT
        MOV     ip, a1          ; so we can return a1
0
        LDRB    a3, [ip], #1
        CMP     a3, #0
        BNE     %0
        SUB     ip, ip, #1      ; want to overwrite NUL
        
1       LDRB    a3, [a2], #1
        STRB    a3, [ip], #1
        CMP     a3, #0
        BNE     %1
        
        MOV     pc, lr


      IF CACHE_SUPPORTED <> 0
        EXPORT Cache_IBR
        ; Added for EBSARM, Instruction barrier range function. r0
        ; contains start of range, r1 end.
Cache_IBR
        ; Simply call the board dependent macro
        CACHE_IBR r0,r1,r2,r3
        ; And return
        MOV   pc,lr
      ENDIF

        IMPORT  Angel_EnterSVC
        EXPORT  __rt_uninterruptable_loop
__rt_uninterruptable_loop
      IF DEBUG <> 0 :LAND: LOGTERM_DEBUGGING <> 0
        IMPORT  logterm_fatalhandler

        ; Rather than sit in an infinite loop, allow memory to be scanned
        B       logterm_fatalhandler
      ENDIF
        
        MRS     a1, CPSR
        AND     a2, a1, #ModeMaskUFIS
        CMP     a2, #USRmode :AND: ModeMaskUFIS
        BLEQ    Angel_EnterSVC
        ORRNE   a1, a1, #InterruptMask
        MSRNE   CPSR_cf, a1
DeadLoop
        B       DeadLoop

        ; This code simulates the LogFatalError macro used in the C parts of
        ; Angel. Enter it with a1 pointing at the error message.
        EXPORT  __rt_asm_fatalerror

__rt_asm_fatalerror
      IF DEBUG = 1

        IMPORT  log_logerror
        IMPORT  Log_logmsginfo
        IMPORT  Angel_GlobalRegBlock 

        STMFD   sp!, {r0-r3}

        LDR     r0, =angel_SWIreason_EnterSVC
        SWI     angel_SWI_ARM          ; Get into SVC with IRQ and FIQ disabled

        SAVETASK Angel_GlobalRegBlock + (RB_Fatal * Angel_RegBlockSize), 0
        
        LDMFD   sp!, {r0-r3}

        SetStack Angel_SVCStackOffset

        ;; these variables are used in C code to say where the call came from;
        ;;  we don't have that info to hand for asm.
        STMFD   sp!, {r0-r3,r12,lr}

        MOV     r0, #0
        MOV     r1, #0
        MOV     r2, #0
        BL      Log_logmsginfo
        LDMFD   sp, {r0-r3,r12,lr}

        BL      log_logerror
        LDMFD   sp!, {r0-r3,r12,lr}

        ; debug build jumps into DeadLoop, release build restarts Angel.
        B       __rt_uninterruptable_loop

      ELSE

        B       __rt_angel_restart

      ENDIF

        ;; This routine exists as the action used when Angel needs to be
        ;; restarted. There are two occasions; when the debugger asks for
        ;; it, and when Angel detects a fatal error. The routine should
        ;; reload Angel from ROM, if possible, and restart as if a
        ;; power-on-reset had just occurred, as NOTHING can be assumed about
        ;; the current state of anything in RAM.
        ;; 
        ;; THIS ROUTINE NEVER RETURNS!
        
__rt_angel_restart

        IMPORT __rom
        EXPORT __rt_angel_restart

        ;; for the moment, just start at the beginning again...
        B __rom
        
                                
        END     ; EOF suppasm.s
