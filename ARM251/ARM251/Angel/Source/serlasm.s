        TTL     Angel serialiser support                 > serlasm.s
        ;
        ; This file provides veneers used by the serialiser module, as
        ; well as some globaly needed functions.
        ;
        ; $Revision: 1.17.2.5 $
        ;   $Author: rivimey $
        ;     $Date: 1998/10/23 15:59:02 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ;

        KEEP

        GET     listopts.s           ; standard listing control
        GET     lolevel.s            ; automatically built manifest definitions
        GET     macros.s             ; standard assembler support
        GET     target.s             ; target specific manifests
        GET     taskmacs.s           ; task manipulation macros

        
        ; See serlock.h for the interface to these functions.
        IMPORT  Angel_StackBase
        IMPORT  angel_WaitCore
        IMPORT  Angel_GlobalRegBlock
        IMPORT  angel_SerialiseTaskCore
      IF DEBUG > 0
        IMPORT  angel_DebugTaskArea
        IMPORT  angel_DebugStartTaskCount
        IMPORT  angel_DebugQueueTaskCount
      ENDIF
        
        IMPORT  __rt_asm_fatalerror  ; error reporting via suppasm.s
        
        IMPORT  angel_TQ_Pool
        
        ; ---------------------------------------------------------------------
        ; CONTEXTTABLE
        ; ------------
        ;
        ; CONTEXTTABLE
        ;
        ; 16 byte lookup table used by the exception restore code to
        ; get the offset for the correct r13 for the mode being entered.
        ;
        AREA    |Serlasm$$RO$$Data|,PIC,READONLY
        ALIGN   
      IF MINIMAL_ANGEL = 0

        EXPORT  ContextLookuptable
ContextLookuptable

        ; r13 offsets by mode
        DCB     RegOffsR13usr
        DCB     RegOffsR13fiq
        DCB     RegOffsR13irq
        DCB     RegOffsR13svc
        DCB     0, 0, 0
        DCB     RegOffsR13abt
        DCB     0, 0, 0
        DCB     RegOffsR13und
        DCB     0, 0, 0
        DCB     RegOffsR13usr   ; sys mode

        ; r14 offsets by mode
        DCB     RegOffsR14usr
        DCB     RegOffsR14fiq
        DCB     RegOffsR14irq
        DCB     RegOffsR14svc
        DCB     0, 0, 0
        DCB     RegOffsR14abt
        DCB     0, 0, 0
        DCB     RegOffsR14und
        DCB     0, 0, 0
        DCB     RegOffsR14usr   ; sys mode

        ; SPSR offsets by mode
        DCB     RegOffsCPSR     ; no USR SPSR, access CPSR instead
        DCB     RegOffsSPSRfiq
        DCB     RegOffsSPSRirq
        DCB     RegOffsSPSRsvc
        DCB     0, 0, 0
        DCB     RegOffsSPSRabt
        DCB     0, 0, 0
        DCB     RegOffsSPSRund
        DCB     0, 0, 0
        DCB     RegOffsCPSR     ; no SYS SPSR, access CPSR instead
        
      ENDIF

        EXPORT  R13ContextLookuptable
R13ContextLookuptable
        R13LookupTable          ; from taskmacs.s

        ALIGN   

        AREA    |Serlasm$$RW$$Data|,PIC,READWRITE

        ; This is used to store the entry mode for EnterSVC, which
        ; is then used to restore the correct interrupt / mode when
        ; ExitToUSR is called.

angel_SVCEntryMode      % 4

        AREA    |Serlasm$$Code|,CODE,PIC,READONLY


        ; ****************************************************************
        ;
        ; (*fn)(void) Angel_EnterSVC(void)
        ;
        ; -------------------------------------------
        ;
        ; On Entry:
        ;    <CPSR Mode is either USR or SVC>
        ;
        ; On Exit:
        ;    <CPSR Mode is SVC, IRQ and FIQ flags set>
        ;    r0 <= address of Angel_ExitToUSR()
        ;    r1, r2 trashed (as allowed by APCS)
        ;    lr preserved
        ;    IF entry mode == USR
        ;       sp_svc <= sp_usr
        ;    ENDIF
        ;    angel_SVCEntryFlag <= (CPSR_entry == SVC)
        ;
        ; -------------------------------------------
        ; 
        ; This routine provides a wrapper for the SWI_EnterSVC 
        ; SWI call, allowing entry to Supervisor mode. Note that
        ; unlike the SWI, this call only allows entry from USR
        ; or SVC mode (note: not SYS).
        ; 
        ; The wrapper must note the entry mode to allow routines
        ; which nest this call and so call it in SVC mode to 
        ; return in SVC mode (even though they call ExitToUSR!)
        ; 
        ; These functions need to be revised.
        ; 
        ; The code below assumes that the SWI only trashes those
        ; registers which are allowed by APCS:  r0-3, ip, lr.
        ; 
        ; [Currently, the SWI will only trash r0, r1, lr]
        ; 
        ; -------------------------------------------

        EXPORT  Angel_EnterSVC

