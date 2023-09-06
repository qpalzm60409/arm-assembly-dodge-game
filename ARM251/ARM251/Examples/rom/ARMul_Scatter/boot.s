; 1.1.2.2
;**************************************************************
;*     File: boot.s                                           *
;*  Purpose: Application Startup Code                         *
;**************************************************************
;
; This code performs all the initialization required before
; branching to the main C application code.  It defines the
; ENTRY point, initializes the Stack Pointers for each mode,
; copies RO code and RW data from ROM to RAM and zero-initializes
; the ZI data areas used by the C code.
;
; This startup code is intended for use with hardware such as 
; the ARM PID board, where memory management maps an aliased 
; copy of the ROM at 0x04000000 to address zero on reset.
; Following reset, RAM is mapped into address zero, and the
; code then branches to execute from the real ROM.

                AREA    Boot, CODE, READONLY
                
; Some standard definitions...

Mode_USR        EQU     0x10
Mode_FIQ        EQU     0x11
Mode_IRQ        EQU     0x12
Mode_SVC        EQU     0x13
Mode_ABT        EQU     0x17
Mode_UNDEF      EQU     0x1B
Mode_SYS        EQU     0x1F            ; only available on ARM Arch. v4

I_Bit           EQU     0x80
F_Bit           EQU     0x40


; Locations of various things in our memory system
PID_RAM_Limit   EQU     0x2000000               ; = 512k, change to ...
                                                ; ... 0x200000 for 2Mb.

Stack_Limit     EQU     0xA00
SVC_Stack       EQU     Stack_Limit             ; = 0xA00
ABT_Stack       EQU     Stack_Limit - 0x200     ; = 0x800
UNDEF_Stack     EQU     ABT_Stack - 0x100       ; = 0x700
IRQ_Stack       EQU     UNDEF_Stack - 0x200     ; = 0x500
FIQ_Stack       EQU     IRQ_Stack - 0x100       ; = 0x400
USR_Stack       EQU     PID_RAM_Limit

ROM_Start       EQU     0x04000000              ; Base address of ROM after remapping
Instruct_2      EQU     ROM_Start + 4           ; Address of second instruction in ROM

ResetBase       EQU     0x0B000000              ; Address of reset controller base
ClearResetMap   EQU     ResetBase + 0x20        ; Offset of remap control from base


; --- Define entry point
        ENTRY

Start_Boot
        EXPORT Start_Boot

; --- Continue execution from ROM rather than aliased copy at zero
        LDR     pc, =Instruct_2

; --- Flip the memory map by writing to the ClearResetMap location
;     in the Reset and Pause Controller
        MOV     r0, #0
        LDR     r1, =ClearResetMap
        STRB    r0, [r1]

; --- Initialize stack pointer registers
; Enter IRQ mode and set up the IRQ stack pointer
        MOV     r0, #Mode_IRQ:OR:I_Bit:OR:F_Bit ; No interrupts
        MSR     cpsr_c, r0
        LDR     sp, =IRQ_Stack

; Enter FIQ mode and set up the FIQ stack pointer
        MOV     r0, #Mode_FIQ:OR:I_Bit:OR:F_Bit ; No interrupts
        MSR     cpsr_c, r0
        LDR     sp, =FIQ_Stack

; Enter ABT mode and set up the ABT stack pointer
        MOV     r0, #Mode_ABT:OR:I_Bit:OR:F_Bit ; No interrupts
        MSR     cpsr_c, r0
        LDR     sp, =ABT_Stack

; Enter IRQ mode and set up the IRQ stack pointer
        MOV     r0, #Mode_UNDEF:OR:I_Bit:OR:F_Bit ; No interrupts
        MSR     cpsr_c, r0
        LDR     sp, =UNDEF_Stack

; Set up the SVC stack pointer last and return to SVC mode
        MOV     r0, #Mode_SVC:OR:I_Bit:OR:F_Bit ; No interrupts
        MSR     cpsr_c, r0
        LDR     sp, =SVC_Stack

; --- Initialize memory system
        ; ...

; --- Initialize critical IO devices
        ; ...

; --- Initialize interrupt system variables here
        ; ...

; --- Initialize memory required by main C code

        IMPORT InitRegions
        BL     InitRegions ; in regioninit.s

; --- Now enable IRQs, change to user mode and set up user mode stack.

        MOV     r0, #Mode_USR:OR:F_Bit ; IRQ enabled
        MSR     cpsr_c, r0
        LDR     sp, =USR_Stack


; --- Now we enter the main C application code

        IMPORT  C_Entry
; If the main C code is in Thumb code rather than ARM, 
; we would need to change to Thumb state here.

        BL      C_Entry ; in C_main.c

; If above subroutine ever returns, just sit in an endless loop
here    B       here

        END


