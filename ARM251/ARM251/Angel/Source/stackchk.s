        TTL     Angel assembler support routines                    > stackchk.s
        ; ---------------------------------------------------------------------
        ; This source files holds the general assembler routines
        ; needed by Angel.
        ;
        ; $Revision: 1.3.2.3 $
        ;   $Author: rivimey $
        ;     $Date: 1998/10/15 17:12:31 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995, 1997, 1998.
        ; All Rights Reserved
        ;
        ; ---------------------------------------------------------------------

        GET     listopts.s              ; standard listing control
        GET     lolevel.s               ; generic ARM definitions
        GET     macros.s                ; standard assembler support
        GET     target.s                ; target specific definitions
        GET     taskmacs.s              ; exception/task mgmt support

        ; ---------------------------------------------------------------------
        
      IF :LNOT: :DEF: LINKING_WITH_CLIB
        
        ; ---------------------------------------------------------------------
        ; -- Software stack checking support ----------------------------------
        ; ---------------------------------------------------------------------

        ; The following functions are provided for software stack
        ; checking. If hardware stack-checking is being used then the
        ; code can be compiled without the PCS entry checks, and
        ; simply rely on VM management to extend the stack for a
        ; thread.
        ;
        ; The stack extension event occurs when the PCS function
        ; entry code would result in a stack-pointer beneath the
        ; stack-limit register value. The system relies on the
        ; following map:
        ;
        ; +-----------------------------------+ <-- end of stack block
        ; | ...                               |
        ; | ...                               |
        ; | active stack                      |
        ; | ...                               | <-- sp (stack-pointer) somewhere in here
        ; | ...                               |
        ; +-----------------------------------+ <-- sl (stack-limit)
        ; | stack-extension handler workspace |
        ; +-----------------------------------+ <-- base of stack block
        ;
        ; The "stack-extension handler workspace" is an amount of
        ; memory in which the stack overflow support code must
        ; execute. It must be large enough to deal with the worst
        ; case through the stack overflow handler code.
        ;
        ; At the moment the compiler expects this to be AT LEAST
        ; 256bytes. It uses this fact to code functions with small
        ; local data usage within the overflow space.
        ;
        ; NOTE: We may need to increase the space between sl and the
        ; true limit to allow for the stack extension code and any
        ; other system software that may temporarily use the stack of
        ; the current foreground thread.
        ;
        ; The following example stack overflow handling code requires
        ; the following run-time functions:
        ;       __rt_allocmem   ; Kernel memory allocator "malloc"
        ;       __rt_freemem    ; Kernel memory allocator "free"
        ; We need to ensure that the MAXIMUM stack usage possible by the
        ; above routines, fits into the stack overflow handler space
        ; left beneath the "sl" value. The above routines also need to
        ; be compiled with stack-overflow checking disabled.
        ;
        ; If the "__rt_" routines needed by this code are not
        ; interrupt safe (i.e. if they cannot be called from within an
        ; interrupt handler) then this code must ensure that any code
        ; executing as part of an interrupt handler does NOT generate
        ; a stack extension event.

        AREA    |StackOverflow$$Code|,CODE,READONLY
        KEEP

        IMPORT  __rt_asm_fatalerror

        ; NOTE: This code assumes that it is entered with a valid
        ; frame-pointer. If the system is being constructed without
        ; frame-pointer support, then the following code will not
        ; work, and an alternative means of providing soft
        ; stack-extension will need to be coded.

        EXPORT __rt_stkovf_split_small
__rt_stkovf_split_small
        ; Called when we have a standard register saving only
        ; stack-limit check failure.
        MOV     ip,sp   ; ensure we can calculate the amount of stack required
        ; and then fall through to...

        IMPORT Angel_StackBase
        EXPORT __rt_stkovf_split_big
__rt_stkovf_split_big
        ; If we are in a privileged mode then stack overflow is fatal, so
        ; we should do nothing more than output a debugging message and
        ; give up!

        MRS     r0, CPSR
        AND     r0, r0, #ModeMaskUFIS
        CMP     r0, #USRmode :AND: ModeMaskUFIS
        BEQ     USRmodeStackCheck

        MOV     r1, r0
        FatalError "Stack Overflow in mode %2X\n"

