; normal.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.5 $
; Checkin $Date: 1998/03/19 15:04:11 $
; Revising $Author: wdijkstr $

;===========================================================================
;Veneers onto the arith.s functions.
;
;This block should be assembled multiple times, once for each function.
;The possible functions are:
;
;       normalise_s     Normalisation functions

        GBLL    SinglePrecision
        GBLL    DoublePrecision
        GBLL    ExtendPrecision

        GET     fpe.s

;===========================================================================

        CODE_32

        AREA    |C$$code|,CODE,READONLY

        EXPORT  __fp_normalise_op1
        EXPORT  __fp_normalise_op2
        EXPORT  __fp_normalise_op1neg
        EXPORT  __fp_norm_denorm_op1
        EXPORT  __fp_norm_denorm_op2

        GBLL    normalise_s

SinglePrecision SETL    {FALSE}
DoublePrecision SETL    {FALSE}
ExtendPrecision SETL    {FALSE}
normalise_s     SETL    {TRUE}

;===========================================================================

        GET     arith.s

        END
