;;; memcpyset.s: fast implementation of memcpy, memset, memmove
;;; Copyright (C) Advanced RISC Machines Ltd., 1991-1998

;;; RCS $Revision: 1.11 $
;;; Checkin $Date: 1998/05/19 10:08:40 $
;;; Revising $Author: wdijkstr $

        GET objmacs.s

        CodeArea

        GBLS  SLA                       ; shift towards low address end
        GBLS  SHA                       ; shift towards high address end
 [ {ENDIAN} = "big"
SLA     SETS "LSL"
SHA     SETS "LSR"
 |                                      ; assume little-endian
SLA     SETS "LSR"
SHA     SETS "LSL"
 ]

 [ make = "memcpy" :LOR: make = "all" :LOR: make = "shared-library"
src     RN    a2
dst     RN    a1
n       RN    a3
tmp1    RN    a4
tmp3    RN    ip

        Function memcpy, leaf

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

 ]

;======================================================================
; Copy down code
; ==============

;       This is exactly the same as the copy up code -
;       but it copies in the opposite direction.

 [ make = "memmove" :LOR: make = "all" :LOR: make = "shared-library"
src     RN    a2
dst     RN    a1
n       RN    a3
tmp1    RN    a4
tmp3    RN    ip

        IMPORT memcpy

        Function memmove, leaf

        CMP     src, dst
        BHI     memcpy

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

;------------------------------------------------------------
 ]

 [ make = "memset" :LOR: make = "all" :LOR: make = "shared-library"
src     RN    a2
dst     RN    a1
n       RN    a3
tmp1    RN    a4
tmp3    RN    ip

; extern void *memset(void * /*s*/, int /*c*/, size_t /*n*/)

        Function memset, leaf

        FunctionEntry , "dst"           ; Must return original dst.
        SUBS    n, n, #4                ; need at least 4 bytes
        BMI     TrailingBytes           ; < 4 bytes to go

        ; word align the dst - first find out how many bytes
        ; must be stored to do this.

        ANDS    tmp3, dst, #3           ; eq means aligned!
        BNE     AlignDst

        ; here when destination is word-aligned,
        ; number of bytes to transfer is (n+4), n is >= 0.

DstAligned
        AND     src, src, #&ff
        ORR     src, src, src, ASL #8
        ORR     src, src, src, ASL #16
        MOV     tmp1, src
        MOV     tmp3, src
        MOV     lr, src

        SUBS    n, n, #12-4             ; 12 bytes or more?
        BLT     TrailingWords

        SUBS    n, n, #32-12            ; n+32 to go.
        BLT     %F1

0       STMIA   dst!, {src, tmp1, tmp3, lr}
        STMIA   dst!, {src, tmp1, tmp3, lr}
        SUBS    n, n, #32
        BGE     %B0
        ; see if we can handle another 8

        CMN     n, #16
        STMGEIA dst!, {src, tmp1, tmp3, lr}
        SUBGE   n, n, #16

        ; note that we still have (n+32) bytes to go, and this is <16.

        ; Here when there are fewer than 16 bytes to go.

1       ADDS    n, n, #32-12               ; (n-12) to go

        ; Ok - do three words at a time.

2       STMGEIA dst!, {tmp1, tmp3, lr}
        SUBGES  n, n, #12
        BGE     %B2
        ; (n-12) bytes to go - 0, 1 or 2 words.  Check
        ; which.

TrailingWords
        ADDS    n, n, #12-4             ; (n-4) to go
        BLT     TrailingBytes        ; < 4 bytes to go
        SUBS    n, n, #4
        STRLT   src, [dst], #4
        STMGEIA dst!, {src, tmp1}
        SUBGE   n, n, #4

        ; Here with less than 4 bytes to go

TrailingBytes
        ADDS    n, n, #4
        Return  , "a1",, EQ             ; 0 bytes
        CMP     n, #2                   ; 1, 2 or 3 bytes
        STRB    src, [dst], #1
        STRGEB  src, [dst], #1
        STRGTB  src, [dst], #1
        Return  , "a1"                  ; recover old dst value

; word align dst - tmp3 contains current destination
; alignment.  We can store at least 4 bytes here.

AlignDst
        RSB     tmp3, tmp3, #4          ; 1-3 bytes to go
        CMP     tmp3, #2
        STRB    src, [dst], #1
        STRGEB  src, [dst], #1
        STRGTB  src, [dst], #1
        SUBS    n, n, tmp3              ; check number to go
        BLT     TrailingBytes           ; less than 4 bytes
        B       DstAligned

 ]

;------------------------------------------------------------

 [ make = "_memcpy" :LOR: make = "all" :LOR: make = "shared-library"

; fast word aligned memcpy (>= 4 words)

dst     RN  a1
src     RN  a2
n       RN  a3
t0      RN  a4
t1      RN  v1
t2      RN  ip
t3      RN  lr

_memcpy EnterWithLR_16
        
        FunctionEntry , "v1"  
        SUBS    n, n, #32
loop
        LDMGEIA src!, {t0,t1,t2,t3}
        STMGEIA dst!, {t0,t1,t2,t3}
        LDMGEIA src!, {t0,t1,t2,t3}
        STMGEIA dst!, {t0,t1,t2,t3}
        SUBGES  n, n, #32
        BGE     loop
        ADDS    n, n, #16
        LDMGEIA src!, {t0,t1,t2,t3}
        STMGEIA dst!, {t0,t1,t2,t3}
        MOVS    n, n, LSL #29
        LDMCSIA src!, {t0,t1}
        STMCSIA dst!, {t0,t1}
        LDRMI   t0, [src]
        STRMI   t0, [dst]
        Return  , "v1"

 ]