USRmodeStackCheck
        ; If this is the AngelStack then sl should
        ; be somewhere within the Angel Stack range
        LDR     r1, =Angel_StackBase
        LDR     r1, [r1]
        ADD     r0, r1, #Angel_AngelStackLimitOffset
        ADD     r1, r1, #Angel_AngelStackOffset
        SUB     r1, r1, #1
        CMP     sl, r0
        CMPHS   r1, sl

AngelStackOverflow
        BHS     AngelStackOverflow

RealStackCheckCode
        ; Called when we have a large local stack allocation (>
        ; 256bytes with the current C compiler) which would cause an
        ; overflow. This version is called when the compiler generated
        ; PCS code has checked its requirement against the stack-limit
        ; register.
        ;
        ; in:   sp = current stack-pointer (beneath stack-limit)
        ;       sl = current stack-limit
        ;       ip = low stack point we require for the current function
        ;       lr = return address back to the function requiring more stack
        ;       fp = frame-pointer
        ;
        ;               original sp --> +----------------------------------+
        ;                               | pc (12 ahead of PCS entry store) |
        ;               current fp ---> +----------------------------------+
        ;                               | lr (on entry) pc (on exit)       |
        ;                               +----------------------------------+
        ;                               | sp ("original sp" on entry)      |
        ;                               +----------------------------------+
        ;                               | fp (on entry to function)        |
        ;                               +----------------------------------+
        ;                               |                                  |
        ;                               | ..argument and work registers..  |
        ;                               |                                  |
        ;               current sp ---> +----------------------------------+
        ;
        ; The "current sl" is somewhere between "original sp" and
        ; "current sp" but above "true sl". The "current sl" should be
        ; "APCS_STACKGUARD" bytes above the "true sl". The value
        ; "APCS_STACKGUARD" should be large enough to deal with the
        ; worst case function entry stacking (160bytes) plus the stack
        ; overflow handler stacking requirements, plus the stack
        ; required for the memory allocation routines, and the
        ; exception handling code.
        ;
        ; THINGS TO NOTE:
        ; We should ensure that every function that calls
        ; "__rt_stkovf_split_small" or "__rt_stkovf_split_big" does so
        ; with a valid PCS. ie. they should use "fp" to de-stack on
        ; exit (see notes above).
        ;
        ; Code should never poke values beneath sp. The sp register
        ; should always be "dropped" first to cover the data. This
        ; protects the data against any events that may try and use
        ; the stack. This is a requirement of APCS-3.
        ;
        SUB     ip,sp,ip        ; extra stack required for the function
        STMFD   sp!,{v1,v2,lr}  ; temporary work registers
        ;

        ; For simplicity never attempt to extend the stack just report
        ; an overflow.
RaiseStackOverflow
        ; in:  v1 = undefined
        ;      v2 = undefined
        ;      sl = stack-limit
        ;      ip = amount of stack required
        ;      sp = FD stack containing {v1,v2,lr}
        ;      lr = undefined
        ;

        ;; 960506 KWelton       
        ;;      
        ;; I'm not sure what this is trying to do, but it all seems redundant
        ;; to me - what we *really* want to do is report the overflow, and
        ;; then stop in a tight loop.
        
        ;; others disagree... this kills Angel, and without angel debug
        ;; code, this is not useful. IJ has requested that such errors
        ;; cause a restart of angel (i.e. jump to __rt_angel_restart)
        ;; while debug builds can jump to Deadloop. -- 22/10/97 RIC
        
        IF {FALSE}

        MRS     v1,CPSR         ; get current PSR
        TST     v1,#ModeMaskUFIS

        MOV     v2, a1
        LDR     a1, =angel_SWIreason_EnterSVC
        SWI     angel_SWI_ARM
        MOV     a1, v2
        ; We are now in a suitably priviledged mode.
        MSR     SPSR_cf,v1         ; get original PSR into SPSR
        STMFD   sp!,{r10-r12}   ; work registers
        LDR     r10,=ADP_Stopped_RunTimeError
        MOV     r11,#ADP_RunTime_Error_StackOverflow
        MOV     lr,pc           ; return address

        ELSE

        MOV     a1, #angel_SWIreason_ReportException
        LDR     a2, =ADP_Stopped_StackOverflow
        SWI     angel_SWI_ARM

        ENDIF
        
        ;; If we get here, print a message and die...
        FatalError "Stack Overflow\n"
        
      ENDIF ; :DEF: LINKING_WITH_CLIB

        ; ---------------------------------------------------------------------

                               
        END     ; EOF stackchk.s