InappropriateModeError
        FatalError "Inappropriate Mode\n"

Angel_EnterSVC
        ; a1-a4 and ip may be corrupted under the APCS
        STMFD   sp!, {lr}
        MRS     a1, CPSR
        LDR     a3, =angel_SVCEntryMode
        STR     a1, [a3]
        AND     a2, a1, #ModeMaskUFIS
        CMP     a2, #SVCmode :AND: ModeMaskUFIS
        CMPNE   a2, #FIQmode :AND: ModeMaskUFIS
        CMPNE   a2, #IRQmode :AND: ModeMaskUFIS
        BEQ     WasSVCMode
        CMP     a2, #USRmode :AND: ModeMaskUFIS
        BNE     InappropriateModeError
        ; The SWI does everthing for us - except note that entry was from USR

WasUSRMode
        LDR     a1, =angel_SWIreason_EnterSVC
        SWI     angel_SWI_ARM   ; Get into SVC with IRQ and FIQ disabled
        ; the SWI returns with a1 == Angel_ExitToUSR, so we don't have to 
        ; load it again.
        LDMFD   sp!, {lr}
        MOV     pc, lr          ; Return to caller
        
WasSVCMode      ; Just disable IRQ and FIQ and note SVC entry from SVC
        ORR     a1, a1, #InterruptMask
        MSR     CPSR_cf, a1
        LDR     a1, =Angel_ExitToUSR
        LDMFD   sp!, {lr}       ; note that this is writing to lr_svc,
                                ; not lr_usr, which was what was saved!
        MOV     pc, lr


        ; ****************************************************************
        ;
        ; void Angel_ExitToUSR(void)
        ;
        ; -------------------------------------------
        ;
        ; On Entry:
        ;    <CPSR Mode MUST BE Privileged [unchecked]>
        ;    <angel_SVCEntryMode MUST contain a CPSR as set up
        ;        by Angel_EnterSVC>
        ;
        ; On Exit:
        ;    IF angel_SVCEntryFlag == USR mode value (0)
        ;       <CPSR Mode is USR, IRQ and FIQ flags unset>
        ;       sp_usr = sp_svc
        ;       sp_svc = Top of SVC Stack
        ;    ELSE ; must be SVC mode (1)
        ;       <CPSR Mode is SVC, IRQ and FIQ flags unset>
        ;    ENDIF
        ;    r0, r1, r2 trashed (as allowed by APCS)
        ;
        ; -------------------------------------------
        ; 
        ; This routine nominally returns from SVC mode to USR mode, on
        ; the assumption that the user has just called Angel_EnterSVC. If
        ; however the previous call to Angel_EnterSVC was made in SVC mode,
        ; then we return in SVC mode.
        ; 
        ; In either case, interrupts are enabled on exit, whatever state
        ; they were in on entry.
        ; 
        ; If returning to USR mode, the SVC stack pointer, which was lost on
        ; entry to SVC mode, is returned to the Angel SVC Top of stack value.

        EXPORT  Angel_ExitToUSR

Angel_ExitToUSR
        STMFD   sp!, {lr}
        LDR     a3, =angel_SVCEntryMode
        LDR     a1, [a3]
        AND     a2, a1, #ModeMaskUFIS
        CMP     a2, #SVCmode :AND: ModeMaskUFIS
        CMPNE   a2, #FIQmode :AND: ModeMaskUFIS
        CMPNE   a2, #IRQmode :AND: ModeMaskUFIS
        BEQ     ReturnFromExitToUSR
        CMP     a2, #USRmode :AND: ModeMaskUFIS
        BNE     InappropriateModeError

        ; The standard return to USR case follows ...
        MOV     a3, sp
        STMFD   a3, {a3}        ; copy sp_svc via SVC stack area ...
        SUB     a3, a3, #4
        LDMFD   a3, {sp}^       ; to sp_usr
        
        ; Reset SVC sp to the empty SVC stack
        SetStack Angel_SVCStackOffset
        
ReturnFromExitToUSR
        MSR     CPSR_cxsf, a1        ; return to starting mode...
        LDMFD   sp!, {lr}       ; note that this is writing to lr_svc,
        MOV     pc, lr          ; return to our caller (bank indep.)


     
        ; ****************************************************************
        ;
        ; int Angel_DisableInterruptsFromSVC(void)
        ;
        ; -------------------------------------------
        ; 
        ; Disable (prevent) interrupts. Assumes you are in a supervisor
        ; mode (that is, that MRS and MSR are allowed instructions).
        ;
        ; Passed no parameters;
        ; Returns the previous state of the CPSR
        ;
        ; FIQ SAFETYLEVEL is used to determine which interrupt
        ; flags are set.
        ;
        ; -------------------------------------------

        EXPORT  Angel_DisableInterruptsFromSVC

