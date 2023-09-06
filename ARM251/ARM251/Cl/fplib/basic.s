; basic.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.10 $
; Checkin $Date: 1998/03/19 15:04:07 $
; Revising $Author: wdijkstr $

; Basic floating point functions

        GET     fpe.s

        CodeArea |FPL$$Basic|

;==============================================================================
;Negate
;
;We just invert the sign bit.
;Since the sign bit is in *the same place* for both floats and doubles we
;use the same code.
;
;CANDIDATE FOR COMPILER INLINING

        [ THUMB
        EXPORT __16_dneg
        EXPORT __16_fneg

        CODE16
__16_dneg
__16_fneg
        MOV     a3, #1
        LSL     a3, #31
        EOR     a1, a3
        BX      lr

        CODE32
        ]

        EXPORT _dneg
        EXPORT _fneg

_dneg
_fneg
        ASSERT  dOPh = a1
        ASSERT  fOP = a1
        EOR     a1, a1, #SignBit
        ReturnToLR

;==============================================================================
;Absolute
;
;Just clear the sign bit. Only ever called with double - as if it matters.
;
;CANDIDATE FOR COMPILER INLINING.

        EXPORT fabs
        EXPORT _fabs

        [ THUMB
        CODE_16
fabs
_fabs
        LSL     dOPh, #1
        LSR     dOPh, #1
        BX      lr

        |
fabs
_fabs
        BIC     dOPh, dOPh, #SignBit
    [ INTERWORK
        BX      lr
    |
        [ {CONFIG} = 32
        MOV     pc, lr
        ]
        [ {CONFIG} = 26
        MOVS    pc, lr
        ]
    ]
        ]

;==============================================================================

        END
