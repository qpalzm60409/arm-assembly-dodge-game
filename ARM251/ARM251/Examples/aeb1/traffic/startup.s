; *********************************************************************
; *
; * Strategic Support Group
; *
; *********************************************************************

; *********************************************************************
; *
; * Module      : startup.s
; * Description : used to initialize the embedded C library
; * Status      : complete
; * Platform    : AEB-1
; * History     :
; *               980416 ASloss
; *                     - alter stack to point to 0x200000 (128k)
; *                     - added headers
; *
; * Notes       : This code was taken from ARM SDT 2.11 User Guide, 
; *               section 13.3
; *
; * Copyright (C) 1998 ARM Ltd. All rights reserved.
; *
; *
; * RCS $Revision: 1.2 $
; * Checkin $Date: 1998/08/06 18:49:58 $
; * Revising $Author: swoodall $
; *
; *********************************************************************
        
        AREA asm_code, CODE

; ********************************************************************* 
; * If assembled with TASM the variable {CONFIG} will be set to 16
; * If assembled with ARMASM the variable {CONFIG} will be set to 32
; * Set the variable THUMB to TRUE or false depending on whether the
; * file is being assembled with TASM or ARMASM.
; ********************************************************************* 

        GBLL THUMB
        [ {CONFIG} = 16

THUMB SETL {TRUE}
        
; *********************************************************************
; * If assembling with TASM go into 32 bit mode as the Armulator will
; * start up the program in ARM state.
; *********************************************************************

        CODE32
        |
THUMB SETL {FALSE}
        ]
        
        IMPORT C_Entry
        
        ENTRY
|__init|

; **********************************************************************
; * Set up the stack pointer to point to the 128K (AEB-1 top of memory).
; **********************************************************************

        MOV sp, #0x20000

; **********************************************************************
; * Get the address of the C entry point.
; **********************************************************************

        LDR lr, =C_Entry
        [ THUMB

; **********************************************************************
; * If building a Thumb version pass control to C_entry using the BX
; * instruction so the processor will switch to THUMB state.
; **********************************************************************

        BX lr
        |

; **********************************************************************
; * Otherwise just pass control to C_Entry in ARM state.
; **********************************************************************

        MOV pc, lr
        ]
         
        END

; **********************************************************************
; * End of startup.s
; **********************************************************************
 