Angel_DisableInterruptsFromSVC
        MRS     a1, CPSR

        DisableAngelInts a1, a2
        
        MSR     CPSR_cf, a2
        MOV     pc, lr


        ; ****************************************************************
        ;
        ; int Angel_EnableInterruptsFromSVC(void)
        ;
        ; -------------------------------------------
        ; 
        ; Enable (allow) interrupts. Assumes you are in a supervisor
        ; mode (that is, that MRS and MSR are allowed instructions).
        ;
        ; Passed no parameters;
        ; Returns the previous state of the CPSR
        ;
        ; FIQ SAFETYLEVEL and HANDLE_INTERRUPTS_ON_IRQ are used to
        ; determine which interrupt flags are cleared. 
        ;
        ; -------------------------------------------

        EXPORT  Angel_EnableInterruptsFromSVC

Angel_EnableInterruptsFromSVC
        MRS     a1, CPSR

        EnableAngelInts a1, a2
        
        MSR     CPSR_cf, a2
        ;; and return in a1 (r0) the original CPSR...
        MOV     pc, lr


        ; ****************************************************************
        ;
        ; void Angel_RestoreInterruptsFromSVC(int old)
        ;
        ; -------------------------------------------
        ; 
        ; Restore the interrupt flags to the state indicated by 'old'.
        ; Assumes you are in a supervisor mode (that is, that MRS and MSR
        ; are allowed instructions).
        ;
        ; calculates: CPSR = (CPSR & ~IRQMASK) | (a1 & IRQMASK)
        ;
        ; Passed a copy of the CPSR in a1;
        ; Returns nothing.
        ;
        ; FIQ SAFETYLEVEL is used to determine which interrupt
        ; flags are changed.
        ;
        ; -------------------------------------------

        EXPORT Angel_RestoreInterruptsFromSVC

Angel_RestoreInterruptsFromSVC
        ;
        ; Get a copy of CPSR as it is now, and mask out the IRQ bits
        ; and mask from the old CPSR the other (non-IRQ) bits.
        
        MRS     a2, CPSR
        
        AND     a1, a1, #AngelInterruptMask
        BIC     a2, a2, #AngelInterruptMask

        ; now OR the previous value with the new IRQ bits and
        ; put the result back in CPSR. 
        ORR     a1, a2, a1
        MSR     CPSR_cf, a1

        MOV     pc, lr

        
        ; ****************************************************************
        ;
        ; int Angel_EnableInterruptsFromUSR(void)
        ;
        ; -------------------------------------------
        ; 
        ; Enable (allow) interrupts. Assumes you are in a USR mode,
        ; so the code must call EnterSVC to go into privileded mode in
        ; order to change the interrupt state.
        ;
        ; Passed no parameters;
        ; Returns the previous state of the CPSR
        ;
        ; -------------------------------------------

        EXPORT  Angel_EnableInterruptsFromUSR

Angel_EnableInterruptsFromUSR
        ; get the current CPSR and check we start in USR mode. Save r0
        ; and LR (which will both be corrupted by the SWI we need to do).
        MRS     r0, CPSR
        STMFD   sp!, {r0, lr}
        
        AND     r0, r0, #ModeMaskUFIS
        CMP     r0, #USRmode :AND: ModeMaskUFIS
        BNE     InappropriateModeError

        ; Get into SVC with IRQ and FIQ disabled and sp_svc <= sp_usr
        LDR     r0, =angel_SWIreason_EnterSVC
        SWI     angel_SWI_ARM
        
        ; now reload LR, CPSR from sp_svc before sp_svc is reset
        LDMFD   sp!, {r0, lr}
        
        ; Reset SVC sp to the empty SVC stack now we've finished with it
        SetStack Angel_SVCStackOffset

        ; do what we really wanted... enable ints again (r0 -> r1)
        EnableAngelInts r0, r1
        MSR     CPSR_cf, r1        ; back in USR mode now.

        ; we pushed two things to sp_usr, and popped from sp_svc, so
        ; we must now readjust sp_usr.
        ADD     sp, sp, #8      
                                ; 
        MOV     pc, lr          ; Return to caller
        
        ; ****************************************************************
        ;
        ; int Angel_GetCPSR(void)
        ;
        ; -------------------------------------------
        ; 
        ; Returns a copy of the current CPSR.
        ;
        ; Passed no parameters;
        ; Returns the state of the CPSR
        ;
        ; -------------------------------------------

        EXPORT  Angel_GetCPSR

