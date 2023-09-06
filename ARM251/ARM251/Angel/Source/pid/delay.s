        SUBT    Ethernet Serialiser Support             > etherser.s
;
; $Revision: 1.1.2.1 $
;   $Author: rivimey $
;     $Date: 1998/10/02 18:51:16 $
;
; Copyright (c) 1996 Advanced RISC Machines Limited
; All Rights Reserved
;
; ---------------------------------------------------------------------

                GET     listopts.s
                GET     lolevel.s

                ASSERT  (listopts_s)
old_opt         SETA    {OPT}
                OPT     (opt_off)       ; disable listing of include files

; ---------------------------------------------------------------------

    [   (:LNOT: :DEF: etherser_s)

                GBLL    etherser_s
etherser_s      SETL    {TRUE}

                IMPORT  Angel_EnterSVC
                IMPORT  Angel_GlobalRegBlock
                IMPORT  angel_NextTask
                IMPORT  angel_SerialiseTaskCore

                AREA    |etherser|, CODE, READONLY

; ---------------------------------------------------------------------

;
; delay - delay for a given number of microseconds
;
; This routine is actually hopelessly inaccurate, as it depends upon
; the system clock frequency, and the speed of the memory it is running
; from.  It is written assuming that it is running as fast as possible,
; i.e. clocked at 25MHz in single-cycle SSRAM; if this is not the case,
; then it will just delay for proportionately longer.
;
; This routine is prototyped in ../support.h, but is implemented here as
; it needs to be target-card specific code.
;

;
; sanity check
;
        IF SYSCLOCK > 25 * 1000
          ERROR "SYSCLOCK too fast for delay"
        ENDIF

        EXPORT  delay
delay           ROUT

;
; we loop 4 times for every microsecond.  This gives a total of 28 clocks
; per microsecond - an error of 12%, which we can probably cope with.
;
; Note that, with the SSRAM on the PID, S-Cycles take a single cycle, and
; N-Cycles take 2 cycles to complete.
;
        mov     r0, r0, lsl #2

5       mov     r0, r0                  ; 1S
        subs    r0, r0, #1              ; 1S
        moveq   pc, lr                  ; 1S
        b       %5                      ; 1N+2S
                                        ; 28 cycles

; ---------------------------------------------------------------------

    ]   ; etherser_s

        OPT     (old_opt)   ; restore previous listing options

; ---------------------------------------------------------------------

        END

; EOF etherser.s
