; ********************************************************************************
; *
; * ARM Strategic Support Group
; *
; ********************************************************************************

; ********************************************************************************
; *
; * Module      : irq_ven.s                   
; * Description : Angel IRQ veneer code                     
; *    
; *           This code is written to allow chaining of the
; *           example IRQ code with the Angel IRQ handler      
; *           caring for the registers using the stack.   
; *
; * Status     : Release
; * Platform   : AEB-1
; * History    : 
; *
; *       970326 DBrooke
; *
; *          - created example code
; *
; *       980420 MEchavarria
; * 
; *          - modified for Sharp 790A
; *          - added timer counter interrupt handler
; *
; *       980429 ASloss
; *
; *          - added button interrupt handler
; *
; *
; * Notes      :
; *
; *  The AREA must have 
; *  - the attribute READONLY, otherwise the linker will not
; *   place it in ROM.
; *  - the attribute CODE, otherwise the assembler will not
; *   let us put any code in this AREA
; *
; * Copyright (C) 1998 ARM Ltd. All rights reserved.
; *
; *
; * RCS $Revision: 1.2 $
; * Checkin $Date: 1998/08/06 18:49:57 $
; * Revising $Author: swoodall $
; *
; ********************************************************************************

; ********************************************************************************
; * IMPORT/EXPORT
; ********************************************************************************

        IMPORT irq_auxtimer
        IMPORT irq_buttonpress
        EXPORT handler_irq
        EXPORT Angel_IRQ_Address
        EXPORT SetupSVC         ; just a stub, angel's done the job already

        AREA    irq, CODE, READONLY

; ********************************************************************************
; * DATA
; ********************************************************************************

IRQStatus       DCD     0xFFFFA814      ;IRQ controller status register address

; *********************************************************************************
; * ROUTINES
; *********************************************************************************

; -- irqHandler -------------------------------------------------------------------
;
; Description   : handles the IRQ interrupt and determines the source and then
;                 vectors to the correct interrupt rountine.
;

handler_irq
        STMFD   sp!, {r0 - r3, LR}      ;Maintain Stack using APCS standard
        LDR     r0, IRQStatus           ;Get address of the IRQ status Reg 
        LDR     r0, [r0]                ;Read the status reg byte
        TST     r0, #0x0080             ;Is it a timer1 interrupt ?
        BNE     handler_event_timer     ;Branch if timer
        TST     r0, #0x0001             ;Is it a button press ?
        BNE     handler_event_button    ;Branch if button
        LDMFD   sp!, {r0 - r3, lr}      ;If not then its an Angel request
        LDR     pc, Angel_IRQ_Address   ;remove the regs from the stack and call 
                                        ;the routine
;
; **********************************************************************************
; EVENT: Timer 
; **********************************************************************************
;

handler_event_timer
        BL      irq_auxtimer            ;if this point is reached then the irq is from 
                                        ;the timer
        LDMFD   sp!, {r0 - r3,lr}       ;return from the irq_auxtimer routine - 
                                        ;restore the registers 
        subs    pc, lr, #4              ;and return from the interrupt
;
; **********************************************************************************
; EVENT: Button
; **********************************************************************************
;

handler_event_button
        BL      irq_buttonpress         ;if this point is reached then the irq is from the button
        LDMFD   sp!, {r0 - r3,lr}       ;return from the irq_buttonpress routine - restore the registers 
        subs    pc, lr, #4              ;and return from the interrupt
                
SetupSVC
        MOV     pc, lr                  ; return


        AREA    var, DATA, READWRITE

Angel_IRQ_Address       
        DCD 0x00000000  ;Scratch location for Angel IRQ Handler address


        END

; ***********************************************************************************
; * End of irq_ven.c 
; ***********************************************************************************