;------------------------------------------------------------

 [ make = "uread4" :LOR: make = "all" :LOR: make = "shared-library"

; read word from unaligned address

_uread4 EnterWithLR_16
        ; corrupts R1-R3
        [ {ENDIAN} = "little"
        LDRB    R1, [R0, #0]
        LDRB    R2, [R0, #1]
        LDRB    R3, [R0, #2]
        LDRB    R0, [R0, #3]
        ORR     R1, R1, R2, LSL #8
        ORR     R1, R1, R3, LSL #16
        ORR     R0, R1, R0, LSL #24
        |
        LDRB    R1, [R0, #0]
        LDRB    R2, [R0, #1]
        LDRB    R3, [R0, #2]
        LDRB    R0, [R0, #3]
        ORR     R0, R0, R3, LSL #8
        ORR     R0, R0, R2, LSL #16
        ORR     R0, R0, R1, LSL #24
        ]
        ReturnToLR
 ]

 [ make = "uwrite4" :LOR: make = "all" :LOR: make = "shared-library"

; write word to unaligned address

_uwrite4 EnterWithLR_16
        ; corrupts r2
        [ {ENDIAN} = "little"
        STRB    R0, [R1, #0]
        MOV     R2, R0, LSR #8
        STRB    R2, [R1, #1]
        MOV     R2, R0, LSR #16
        STRB    R2, [R1, #2]
        MOV     R2, R0, LSR #24
        STRB    R2, [R1, #3]
        |
        STRB    R0, [R1, #3]
        MOV     R2, R0, LSR #8
        STRB    R2, [R1, #2]
        MOV     R2, R0, LSR #16
        STRB    R2, [R1, #1]
        MOV     R2, R0, LSR #24
        STRB    R2, [R1, #0]
        ]
        ReturnToLR

 ]


 [ make = "strcmp" :LOR: make = "all" :LOR: make = "shared-library"

; fast word based strcmp (equivalent to the C version)

a   RN 0
b   RN 1
w1  RN 2
w2  RN 3
tmp RN ip
ones RN lr
res RN 0

strcmp  EnterWithLR_16
        TST     a, #3
        TSTEQ   b, #3
        BNE     strcmp_byteloop
        STR     lr, [sp, #-4]!
        LDR     ones, ones_word
strcmp_loop
        LDR     w1, [a], #4
        LDR     w2, [b], #4
        SUB     tmp, w1, ones
        BIC     tmp, tmp, w1
        ANDS    tmp, tmp, ones, LSL #7
        CMPEQ   w1, w2
        BEQ     strcmp_loop
    
        LDR     lr, [sp], #4

    [ {ENDIAN} = "little"

        MOV     res, w2, LSL #24    ; do subtraction reversed so that the
        SUBS    res, res, w1, LSL #24   ; C bit is setup correctly
        TSTEQ   tmp, #0x0FF0        ; clears carry (CANNOT use 0xFF!)
        BNE     strcmp_return

        MOV     res, w2, LSL #16
        SUBS    res, res, w1, LSL #16
        TSTEQ   tmp, #0xFF00        ; clears carry
        BNE     strcmp_return

        MOV     res, w2, LSL #8
        SUBS    res, res, w1, LSL #8
        TSTEQ   tmp, #0xFF0000      ; clears carry

        SUBEQS  res, w2, w1

strcmp_return
        ; If Z bit clear, the C bit determines whether result is
        ; positive or negative.
        MOVNE   res, res, RRX   ; only RRX if NE
        ReturnToLR

    |   ; big-endian

        MOV     res, w1, LSR #24
        SUBS    res, res, w2, LSR #24
        TSTEQ   tmp, #0xFF000000
        ReturnToLR NE

        MOV     res, w1, LSR #16
        SUBS    res, res, w2, LSR #16
        TSTEQ   tmp, #0x00FF0000
        ReturnToLR NE

        MOV     res, w1, LSR #8
        SUBS    res, res, w2, LSR #8
        TSTEQ   tmp, #0x0000FF00
        ReturnToLR NE

        SUB     res, w1, w2
        ReturnToLR

    ]

strcmp_byteloop
        LDRB    w1, [a], #1
        LDRB    w2, [b], #1
        CMP     w1, #1
        CMPCS   w1, w2
        BNE     strcmp_byteend
        LDRB    w1, [a], #1
        LDRB    w2, [b], #1
        CMP     w1, #1
        CMPCS   w1, w2
        BEQ     strcmp_byteloop
strcmp_byteend
        SUB     res, w1, w2
        ReturnToLR

ones_word   DCD 0x01010101

 ]
         
        END