Angel_GetCPSR
        MRS     a1, CPSR
                                ; 
        MOV     pc, lr          ; Return to caller

        
        ; ****************************************************************
        ;
        ; void Angel_SerialiseTask(int fromyield, (*fn)(), int state,
        ;                            int stack, Angel_RegBlock *interrupted)
        ; 
        ; -------------------------------------------
        ; 
        ; Passed:       nothing
        ; Returns:      nothing
        ; Side-effects: Enters the serialiser.
        ; 
        ; For the interface details of this pseudo function refer to
                ; serlock.h. Although it doesn't return (and thus preservation issues
        ; are irrelevant) the parameter passing is APCS conformant.
        ; 
        ; This routine is the entry point into the serialiser from exception
        ; handlers.
        ; 
        ; It must be called from a privileged mode with (Angel-relevant)
        ; interrupts disabled. Typically, it is called from an exception
        ; handler -- the interrupt, SWI, undef handlers all make heavy use
        ; of it. When called from such a handler, the 'stack' value is used
        ; to flatten the caller's mode's stack: this allows the stack at
        ; the point of the call to be unwound ready for the next exception,
        ; a difficult thing to do if the caller is a C program.
        ; 
        ; There is one other big use of this routine: Angel_Yield. See the
        ; comments for that routine for more details. However, the flag
        ; 'fromyield' is set by Angel_Yield to indicate that the stack
        ; parameter is not relevant as Yield managed it's own stacks.
        ; 
        ; The routine's operation is actually relatively simple. The
        ; stacked parameter is read and the stack flattened before the
        ; SAVETASK macro is used to save the current context. This 
        ; context is used as a template to create the new task. It
        ; also holds the other parameters, which must be carefully
        ; switched around to create the desired new context.
        ; 
        ; Bearing in mind that we may have been called in any privileged
        ; mode, we mow modify CPSR to move into SVC mode and ensure the
        ; SVC stack is suitable. Finally, we jump into the coroutine
        ; Angel_SerialiseTaskCore, which performs the task-selection
        ; actions before finally calling Angel_StartTask to execute the
        ; selected task.
        ; 

      IF MINIMAL_ANGEL = 0
        
        EXPORT  Angel_SerialiseTask
        EXPORT  angel_SaveTask

