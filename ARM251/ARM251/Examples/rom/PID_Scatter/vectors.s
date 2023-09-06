        AREA Vect, CODE, READONLY

; These exception vectors and dummy exception handlers will be
; relocated at start-up from ROM to SSRAM at address 0

; *****************
; Exception Vectors
; *****************

        LDR     PC, Reset_Addr
        LDR     PC, Undefined_Addr
        LDR     PC, SWI_Addr
        LDR     PC, Prefetch_Addr
        LDR     PC, Abort_Addr
        NOP                             ; Reserved vector
        LDR     PC, IRQ_Addr
        LDR     PC, FIQ_Addr
        
        IMPORT  IRQHandler              ; In C_int_handler.c
        IMPORT  Start_Boot              ; In boot.s
        
Reset_Addr      DCD     Start_Boot
Undefined_Addr  DCD     Undefined_Handler
SWI_Addr        DCD     SWI_Handler
Prefetch_Addr   DCD     Prefetch_Handler
Abort_Addr      DCD     Abort_Handler
                DCD     0               ; Reserved vector
IRQ_Addr        DCD     IRQHandler
FIQ_Addr        DCD     FIQ_Handler


; ************************
; Dummy Exception Handlers
; ************************
; The following handlers do not do anything useful in this example.
; They are set up here for completeness.

Undefined_Handler
        B       Undefined_Handler
SWI_Handler
        B       SWI_Handler     
Prefetch_Handler
        B       Prefetch_Handler
Abort_Handler
        B       Abort_Handler
FIQ_Handler
        B       FIQ_Handler
        
        END

