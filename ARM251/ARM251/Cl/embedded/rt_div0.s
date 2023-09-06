;;; unhosted.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.3 $
;;; Checkin $Date: 1998/04/20 15:41:47 $
;;; Revising $Author: wdijkstr $

        GET     objmacs.s
        GET     h_errors.s

        CodeArea

        CODE_32

 [ make = "div0" :LOR: make = "all" :LOR: make="shared-library"
        IMPORT  |__rt_trap|, WEAK

__rt_div0 EnterWithLR_16
        STMDB   sp!, {sp-pc}
        STMDB   sp!, {r0-r12}
        SUB     r14, r14, #4
        STR     r14, [sp, #15*4]
        ADR     r0, E_DivideByZero
        MOV     r1, sp
        LDR     ip, =__rt_trap
        CMP     ip, #0
    [ INTERWORK:LOR:THUMB
        BXNE    ip
    |
        MOVNE   pc, ip
    ]
        DCD     0xe6000010
        
        ErrorBlock DivideByZero, "Divide by zero"
 ]

        END
