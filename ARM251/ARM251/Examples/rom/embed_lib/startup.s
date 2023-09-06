; 1.1.2.2
        AREA    asm_code, CODE

; If assembled with ARMASM -16 the variable {CONFIG} will be set to 16
; II assembled with ARMASM the variable {CONFIG} will be set to 32
; Set the variable THUMB to TRUE or false depending on whether the
; file is being assembled for ARM or Thumb.
        GBLL    THUMB
    [ {CONFIG} = 16
THUMB   SETL    {TRUE}
; If assembling with ARMASM -16 go into 32 bit mode as the ARMulator will
; start up the program in ARM state.
        CODE32
    |
THUMB   SETL    {FALSE}
    ]

        IMPORT  C_Entry

        ENTRY

; Set up the stack pointer to point to the 512K.
        MOV     sp, #0x80000
; Get the address of the C entry point.
        LDR     lr, =C_Entry
        [ THUMB
; If building a Thumb version pass control to C_entry using the BX
; instruction so the processor will switch to THUMB state.
        BX      lr
    |
; Otherwise just pass control to C_entry in ARM state.
        MOV     pc, lr
    ]

        END