Angel_SerialiseTask
        ; r0 (a1) = called_by_yield
        ; r1 (a2) = fn
        ; r2 (a3) = state
        ; r3 (a4) = base stack pointer for calling ISR
        ; [sp] = interrupted_regblock address
        ;
        ; Note:  r3 is not relevant when called by Angel_Yield.

        ; Load interrupted_regbock for angel_SerialiseTaskCore from the stack,
        ; where it was put by the caller, *before* we flatten the stack.
        LDMFD   sp!, {r5} 

        ; if not called by yield (implication: we have been called by an
        ; interrupt service routine instead), flatten the stack of calling mode
        ; to prepare for the next interrupt).
        CMP     r0, #0
        MOVEQ   sp, r3

        ; Get a copy of the current state of the processor, which we
        ; can then modify to our requirements. Note this will trash the entry
        ; registers, but not before storing them in the desired regblock!

        SAVETASK Angel_GlobalRegBlock + (RB_Desired * Angel_RegBlockSize), 0
        
        ; Load base reg (which was set up by the SAVETASK macro) into r1, which is
        ; where it must be for SerialiseTaskCore
        MOV     r1, r0  
        
        ; Set up desired_regblock:
        ;   get R1 (fn) and store as new PC
        LDR     r3, [r1, #RegOffsR1]
        STR     r3, [r1, #RegOffsPC] ; function to call

        ; get original r0 (called_by_yield) before we overwrite it.
        LDR     r0, [r1, #RegOffsR0]
        
        ;   get R2 (state) and store as new R0
        LDR     r3, [r1, #RegOffsR2]
        STR     r3, [r1, #RegOffsR0] ; arg to function

        ; Load interrupted_regbock for angel_SerialiseTaskCore from the stack,
        ; where it was put by the caller, *before* we flatten the stack.
        LDR     r2, [r1, #RegOffsR5]
        
        ; Now:   In Desired regblock:
        ;        r0, pc set as required, mode-registers as
        ;          on entry, sp = flattened (if not yield).
        ;
        ;        r0 is now 'called by yield' arg for S.T.C
        ;        r1 is now 'desired regblock' arg for S.T.C
        ;        r2 is now 'interrupted regblock' arg for S.T.C
        ;
        ; r3, r4 are dead. [I *think* r5-9 are dead too.]

        ; move into SVC mode (previous code could well have been IRQ or FIQ)
        MRS     r4, cpsr
        BIC     r4, r4, #ModeMask
        ORR     r4, r4, #SVCmode
        MSR     cpsr_cf, r4

        ; Note that at this point the SVC stack may be in any of the
        ; following states:
        ;  * Empty Angel_SVCStack - if no task of WantsLock priority
        ;    is running, and the application is not using the SVC stack
        ;  * Non empty Angel_SVCStack - if a task with WantsLock priority
        ;    is running (this  call of serialise task will be due
        ;    to another packet arriving).
        ;  * Application SVC stack
        ;
        ; However, using the Application SVC stack is not acceptable
        ; since it may not be large enough, or indeed r13 may have been
        ; corrupted.  Therefore we must spot sp outside the range
        ; Angel_SVCStack - Angel_SVCStackLimit and if that is the case
        ; the it must be the Application stack in use, so switch to
        ; the Angel_SVCStack (which is empty in this case).
        
        LDR     r4, =Angel_StackBase
        LDR     r4, [r4]
        ADD     sl, r4, #Angel_SVCStackLimitOffset
        ADD     r4, r4, #Angel_SVCStackOffset
        CMP     sp, sl
        BLE     StackNotSetUp      ; sp <= Angel_SVCStackLimit
        CMP     sp, r4
        BMI     StackIsSetUp       ; sp <= Angel_SVCStack
StackNotSetUp
        MOV     sp, r4

StackIsSetUp        
        ; Call angel_SerialiseTaskCore and set up lr to "return" to
        ; angel_NextTask.

        LDR     lr, =angel_NextTask
        LDR     r4, =angel_SerialiseTaskCore
        MOV     pc, r4
        
      ENDIF                   ;  not minimal angel 


        ; ****************************************************************
        ;  void Angel_DebugLog(angel_RegBlock *rb, angel_TaskQueueItem *tq, int id)
        ;
        ; Write to memory (defined as 0x3000000) the following structure
        ;
        ;    [word   taskcount]
        ;    word   id
        ;    word   taskblock (tq)
        ;    word   regblock (rb)
        ;    word   current CPSR
        ;    word   current SP
        ;    word   pc value from regblock (rb)
        ;    word   cpsr value from regblock (rb)
        ;    word   stack pointer from regblock (rb)
        ;    word   r0 from regblock (rb)
        ;
        ; ID codes:      1 == StartTask
        ;                2 == QueueTask
        ;                3 == IntHandler Entry (tq == 0 -> count := starttask)
        ;                4 == SaveTask
        ;                5 == Yield Save
        ;                6 == SWI Save
        ;                7 == Undef (breakpoint) Save
        ;                8 == Wait Save
        ;                9 == Abort Save
        ;               10 == Delete Task
        
      IF MINIMAL_ANGEL = 0 :LAND: DEBUG <> 0

        EXPORT  Angel_DebugLog

Angel_DebugLog
        STMFD   sp!, {r3-r6}
        CMP     r1, #0
        LDRNE   r3, =angel_DebugQueueTaskCount
        LDREQ   r3, =angel_DebugStartTaskCount
        LDR     r6, [r3]
        ADD     r6, r6, #1
        STR     r6, [r3]
        LDR     r3, =angel_DebugTaskArea
        LDR     r4, [r3]
        STR     r6, [r4], #4            ; Store task count
        STR     r2, [r4], #4            ; Store id code
        STR     r0, [r4], #4            ; Store taskblock ptr
        STR     r1, [r4], #4            ; Store regblock ptr
        MRS     r6, CPSR
        STR     r6, [r4], #4            ; Store Current CPSR
        STR     sp, [r4], #4            ; Store Current SP
        LDR     r5, [r0, #RegOffsPC]
        LDR     r6, [r0, #RegOffsCPSR]
        STR     r5, [r4], #4            ; Store the dest pc
        STR     r6, [r4], #4            ; Store the CPSR
        
        ; Now find the stack pointer
        LDR     r6, =ContextLookuptable ; address of r13 lookup table
        AND     r5, r5, #0xF            ; Mode Mask, 32/26 insensitive
        LDRB    r6, [r6, r5]            ; SP offset for desired mode
        LDR     r5, [r0, r6]
        STR     r5, [r4], #4            ; Store the SP

        LDR     r6, [r0, #RegOffsR0]
        STR     r6, [r4], #4            ; Store R0
        
        ; Wrap after 7MB (leave some room for Angel stacks in an 8MB SIMM)
        LDR     r5, =DEBUG_BASE+DEBUG_SIZE
        CMP     r4, r5
        LDRGE   r4, =DEBUG_BASE
        STR     r4, [r3]

        LDMFD   sp!, {r3-r6}
        
        MOV     pc, lr
        
      ENDIF                   ;  not minimal angel 

        ; ****************************************************************
        ;
        ; void angel_StartTask(angel_RegBlock *regblock)
        ;
        ; -------------------------------------------
        ; 
        ; APCS interface to the RESUMETASK macro. Restore the current CPU context,
        ; from the register block pointed to by r0.
        ;
        ; THIS ROUTINE DOES NOT RETURN NORMALLY!
        ;
        ; -------------------------------------------
        

      IF MINIMAL_ANGEL = 0

        EXPORT angel_StartTask

angel_StartTask
        ; r0 (a1) = regblock
        ; see taskmacs.s for details.
      IF DEBUG <> 0
        
        ;; Angel_DebugLog's r0 is already there.
        MOV     r1, #0
        MOV     r2, #DL_StartTask
        BL      Angel_DebugLog

        LDR     r1, =angel_TQ_Pool
        CMP     r0, r1
        BLO     badrberr
        LDR     r1, =Angel_GlobalRegBlock + (RB_NumRegblocks * Angel_RegBlockSize)
        CMP     r0, r1
        BHI     badrberr
        ; 
        LDR     r2, [r0, #RegOffsPC]
        CMP     r2, #0x0006000
        BLO     badpcerr
        LDR     r1, =ROMTop
        CMP     r2, r1
        BHI     badpcerr
        ; 
        MRS     r2, cpsr
        AND     r2, r2, #ModeMaskUFIS
        CMP     r2, #USRmode :AND: ModeMaskUFIS
        BEQ     badmodeerr

      ENDIF
        
        RESUMETASK 
        ; And the task executes!
        
      IF DEBUG <> 0

badrberr
        MOV     r1, r0
        FatalError "StartTask: Invalid RB: %08lx.\n"

badpcerr
        MOV     r1, r2
        FatalError "StartTask: Invalid PC: %08lx.\n"

badmodeerr
        FatalError "StartTask: Invalid Mode  [cannot be USR mode].\n"

      ENDIF
        
      ENDIF  ; minimal angel

        ; ****************************************************************
        ;
        ; void angel_SaveTask(angel_RegBlock *regblock)
        ;
        ; -------------------------------------------
        ; 
        ; APCS interface to the SAVETASK macro. Saves the current CPU context,
        ; except that r0 is needed for the address and r1 as a temp.
        ;
        ; Callers should not assume a return value (although it will be the
        ; same as the entry parameter).
        ;
        ; -------------------------------------------
        
      IF MINIMAL_ANGEL = 0

angel_SaveTask
        ; r0 (a1) = regblock
        ; see taskmacs.s for details.
        
      IF DEBUG <> 0
        STMFD   sp!, {r0-r2, lr}
        ;; r0 is already ok.
        MOV     r1, #0
        MOV     r2, #DL_SaveTask
        BL      Angel_DebugLog
        
        MRS     r2, cpsr
        AND     r2, r2, #ModeMaskUFIS
        CMP     r2, #USRmode :AND: ModeMaskUFIS
        LDMFD   sp!, {r0-r2, lr}
        BEQ     badmodeerr      ; can't do this in USR mode
      ENDIF
        
        SAVETASK ignored, 1

        ; restore APCS save registers trashed by SAVETASK       
        ADD     r0, r0, #RegOffsR4
        LDMIA   r0, {r4-r7}

        ; and return.
        MOV     pc, lr
        
      ENDIF  ; minimal angel

        ; ****************************************************************
        ;
        ; void Angel_Yield(void)
        ;
        ; -------------------------------------------
        ; 
        ; Passed:       nothing
        ; Returns:      nothing
        ; Side-effects: Uses the serialiser. Enables interrupts (temporary).
        ; 
        ; For the interface details of this pseudo function refer to
        ; serlock.h.
        ; 
        ; Note that this an APCS conformant fn from the callers point
        ; of view, so we (only) have to preserve r4-r11, r13 and the
        ; mode we were called in. In this code all registers other than
        ; r0 and r1 are preserved.
        ; 
        ; Angel_Yield is a very complex function, which involves several
        ; state changes. In overview:
        ;   Angel_Yield()       enters SVC mode if not there already
        ;                       saves the current context (to return to)
        ;                       loads the call info for Angel_YieldCore
        ;                       calls Angel_SerialiseTask
        ;   Angel_SerialiseTask creates a context for Angel_YieldCore
        ;                       ... and ASAP enters Angel_YieldCore
        ;                       in SVC mode with interrupts enabled.
        ;   Angel_YieldCore     checks polled devices, if any.
        ;                       returns to the serialiser via NextTask
        ;   angel_NextTask      jumps to SelectNextTask
        ;   Angel_SelectNextTask  resumes the context saved in Angel_Yield,
        ;                       returning to the caller.
        ; 
        ; Because both the SAVETASK macro and the serialiser must be entered
        ; in supervisor mode with (at least the Angel-relevant) interrupts
        ; disabled, and Angel_Yield can be validly called in USR mode, the
        ; routine must check the current mode. If it is USR mode then it
        ; has to call Angel_EnterSVC to enter SVC mode. If in SVC mode we
        ; must ensure that Angel interrupts are disabled first.
        ; 
        ; Having got into the right mode, we must take a copy of the current
        ; context so we can return properly to the caller; a we have changed
        ; this (most notably in changing CPSR) this must be fixed up after
        ; calling SAVETASK.
        ; 
        ; Finally, the parameters for SerialiseTask must be set up, and
        ; at last SerialiseTask can be entered.
        ; 
        ; -------------------------------------------

        
      IF MINIMAL_ANGEL = 0

        EXPORT Angel_Yield
        IMPORT Angel_YieldCore

Angel_Yield
        ; See what mode we are in and get into SVC and disable interrupts
        MRS     r0, CPSR
        ; Save original cpsr in r0 before checking the entry mode
        AND     r1, r0, #ModeMaskUFIS
        CMP     r1, #USRmode :AND: ModeMaskUFIS
        BNE     NotEnteredInUSR
        
        ; In USR mode... must get into SVC mode. 
        STMFD   sp!, {r0, r14}          ; Save the original CPSR and r14
                                        ; in USR stack memory
        BL      Angel_EnterSVC          ; This makes svc_sp := usr_sp

        ; Now:  svc_sp = usr_sp, lr = [pc], r0-r3 (possibly) trashed,
        ; cpsr = {SVCmode, I=1, F=1}, SPSR = USRmode (as on entry).
        ; usr_sp = usr_sp(entry) - 8 (two words pushed earlier)

        ; The two words we pushed before the EnterSVC are popped from svc_sp;
        ; if we didn't do something to usr_sp here we will eventually return
        ; from the yield with the usr stack imbalanced. So, 'pop' these words
        ; from *both* stacks before we continue.
        
        ADD     r0, sp, #8              
        STMFD   sp!, {r0}               ; copy sp_svc via USR stack memory ...
        MOV     r0, sp
        LDMFD   r0, {sp}^               ; to sp_usr
        ADD     r0, r0, #4              ; restore sp to counteract STMFD
        MOV     sp, r0                  ; return to sp_svc
        
        LDMFD   sp!, {r0, r14}          ; now restore r0,14 from svc_sp
                                        ; (still USR memory though)
        
        ; this is required as we never call Angel_ExitToUSR, which would
        ; normally do it for us.
        
        SetStack Angel_SVCStackOffset
        
InSVCInterruptsDisabled
        ; This macro saves the current context to the Yield regblock, setting
        ; the return address to current LR, which *should* be the return PC
        ; for the thing which called Angel_Yield.
        SAVETASK Angel_GlobalRegBlock + (RB_Yield * Angel_RegBlockSize), 0

        LDR     r1, [r0, #RegOffsR0]    ; r0 had the original CPSR
        STR     r1, [r0, #RegOffsCPSR]  ; save it in CPSR ready for return
        
      IF DEBUG <> 0
        ;; r0 is already ok.
        MOV     r1, #0
        MOV     r2, #DL_YieldSave
        BL      Angel_DebugLog
      ENDIF
        
        ;  put regblock address on stack for SerialiseTask
        STMFD   sp!, {r0}
        
        ; Now prepare to call Angel_SerialiseTask
        ; r0 = called_by_yield          ; r1 = fn
        ; r2 = state                    ; r3 = empty_stack
        ; [sp] = interrupted (yield) regblock address (set up above)
        MOV     r0, #1
        LDR     r1, =Angel_YieldCore
        MOV     r2, #0                  ; YieldCore has no parameter
        MOV     r3, #0                  ; empty_stack not used for Yield

        B     Angel_SerialiseTask
        ; and Angel_YieldCore will execute when SerialiseTask allows it to

NotEnteredInUSR
        CMP     r1, #SVCmode :AND: ModeMaskUFIS
        BNE     YieldNotInSvcOrUsrMode
        
YieldInSvcMode
        ; Using the CPSR in r0 (from the start of the routine) create a new
        ; cpsr in r1 with interrupts disabled, then make that current. We
        ; use the original cpsr in r0 -- it must not be trashed.
        
        DisableAngelInts r0, r1
        MSR     cpsr_cf, r1
        B       InSVCInterruptsDisabled
        
YieldNotInSvcOrUsrMode
        ; Not USR or SVC - Oh dear!!!
        FatalError "Yield not USR or SVC mode\n"

      ENDIF  ; minimal angel
        

        ; ****************************************************************
        ;
        ; int Angel_Wait(int signalbits)
        ; r0                      r0
        ;
        ; -------------------------------------------
        ; 
                
      IF MINIMAL_ANGEL = 0

        EXPORT Angel_Wait

Angel_Wait
        ; See what mode we are in and get into SVC and disable interrupts
        MRS     r2, CPSR
        ; Save original cpsr in r2 before checking the entry mode
        AND     r1, r2, #ModeMaskUFIS
        CMP     r1, #USRmode :AND: ModeMaskUFIS
        BNE     WaitNotEnteredInUSR
        
        ; In USR mode... must get into SVC mode. 
        STMFD   sp!, {r0, r2, r14}      ; Save the original r0, CPSR and r14
                                        ; in USR stack memory
        BL      Angel_EnterSVC          ; This makes svc_sp := usr_sp

        ; Now:  svc_sp = usr_sp, lr = [pc], r0-r3 (possibly) trashed,
        ; cpsr = {SVCmode, I=1, F=1}, SPSR = USRmode (as on entry).
        ; usr_sp = usr_sp(entry) - 8 (two words pushed earlier)

        ; The three words we pushed before the EnterSVC are popped from svc_sp;
        ; if we didn't do something to usr_sp here we will eventually return
        ; from the yield with the usr stack imbalanced. So, 'pop' these words
        ; from *both* stacks before we continue.
        
        ADD     r2, sp, #12              
        STMFD   sp!, {r2}               ; copy sp_svc via USR stack memory ...
        MOV     r2, sp
        LDMFD   r2, {sp}^               ; to sp_usr
        ADD     r2, r2, #4              ; restore sp to counteract STMFD
        MOV     sp, r2                  ; return to sp_svc
        
        LDMFD   sp!, {r0, r2, r14}      ; now restore r0,r2,14 from svc_sp
                                        ; (still USR memory though)
        
        ; this is required as we never call Angel_ExitToUSR, which would
        ; normally do it for us.
        
        SetStack Angel_SVCStackOffset
        
InWaitSVCInterruptsDisabled
        ; This macro saves the current context to the Yield regblock, setting
        ; the return address to current LR, which *should* be the return PC
        ; for the thing which called Angel_Yield.
        SAVETASK Angel_GlobalRegBlock + (RB_Yield * Angel_RegBlockSize), 0

        LDR     r1, [r0, #RegOffsR2]    ; r2 had the original CPSR
        STR     r1, [r0, #RegOffsCPSR]  ; save it in CPSR ready for return
        
        ; now have the caller's context saved (apart from r2). Get our
        ; TQI pointer. Leave r0 pointing at the Yield Regblock.
        LDR     r1, =angel_CurrentTask
        LDR     r1, [r1]
        
      IF DEBUG <> 0
        ;; r0, r1 are already ok.
        MOV     r2, #DL_WaitSave
        BL      Angel_DebugLog
      ENDIF

        B       angel_WaitCore

        
WaitNotEnteredInUSR
        CMP     r1, #SVCmode :AND: ModeMaskUFIS
        BNE     WaitNotInSvcOrUsrMode
        
WaitInSvcMode
        ; Using the CPSR in r0 (from the start of the routine) create a new
        ; cpsr in r1 with interrupts disabled, then make that current. We
        ; use the original cpsr in r0 -- it must not be trashed.
        
        DisableAngelInts r2, r1
        MSR     cpsr_cf, r1
        B       InWaitSVCInterruptsDisabled
        
WaitNotInSvcOrUsrMode
        FatalError "Wait not USR or SVC mode\n"

      ENDIF  ; minimal angel
        
        
        ; ****************************************************************
        ;
        ; void angel_NextTask(void)
        ;
        ; -------------------------------------------
        ; 
        ; Passed:       nothing
        ; Returns:      nothing
        ; Side-effects: enables interrupts; calls Angel_Yield, which
        ;               polls the polled interfaces, if any.
        ; 
        ; THIS ROUTINE NEVER RETURNS TO THE CALLER.
        ; 
        ; This 'routine' is used as the veneer between a task which has
        ; completed and the serialiser, which must select a new task to
        ; run. 
        ; 
        ; The routine must ensure the serialiser is called in supervisor
        ; mode with interrupts disables and the Angel supervisor stack in
        ; place; this may require calling Angel_EnterSVC to get into 
        ; supervisor mode from USR mode if the exiting task was a USR mode
        ; callback.
        ; 
        ; It should consequently only ever be entered because a function
        ; executes a MOV pc, lr instruction (or equivalent LDM) with
        ; lr = angel_NextTask; this is set up before the task is run by
        ; Angel_SetupTaskEnvironment.
        ; 
        ; -------------------------------------------

      IF MINIMAL_ANGEL = 0

        IMPORT angel_SelectNextTask
        IMPORT angel_CurrentTask
        IMPORT angel_DeleteTask
        EXPORT angel_NextTask
        
angel_NextTask
        ; This can be "called" from SVC or USR.  It must get into SVC and
        ; disable interrupts and grab the empty SVC stack.
        ; Then it must call angel_SelectNextTask with no arguments.
        ;
        ; See what mode we are in and get into SVC and disable interrupts

        MRS     r0, CPSR
        AND     r1, r0, #ModeMaskUFIS
        CMP     r1, #USRmode :AND: ModeMaskUFIS
        CMPNE   r1, #SYSmode :AND: ModeMaskUFIS
        BNE     NotEnteredInUSR2
        BL      Angel_EnterSVC          ; This keeps the stacks the same
        B       InSVCInterruptsDisabled2

NotEnteredInUSR2
        CMP     r1, #SVCmode :AND: ModeMaskUFIS
        BNE     NextTaskNotUsrOrSvcMode
        
        DisableAngelInts r0, r1
        MSR     cpsr_cf, r1
        
        ; Now in SVC with interrupts disabled
InSVCInterruptsDisabled2
        ; Grab an empty SVC stack and call angel_SelectNextTask
        SetStack Angel_SVCStackOffset

        ; Call angel_DeleteTask with CurrentTask as the task to delete.
        LDR     r0, =angel_CurrentTask
        LDR     r0, [r0]
        BL      angel_DeleteTask

        ; and select another....
        LDR     r0, =angel_SelectNextTask
        MOV     pc, r0

        
NextTaskNotUsrOrSvcMode 
        FatalError "NextTask not USR or SVC mode\n"
        
      ENDIF  ; minimal angel



        ; ****************************************************************

        END
