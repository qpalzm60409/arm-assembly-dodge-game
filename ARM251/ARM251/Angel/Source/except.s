        TTL     Angel exception support                 > except.s
        ; ---------------------------------------------------------------------
        ;
        ; This file provides the default Angel exception vector handlers.
        ;
        ; $Revision: 1.24.2.8 $
        ;   $Author: rivimey $
        ;     $Date: 1998/10/23 15:52:16 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995, 1997, 1998.
        ; All Rights Reserved
        ;
        ; ---------------------------------------------------------------------

        KEEP

        GET     listopts.s           ; standard listing control
        GET     lolevel.s            ; automatically built manifest definitions
        GET     macros.s             ; standard assembler support
        GET     target.s             ; target specific manifests
        GET     taskmacs.s


        EXPORT  __VectorStart
        EXPORT  __SoftVectors
        IMPORT  angel_IRQInterruptHandler
        IMPORT  angel_FIQInterruptHandler
        IMPORT  Angel_ExitToUSR
        IMPORT  __rt_asm_fatalerror
        IMPORT  ContextLookuptable
        IMPORT  Angel_SerialiseTask
        IMPORT  Log_logmsginfo
        IMPORT  log_loginfo
        IMPORT  angel_SWIReturnToApp
        IMPORT  angel_SysCallCount
        IMPORT  angel_CurrentTask
        IMPORT  angel_ComplexSWILock
        IMPORT  Angel_StackBase
        IMPORT  angel_SWIDeferredBlock
        IMPORT  Angel_AccessApplicationTask
        IMPORT  angel_QueueTask
        IMPORT  angel_SelectNextTask
        IMPORT  Angel_DebugLog

      IF MINIMAL_ANGEL = 1

        AREA    |C$$zidata|,DATA,READWRITE,NOINIT

        EXPORT  Angel_GlobalRegBlock

Angel_GlobalRegBlock
        %       Angel_RegBlockSize * RB_NumRegblocks

        
      ELSE

        IMPORT  Angel_GlobalRegBlock

      ENDIF
        

        ; Default ARM hardware exception vectors
        ;
        ; When building ROM at zero systems, the link should be modified
        ; to force this area to be first.

      IF      ROMonly

        AREA    |__Vectors|,CODE,PIC,READONLY

      ELSE
        ; Masquerade as ReadWrite Code so that this AREA gets placed first
        ; in the Read-Write segment by the linker, and has two underscores
        ; to help it get sorted first in case anyone has any real RW Code !

        AREA    |__Vectors|,CODE,PIC,READWRITE

      ENDIF
        ;
        ; After initialisation the following vectors *MUST* start at
        ; 0x00000000 (an ARM processor requirement).
        ;
        ; The following vectors load the address of the relevent
        ; exception handler from an address table.  This makes it
        ; possible for these handlers to be anywhere in the address space.
        ;
        ; To bootstrap the ARM, some form of ROM is present at
        ; 0x00000000 during reset.  Often the startup code performs
        ; some target specific magic to remap RAM to address
        ; zero. The ROM based Read/Write data and BSS must then be
        ; copied to the relevant RAM address during the Angel ROM
        ; initialisation.
        ;
__VectorStart                   ; Start of ARM processor vectors
        LDR     pc,ResetV       ; 00 - Reset
        LDR     pc,UndefV       ; 04 - Undefined instructions
        LDR     pc,SWIV         ; 08 - SWI instructions
        LDR     pc,PAbortV      ; 0C - Instruction fetch aborts
        LDR     pc,DAbortV      ; 10 - Data access aborts
        LDR     pc,UnusedV      ; 14 - Reserved (was address exception)
        LDR     pc,IRQV         ; 18 - IRQ interrupts
        LDR     pc,FIQV         ; 1C - FIQ interrupts
        ;
        ; NOTE: In a normal optimised ARM system the FIQ vector would
        ; not contain a branch to handler code, but would have an
        ; allocation immediately following address 0x1C, with the FIQ
        ; code being placed directly after the vector table. This
        ; avoids the pipe-line breaks associated with indirecting to a
        ; handler routine.

        ; However Angel is designed to be a simple system so we
        ; treat FIQ like all the other vectors, and this allows the
        ; actual handler addresses to be stored immediately after the
        ; ARM vectors. If optimal FIQ entry is required, then space
        ; could be allocated at this point to hold the direct FIQ
        ; code. The __SoftVectors table would then simply appear
        ; higher in the RAM allocation.
        ;
__SoftVectors
        ; Reset - an error unless ROM always at zero, or a branch
        ; to High ROM on reset has been requested explicitly (Cogent Board)

        IMPORT  __rom
ResetV  
    IF ROMonly
        DCD     __rom                ; 00 - Reset, not mapped at boot
    ELSE
      IF BRANCH_TO_HIGH_ROM_ON_RESET <> 0
        DCD     ROMBase              ; 00 - Reset, remap ROM during boot (e.g. PID)
      ELSE
        DCD     __rom                ; 00 - Reset, mapped at boot
      ENDIF
    ENDIF
        
UndefV  DCD     HandlerUndef         ; 04 - undef
SWIV    DCD     HandlerSWI           ; 08 - software interrupt
PAbortV DCD     HandlerPrefetchAbort ; 0C - prefectch abort
DAbortV DCD     HandlerDataAbort     ; 10 - data abort
UnusedV DCD     0                    ; 14 - will reset if called...
IRQV    DCD     angel_IRQInterruptHandler ; 18 - IRQ
FIQV    DCD     angel_FIQInterruptHandler ; 1C - FIQ

        
__VectorEnd             ; End of Angel code copied to zero


        AREA    |ExceptionInit|,CODE,PIC,READONLY

      IF :LNOT: ROMonly

        ; This is removed for FLASH at 0 systems where a 
        ; write to the flash may cause reprogramming - in the case of the
        ; ATMEL devices on the PID. If ROM is at 0 there is no need to
        ; initialise the vectors as they are hard coded.
        
        ; This function performs the exception system initialisation:
        
        EXPORT angel_ExceptionInit
angel_ExceptionInit
        ; in:   no arguments
        ; out:  no result
        ;
        ; Here we copy the vector table (__VectorStart to __VectorEnd)
        ; to address 0.
        ; This is clearly necessary if we have a RAM based system, and
        ; is clearly pointless if we have a ROM at 0 based system.

        LDR     a1,=__VectorStart       ; start of data
        MOV     a2,#0x00000000          ; destination address
        LDR     a3,=__VectorEnd         ; end of data
01      LDR     a4,[a1],#4              ; get word from data AREA
        STR     a4,[a2],#4              ; store to RAM at zero
        CMP     a1,a3                   ; check for end condition
        BLO     %BT01                   ; if (a2 < a3) we have more to transfer
        ;
        MOV     pc,lr

      ENDIF ;:LNOT: ROMonly
        
        ; ****************************************************************
        ;
        ; void CallSerialiseTask(angel_Regblock *, int reason)
        ;                               r0             r2
        ;
        ; -------------------------------------------
        ; 
        ; Internal 'function': Set up a call to SerialiseTask to run
        ; the Angel ThreadStopped callback. The regblock which contains
        ; the context of the stopped thread is in r0, and the reason it
        ; stopped in r2. 
        ; 
        ; 
        ; 
        ; 
        ; THIS FUNCTION DOES NOT RETURN TO THE CALLER.
        ; 
        ; -------------------------------------------
      IF MINIMAL_ANGEL = 0

        IMPORT angelOS_ThreadStopped
CallSerialiseTask
        ; Now prepare to call Angel_SerialiseTask
        ; r0 and r2 are set up by caller
        ; r0 = register block to save to
        ; r2 = Stopped reason code to pass to debugger
        ;

        ; SerialiseTask expects:
        ; r0 = called_by_yield
        ; r1 = function to call (angelOS_ThreadStopped)
        ; r2 = type (already set up)
        ; r3 = empty_stack
        ; [sp] = save regblock
        
        LDR     r1, =angelOS_ThreadStopped
        MOV     r3, sp                  ; this MOV must be before the STMFD, below!
        STMFD   sp!, {r0}
        MOV     r0, #0
        B       Angel_SerialiseTask     ; Not coming back
        
        ; and angelOS_ThreadStopped will execute when SerialiseTask
        ; allows it to

      ENDIF

        
        ; *****************************************************************

        ; Exception handlers

        AREA    |DefaultVectorHandlers|,CODE,PIC,READONLY

        ; This is the ARM exception interface to the Angel debug world.
        ;
        ; This code provides the default ARM vector handlers for the
        ; processor exceptions. If the target application (or the
        ; more likely an O/S) does provide vector handlers,
        ; then they should be coded to provide the VectorCatch
        ; checking code performed below, to ensure that a debug agent
        ; can stop on system events. However, this is very system
        ; specific.
        ;
        ; By default the Angel debug world makes use of the
        ; Undefined Instruction exception, SWI's and eother IRQ's or
        ; FIQ's depending on which the board uses.  All of the
        ; other exceptions are unused by Angel, and the default action
        ; is purely to raise a debug event.
        ;
        ; The model used by the serialisation module is discussed
        ; in detail elsewhere.  This module follows the rules and
        ; guidelines laid out by the serialiser.

        ; in: UND mode; IRQs disabled; FIQs undefined
        ;     r13 = FD stack
        ;     r14 = address of undefined instruction + 4
        ;     All other registers must be preserved

        EXPORT HandlerUndef
        EXPORT HandlerDataAbort
        EXPORT HandlerPrefetchAbort
        IMPORT memory_is_being_accessed
        IMPORT memory_access_aborted
        
        
        ; ****************************************************************
        ;
        ; void HandlerPrefetchAbort()
        ;
        ; -------------------------------------------
        ; 
        ; 
        ; 
        ; -------------------------------------------
        
HandlerPrefetchAbort
        
      IF MINIMAL_ANGEL <> 0

        ;; we're not supposed to be handling this, so don't
        SUBS    pc, lr, #4
        
      ELSE
        
        EXCEPTENTRY_PART1 ABTmode, Angel_GlobalRegBlock + (RB_ABORT * Angel_RegBlockSize)
        LDR     r6, =ADP_Stopped_PrefetchAbort
        B       GenericAbort
        
      ENDIF
        
        ; ****************************************************************
        ;
        ; void HandlerDataAbort()
        ;
        ; -------------------------------------------
        ; 
        ; 
        ; 
        ; -------------------------------------------
        
HandlerDataAbort
        
      IF MINIMAL_ANGEL <> 0

        ;; we're not supposed to be handling this, so don't
        SUBS    pc, lr, #4
        
      ELSE
        
        EXCEPTENTRY_PART1 ABTmode, Angel_GlobalRegBlock + (RB_ABORT * Angel_RegBlockSize)
        LDR     r6, =ADP_Stopped_DataAbort
        
        ;; fall through to...
        ; B     GenericAbort
      ENDIF

        
      IF MINIMAL_ANGEL = 0
        
        IMPORT  angel_StartTask
GenericAbort

        ; Complete the exception entry sequence. This bit is not handler type
        ; specific. R6 contains the exception type code.
        
        EXCEPTENTRY_PART2
        
      IF DEBUG <> 0
        STMFD   sp!, {r0-r3,lr}
        MOV     r1, #0
        MOV     r2, #DL_AbortSave
        BL      Angel_DebugLog
        LDMFD   sp!, {r0-r3,lr}
      ENDIF

        ; The value of r14 has been decremented by 4, i.e. referring to the
        ; instruction after the aborted instruction.
        ; r0 points at (the start of) the regblock referred to in the
        ; entry macro.

        ; We have to check that the undefined instruction we hit
        ; was indeed the Angel Undefined instruction
        ;
        ; Clearly this uses different code for Thumb and ARM states
        ; but make the Thumb support code removable
        ;
        ; Note that r14_und has been adjusted to point to the undef
        ; (ARM or Thumb).
        LDR     r1, =memory_is_being_accessed
        LDR     r1, [r1]
        CMP     r1, #0
        BEQ     NotAngelOSAbort  ; Not due to Debugger Request Aborting!

        ; Now we know that it is either angelOS_MemRead or angelOS_MemWrite
        ; that has aborted.  These both run in User32 mode, and if aborted
        ; then we went into Abort32 mode, storing the real pc, cpsr
        ; in the abort regs.  This will in turn have gone through the
        ; table of undefined instructions and hence got into Und32 mode,
        ; which is where we are now.  Thus we have to read the Abort regs
        ; and put them into the saved register block and then restart
        ; using that block.

        ; Indicate an abort has happened by incrementing this variable,
        ; so the code can find out how many times...
        LDR     r1, =memory_access_aborted
        LDR     r2, [r1]
        ADD     r2, r2, #1
        STR     r2, [r1]
        
        ; r0 already has the regblock pointer -- just jump back...
        B       angel_StartTask

NotAngelOSAbort
        IMPORT  |Image$$RO$$Base|   ; start of code area
        IMPORT  |Image$$RO$$Limit|  ; end of code area
        
        LDR     r1, =|Image$$RO$$Base|
        CMP     lr, r1
        BLO     AbortNotAngelCode
        
        LDR     r1, =|Image$$RO$$Limit|
        CMP     lr, r1
        BHI     AbortNotAngelCode

AbortInAngelCode
        ; A data or prefetch abort in Angel ... oops!
        ; Note: the test code above misses out a check for the
        ; relocating code in the ROM etc.
        
        MOV     a2, lr          ; parameter for msg
        FatalError "Abort encountered in Angel code at pc=%p.\n"
        

AbortNotAngelCode
        ; For a data abort, we need to subtract 8 to get back to the address
        ; at which the aborted instruction was loaded; we have already subtracted
        ; 4 in EXCEPTENTRY_PART1; now do the other 4.
        LDR     r2, =ADP_Stopped_DataAbort
        CMP     r2, r6
        SUBEQ   r14, r14, #4
        
        ; r0 already has the regblock pointer -- load r2 with the
        ; reason code from r6 and report back to the debugger.
        MOV     r2, r6
        B       CallSerialiseTask

      ENDIF     ; not Minimal Angel



        ; ****************************************************************
        ;
        ; void HandlerUndef()
        ;
        ; -------------------------------------------
        ; 
        ; Vector handler function for the Undefined instruction vector.
        ; When compiled into EmbeddedICE (ICEMAN) the exception simply means
        ; that the code is broken; hence we just ignore it!
        ; 
        ; Angel uses undefined instructions to implement breakpoints for the
        ; debugger, as well as the possibility that the user has inserted their
        ; own undefines which are to be trapped by the debugger. So the handler
        ; must check which instruction code caused the exception. The only
        ; difference here, however, is which of the ADP_Stopped reason codes
        ; is used to report this event to the debugger.
        ; 
        ; The code to check the breakpoint is complicated slightly by the
        ; requirement to check for both Thumb and ARM breakpoints, although
        ; the entry and exit actions are the same in both cases.
        ; 
        ; The helper function CallSerialiseTask is used to call the serialiser
        ; with appropriate args to report the exception. 
        ; 
        ; If the exception was caused by a breakpoint instruction, the debugger
        ; should rewrite it with the correct instruction before returning here
        ; to continue execution.
        ; 
        ; Entry:        r14 = <address of undef>+4
        ;               r13 = empty_undef_stack
        ;               cpsr_mode = UNDmode
        ; 
        ; Exit:         <via Serialiser, does not return directly>
        ; 
        ; -------------------------------------------
        

HandlerUndef
      IF MINIMAL_ANGEL<>0

          ;; we're not supposed to be handling this, so don't
        SUBS    pc, lr, #4
        
      ELSE

        EXCEPTENTRY UNDmode, Angel_GlobalRegBlock + (RB_UNDEF * Angel_RegBlockSize)
        
        ; The value of r14 has been decremented by 4, i.e. referring to the
        ; undefined instruction. r0 points at (the start of) the regblock
        ; referred to in the entry macro.

        ; We have to check that the undefined instruction we hit
        ; was indeed the Angel Undefined instruction
        ;
        ; Clearly this uses different code for Thumb and ARM states
        ; but make the Thumb support code removable
        ;
        ; Note that r14_und has been adjusted to point to the undef
        ; (ARM and Thumb).

        
      IF DEBUG <> 0
        STMFD   sp!, {r0-r3,lr}
        MOV     r1, #0
        MOV     r2, #DL_UndefSave
        BL      Angel_DebugLog
        LDMFD   sp!, {r0-r3,lr}
      ENDIF

      IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
        MRS     r0, SPSR                ; read CPSR of mode we were in...
        TST     r0, #Tbit
        BEQ     CompareWithARMBreakPoint   
      
        LDRH    r0, [r14]               ; load the Thumb instruction
        MOV     r1, #angel_BreakPointInstruction_THUMB :AND: 0xff00
        ORR     r1, r1, #angel_BreakPointInstruction_THUMB :AND: 0xff
        CMP     r0, r1
        BNE     UnrecognisedInstruction
        B       BreakPointInstruction
      ENDIF

CompareWithARMBreakPoint
        LDR     r0, [r14]        ; load the ARM instruction
        LDR     r1, =angel_BreakPointInstruction_ARM
        CMP     r0, r1
        BNE     UnrecognisedInstruction
        
        ; We now know it is Angel's Undefined instruction

        ; r0 = r1 = The undefined instruction
        ; lr = address of the undefined instruction
        ; spsr = unchanged
        ; sp  = FD UND stack (currently empty)

BreakPointInstruction
        ; This is executed for both an ARM and Thumb breakpoint
        LDR     r0, =Angel_GlobalRegBlock + (RB_UNDEF * Angel_RegBlockSize)
        LDR     r2, =ADP_Stopped_BreakPoint
        B       CallSerialiseTask


UnrecognisedInstruction
        LDR     r0, =Angel_GlobalRegBlock + (RB_UNDEF * Angel_RegBlockSize)
        LDR     r2, =ADP_Stopped_UndefinedInstr
        
        B       CallSerialiseTask


      ENDIF ; End of MINIMAL ANGEL


        ; *************************************************************
        ;
        ; void HandlerSWI()
        ;
        ; -------------------------------------------
        ; 
        ; 
        ; 
        ; in: SVC mode; IRQs disabled; FIQs undefined
        ;     r13 = FD stack
        ;     r14 = address of SWI instruction + 4
        ;     All other registers must be preserved
        ;
        ; -------------------------------------------
        
        IMPORT  angelOS_SemiHostingEnabled
        
        EXPORT  HandlerSWI

HandlerSWI
        STMFD   sp!, {r0, r1}
        
        ; Disable interrupts. The FIQ handler 'knows' that SWI has problems
        ; here and will return as soon as it realizes a FIQ has been taken
        ; around here.

        EnsureFIQDisabled cpsr, r0
        
        ; We have to check that the SWI we hit was indeed the Angel SWI
        ;
        ; This initial code is designed to be interrupt-safe, although we
        ; don't re-enable IRQ (which would be disabled on entry to the SWI
        ; automatically).

      IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
        
        MRS     r1, SPSR
        TST     r1, #Tbit
        BEQ     CompareWithARMSWI1

        LDRH    r1, [r14, #-2]          ; load the Thumb instruction
        BIC     r1, r1, #0xff00
        CMP     r1, #angel_SWI_THUMB
        BNE     ComplexSWI
        B       CheckSimpleSWI
        
      ENDIF                             ; THUMB_SUPPORT

CompareWithARMSWI1
        LDR     r1, [r14, #-4]          ; load the ARM instruction
        LDR     r0, =angel_SWI_ARM      ; load SWI number (+ SWI instr. code)
        BIC     r1, r1, #0xFF000000     ; condition & instr. code, which shouldn't be compared.
        EORS    r0, r0, r1              ; If the values are the same, we'll end up with zero
        BNE     ComplexSWI
        ; ... fall through to 'simple' SWI case
        
        ;; 
        ; is this Angel SWI a "Simple" one -- that is, it doesn't call the
        ; scheduler and is fast.
CheckSimpleSWI
        LDR     r0, [sp]                ; load original value of r0 from stack
        CMP     r0, #angel_SWIreason_EnterSVC
        BNE     ComplexSWI

DoSWIEnterSVC
        ; Handle the EnterSVC SWI. See "ARM SDT Reference Manual" $8.3 for what
        ; docs exist on this.

        ; Basically:     enter with r0 == angel_SWIreason_EnterSVC
        ;                           mode == USR mode
        ;                           sp == Angel/Appl USR stack
        ;
        ;                exit with  r0   <= address of a routine to return to
        ;                                   USR mode
        ;                           mode <= SVC mode with IRQ, FIQ disabled
        ;                           sp_svc <= sp_usr
        ; 
        ; The SDT Reference Manual guarantees that r1 - r3 are preserved by
        ; Angel when an Angel system call is made, so other regs can be used.
        ; 
        ; -----
        ; Angel additionally allows one level of nesting, so that if
        ; entered in SVC mode, the return routine will return in SVC mode.
        ;
        ; As written this code will allow entry in any privileged mode, and
        ; return in SVC mode as stated above.
        ;
        ; However, the return routine (Angel_ExitToUSR) will not return to
        ; the previous mode, but to SVC mode instead, with the wrong stack.
        
        MRS     r1, SPSR                ; mode we came from
        ; Accessing other modes depends on the mode
        AND     r1, r1, #ModeMaskUFIS
        CMP     r1, #USRmode :AND: ModeMaskUFIS
        CMPNE   r1, #SYSmode :AND: ModeMaskUFIS
        BNE     EnterSVCfromPriv

        ; Now either we came from USR or SYS mode.
        ; Write USR or SYS sp on the entry stack over saved r0.
        ; Original SVC SP IS LOST!
        STMIA   sp, {sp}^
        
      IF HANDLE_INTERRUPTS_ON_FIQ = 0
        ; Not handling interrupts on FIQ, so we didn't disable FIQ
        ; at the start of the handler. Do so now, so we return with
        ; both interrupt sources disabled.
        MRS     r0, cpsr
        ORR     r0, r0, #FIQDisable
        MSR     cpsr_cf, r0
      ENDIF

        ; jump to exit code to load r0 with it's correct value.
        B       ReturnFromEnterSVC

        
EnterSVCfromPriv
        ; we have called the EnterSVC SWI in a privileged mode.
        ;   r0 is the angel_SWIreason_EnterSVC code number,
        ;   r1 is the masked mode we came from
        ; neither values are needed any more.
        
        MRS     r1, cpsr        ; need a copy of SVC CPSR
        MRS     r0, spsr        ; get CPSR we came from ...
        
      IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
        ; If we came from thumb state, don't (try to) return there just
        ; yet!
        BIC     r0, r0, #Tbit
      ENDIF
        
        ; In any mode, don't return to 26 bit mode (0x10), and don't
        ; enable interrupts just yet.
        ORR     r0, r0, #InterruptMask + 0x10
        
        MSR     cpsr_cf, r0        ; switch to the mode we came from
        
        AND     r0, r0, #ModeMaskUFIS
        ; NOTE: If the spsr was _already_ SVCmode, the value we save has
        ; to be adjusted for data put on the stack by the SWI!
        CMP     r0, #SVCmode :AND: ModeMaskUFIS
        MOV     r0, sp          ; take a copy of this mode's SP
        ADDEQ   r0, r0, #8
        
      IF HANDLE_INTERRUPTS_ON_FIQ = 0
        ; Not handling interrupts on FIQ, so we didn't disable FIQ
        ; at the start of the handler. Do so now, so we return with
        ; both interrupt sources disabled.
        ORR     r1, r1, #FIQDisable
      ENDIF
        
        MSR     cpsr_cf, r1        ; switch back to SVC mode
        STMIA   sp, {r0}        ; Store out the USR or SYS stack pointer
        
        ; now in SVC mode with sp_svc <= sp_<other mode>
        ; fall through to exit code to return to the caller in this state.

ReturnFromEnterSVC
        ; Set the stack back to that at the beginning of the SWI.
        LDMFD   sp!, {r0, r1}
        MOV     sp, r0          ; put other mode's SP in here.

        MRS     r0, spsr        ; We need to find out if the system was in
                                ; Thumb state in its original operation
        
        ; We could be returning to a Thumb code segment so we must use the
        ; correct return method for Thumb ie BX rn

        [ :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
        TST     r0, #Tbit
        BNE     SVCThumb
        ]        
        
        ; Set r0 to the address of Angel_ExitToUser
        LDR     r0, =Angel_ExitToUSR

        ; Resume in SVC mode with interrupts disabled.
        MOV     pc, lr
        
        [ :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
SVCThumb

        ; Set r0 to the address of Angel_ExitToUser
        LDR     r0, =Angel_ExitToUSR
        
        ; Push r0 and the lr (for the return) on to the stack
        STMFD   sp!, {r0, lr}

        ; Set bit 0 to 1 for the change to Thumb State
        ADR     r0, SVCRettoThumb+1

        BX      r0

        CODE16
SVCRettoThumb

        
        ; And do the return BX - now in SVC mode retaining the stack info
        POP {r0, pc}

        ALIGN
        CODE32
        ]
        
NotEnterSVC

        CMP     r0, #angel_SWIreason_SysElapsed
        CMPNE   r0, #angel_SWIreason_SysTickFreq
        BNE     NotSimpleSWI

        ; Reason code: SYS_TICKFREQ:
        ;     Purpose: Returns the number of elapsed target 'ticks' since the
        ;              support code started executing.  Ticks are defined by
        ;              SYS_TICKFREQ.  (If the target cannot define the length of
        ;              a tick, it may still supply SYS_ELAPSED.)

        ; Reason code: SYS_TICKFREQ
        ;     Purpose: Define a tick frequency.  (The natural implementation is
        ;              to have a software model specify one core cycle as one tick,
        ;              and for Angel to return Timer1 ticks)

        ; Both these are recognised but not implemented by Angel. Return -1
        ; to indicate failure.

        LDMFD   sp!, {r0, r1}

        MOV     r1, #-1
        MOVS    pc, lr


NotSimpleSWI
        B       ComplexSWI



        LTORG
        
        ; 
        ; This code handles the cases where the SWI causes Scheduler actions
        ; to take place. It encompases three groups of action:
        ;   a. unrecognised SWI numbers
        ;   b. unrecognised Angel SWI reasons
        ;   c. C Library calls such as SWI_READ
        ;   d. Complex Angel actions, such as LateStartup.
        ; 
        ; In order for the scheduler to be callable, we must have saved our
        ; execution context. It is best if this is done with the original
        ; (on entry to handler) context, as this may be used by a debugger
        ; with that meaning.
        ;
        ; We have already disabled FIQ's, so that isn't a problem (remember we
        ; save SPSR as the return CPSR).
        ; 
        ; Sadly, we now have to go check the SWI instruction all over again,
        ; as we can't remember what we've just done :-(
        ; 
SWILockBad
        FatalError "Complex SWI nested: would overwrite context.\n"

CSWIFromAngel
        FatalError "Complex SWI called from within Angel: stack use conflict.\n"

ComplexSWI

        ; Check first that we're not already nested inside another SWI.
        ; We can use r0, r1 here as they are already on the stack.
        LogInfo "except.s", 693, LOG_EXCEPT, "ComplexSWI Takelock\n"
        
        TAKELOCK angel_ComplexSWILock, r0, r1
        CMP     r1, #2
        BHS     SWILockBad      ; bad if lock now 2 or more.

      IF DEBUG = 1
        ; Keep track of the number of calls for debugging etc.
        LDR     r0, =angel_SysCallCount
        LDR     r1, [r0]
        ADD     r1, r1, #1
        STR     r1, [r0]
      ENDIF

        ; restore the CPU state to that at the beginning of the SWI.
        LDMFD   sp!, {r0, r1}
        
        EXCEPTENTRY SVCmode, Angel_GlobalRegBlock + (RB_SWI * Angel_RegBlockSize)
        
        ; The value of r14 is STILL as on entry, i.e. referring to the instruction
        ; after the SWI. r0 points at (the start of) the regblock referred to in
        ; the entry macro.
        
      IF DEBUG = 1
        STMFD   sp!, {r0-r3,lr}
        MOV     r1, #0
        MOV     r2, #DL_SWISave
        BL      Angel_DebugLog
        LDMFD   sp!, {r0-r3,lr}
      ENDIF
        
      IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
        
        MRS     r1, SPSR
        TST     r1, #Tbit
        BEQ     CompareWithARMSWI2

        LDRH    r1, [r14, #-2]          ; load the Thumb instruction
        BIC     r1, r1, #0xff00

        LogInfo "except.s", 717, LOG_EXCEPT, "Thumb SWI %lx\n"

        CMP     r1, #angel_SWI_THUMB
        BNE     UnrecognisedSWI
        B       ComplexAngelSWI
        
      ENDIF ; THUMB_SUPPORT

CompareWithARMSWI2
        LDR     r1, [r14, #-4]          ; load the ARM instruction
        BIC     r1, r1, #0xFF000000     ; condition & instr. code, which shouldn't be compared.

        LogInfo "except.s", 717, LOG_EXCEPT, "ARM SWI %lx\n"

        LDR     r2, =angel_SWI_ARM      ; load SWI number (+ SWI instr. code)
        EORS    r2, r2, r1              ; If the values are the same, we'll end up with zero
        BNE     UnrecognisedSWI
        
ComplexAngelSWI
        ; We now know it is our SWI, so we have to interpret the reason code
        ; which was in r0 on entry.
        ;
        ; Here we use the fact that r0 still addresses regblock (see above)
        ;

        LDR     r1, [r0, #RegOffsR0]    ; original r0

      IF MINIMAL_ANGEL = 0

        ; Only recognise the C Library SWIs if Semihosting is enabled
        LDR     r2, =angelOS_SemiHostingEnabled
        LDR     r2, [r2]
        CMP     r2, #0
        BEQ     NonCLibReasonCode
        
        ; Now we know semihosting is switched on
        LDR     r2, =angel_SWIreason_CLibBase
        CMP     r1, r2
        BLO     NonCLibReasonCode     ; orig r0 < angel_SWIreason_CLibBase
        LDR     r2, =angel_SWIreason_CLibLimit
        CMP     r1, r2
        BHI     NonCLibReasonCode     ; orig r0 > angel_SWIreason_CLibLimit

        ; The SWI code needs to run as a normal Angel task -- currently it
        ; will be flagged as TP_Application. Reset the priorities to be
        ; AngelCallBack and set a flag indicating we've done this.
        STMFD   sp!, {r0-r3, lr}
        
        BL      angel_EnterSWI
        
        LDMFD   sp!, {r0-r3, lr}
        
        ; Now we know it is a CLib SWI
        IMPORT  SysLibraryHandler
SysLibraryCall
        LDR     r2, =SysLibraryHandler
        B       DoIndirectCall

      ELSE                          ; minimal angel
        
        B       NonCLibReasonCode
      ENDIF

        ;; Literal pool here if needed.
        LTORG

        
        ; ****************************************************************
        ;
        ; void DoIndirectCall(angel_RegBlock *rb, void *arg, void (*fn)())
        ;
        ; On Entry:     
        ;    r0 contains the address of the caller's regblock
        ;    r1 is used by some functions (e.g. SysLibraryHandler) to pass
        ;       a parameter
        ;    r2 contains the address of the function to be called indirectly
        ;
        ; On Exit:
        ;    <<this code does not return in the normal way>>
        ;    It will return via the state in the SWI RegBlock.
        ; 
        ; ***************************************************************
        ; * This function must be entered in SVC mode from the SWI handler.
        ; ***************************************************************
        ; 
        ; It creates a new USR mode context for the function whose address
        ; is in r2. This function is passed a parameter in register r1
        ; (note this will be the *second* parameter in an APCS "C" function).
        ; 
        ; There is an assumption that a Complex SWI cannot be called as a
        ; result of this function being called. However, simple SWI's (such
        ; as EnterSVC) are allowed. The SWI nesting level is used to verify
        ; this constraint.
        ; 
        ; There is also the more severe assumption that a Complex SWI is
        ; not executed within Angel (by any means). This assumption means
        ; that no Angel stack is in use at the time of the SWI, which in turn
        ; means that the Angel SVC Stack must be available for use (note that
        ; the stack we used earlier *may have been* the application stack,
        ; depending whether the application uses it's own SVC mode stack).
        ; 
        ; With these assumptions, the context we saved on entry to the
        ; Complex SWI code can be used to return to the caller. All we need
        ; to do here is set up the USR mode context:
        ; 
        ;   R0 - R3          -- as for original SWI
        ;   R13usr           -- Caller's SP
        ;   R10usr           -- Caller's SP - <size>
        ;   LR               -- address of SysCallReturnVeneer
        ;   PC               -- value of r2 on entry
        ; 
        ; Note the 'fix' for SL. If the application is not using SL, it
        ; won't have a valid limit set up in SL. Hence, as Angel needs SL
        ; to be valid, we must synthesize a value. The simplest case is to
        ; assume there is space, and set SL = SP - <constant value>, where
        ; the simplest value is SVCStackSize. It might be appropriate to
        ; try to 'second guess' the existing SL to see if it could be a
        ; valid limit already, but that is prone to error too...
        ; 
        ; Finally, this call counts as a kind of 'end of interrupt' point,
        ; so we must flatten the SVC stack before we leave (in the same
        ; way SerialiseTask does for ISR's).
        
      IF MINIMAL_ANGEL = 0
        IMPORT  angel_EnterSWI

DoIndirectCall
        ; Flatten the SVC stack to use the Angel SVC stack.
        SetStackAndLimit Angel_SVCStackOffset, Angel_SVCStackLimitOffset

        LDR     r3, [r0, #RegOffsCPSR]  ; get the mode being returned to
        AND     r3, r3, #0xF            ; Mode Mask, 32/26 insensitive
        LDR     r4, =ContextLookuptable ; get the address of the r13 lookup
                                        ;  table (r13 is first)
        LDRB    r4, [r4, r3]            ; get SP offset for desired mode
        LDR     r3, [r0, r4]            ; get actual SP for mode

        ; this use of r10/sl is FIQ safe -- we're in SVC, going to USR, so
        ; r10 is always R10usr. See comments above for the purpose of this.
        SUB     sl, r3, #Angel_SVCStackSize
                                        ; get calculated SLusr from SP.
        
        ; This may be appropriate too:
        MOV     fp, #0                  ; start of call-frame
        
        LDR     r4, =SysCallReturnVeneer
        
        ; would normally be sp! in both, but we're not supposed to use
        ; this form of LDM with '!' so don't do either with '!'.
        STMFD   sp, {r3, r4}            ; save the new User SP, LR on Angel SVC stack
        SUB     r3, sp, #8
        LDMFD   r3, {r13, r14}^         ; Load back into R13usr, R14usr
        
        MRS     r3, cpsr
        BIC     r3, r3, #ModeMask
        ORR     r3, r3, #USRmode
        MSR     spsr_cf, r3

        LogInfo "except.s", 873, LOG_EXCEPT, "DoIndirectCall: jumping to routine\n"

        MOV     lr, r2                  ; get fn to call in safe place.

        ;; We are leaving the SWI code so we should remove lock - to ensure further SWI's
        ;; can be taken. r1 and r2 are currently spare so use them.
        ; LogInfo "except.s", 879, LOG_EXCEPT, "DoIndorectCall Givelock\n"
        ; GIVELOCK angel_ComplexSWILock, r1, r2
        
        ; r0 is still pointing at the regblock. Read in r1 - r3 from the
        ; regblock, then read r0 
        ADD     r0, r0, #RegOffsR1
        LDMIA   r0, { r1, r2, r3 }
        LDR     r0, [ r0, #RegOffsR0 - RegOffsR1 ] ; neg offset back to R0
        
        ; And leap into the indirectly called routine in USR mode
        MOVS    pc, lr

      ENDIF ; Minimal Angel

        
        LTORG
       
        ; ****************************************************************
        ;
        ; void SysCallReturnVeneer(unsigned retval)
        ;
        ; On Entry:     
        ;    r0 contains the return value from the syscall
        ;    r1-r9, r11, r12 are undefined.
        ;    r10 should be the SL value as written in DoIndirectCall
        ;    r13 should be the SP value as written in DoIndirectCall
        ;    r14 will be the address of this function.
        ;
        ; On Exit:
        ;    <<this code does not return in the normal way>>
        ;    It will return via the state in the SWI RegBlock.
        ; 
        ; ***************************************************************
        ; * This function expects to be entered in USR mode.
        ; ***************************************************************
        ;
        ; 
      IF MINIMAL_ANGEL = 0

        IMPORT  angel_ExitSWI

SysCallReturnVeneer   
        ; Save the return value in R0 from the SWI into the regblock
        LDR     r7, =Angel_GlobalRegBlock + (RB_SWI * Angel_RegBlockSize)
        STR     r0, [r7, #RegOffsR0]

        ; Need to be in a privileged mode to do the restore; use EnterSVC to
        ; do this.  (This is ok, as EnterSVC is a Simple SWI, not a Complex
        ; one, and thus doesn't use the SWI regblock).
        LDR     r0, =angel_SWIreason_EnterSVC
        SWI     angel_SWI_ARM

        LogInfo "except.s", 927, LOG_EXCEPT, "SysCallReturnVeneer Givelock\n"
        
        GIVELOCK angel_ComplexSWILock, r0, r1

        ; restore task priorities and the 'in swi' flag.
        STMFD   sp!, {r0-r3, lr}
        BL      angel_ExitSWI
        LDMFD   sp!, {r0-r3, lr}

        LDR     r0, =angel_SWIDeferredBlock
        LDR     r1, [r0]
        CMP     r1, #0
        BNE     SysCallDeferredBlock
        
        ; Now in SVC mode. r7 should still point to the register block,
        ; so get it into r0 and branch to StartTask to return to the
        ; called function
        
        LDR     r0, =Angel_GlobalRegBlock + (RB_SWI * Angel_RegBlockSize)
        B       angel_StartTask
        
SysCallDeferredBlock

        ; first off, zero deferred block flag ready for next time
        MOV     r1, #0
        STR     r1, [r0]

        ; now get the appl TQI, and set the signal bits to block it
        BL      Angel_AccessApplicationTask
        MOV     r1, #TS_Blocked
        STR     r1, [r0, #TQI_State]
        MOV     r1, #1
        STR     r1, [r0, #TQI_SignalWaiting]

        ; now save the SWI regblock in the appl tqi
        MOV     r1, r0          ; put tqi ptr from access into r1
                                ; and SWI regblock ptr into r0
        LDR     r0, =Angel_GlobalRegBlock + (RB_SWI * Angel_RegBlockSize)
        BL      angel_QueueTask ; for call to queue

        ; and look for another task to run.
        B       angel_SelectNextTask
        
      ENDIF
        
        ; **********************************************************

        ; Deal with cases other than the semi-hosted CLib SWI
        ; r0 = our regblock with the saved regs stored
        ; r1 = the original r0 when we get here
NonCLibReasonCode

      IF MINIMAL_ANGEL = 0
        IMPORT  angel_ApplDeviceHandler

        ;; ApplDevice only known on full Angel, non-EmbeddedICE systems
        CMP     r1, #angel_SWIreason_ApplDevice
        BNE     %F1
        
        LDR     r2, =angel_ApplDeviceHandler
        B       DoIndirectCall
        
1
      ENDIF
        
        LDR     r2, =angel_SWIreason_ReportException
        CMP     r1, r2
        BNE     UnrecognisedSWI
        
      IF MINIMAL_ANGEL <> 0
        ; We cannot report an exception within ICEMan
ReportException
        B       ReportException
      ELSE
        ; this is a bit early (we haven't quite finished with the regblock, as
        ; SerialiseTask wants it), but ok as we won't enable ints until it
        ; really is ok.
        LogInfo "except.s", 1012, LOG_EXCEPT, "ReportException Givelock\n"
        
        GIVELOCK angel_ComplexSWILock, r0, r2 
        
        ; The ADP_Stopped code was in r1 before the SWI happened -- load it
        LDR     r0, =Angel_GlobalRegBlock + (RB_SWI * Angel_RegBlockSize)
        LDR     r2, [r0, #RegOffsR1]
        BL      CallSerialiseTask
      ENDIF

UnrecognisedSWI
      IF MINIMAL_ANGEL<>0
        ; We cannot report an unrecognised SWI within ICEMan
        B       UnrecognisedSWI
        
      ELSE
        ; this is a bit early (we haven't quite finished with the regblock, as
        ; SerialiseTask wants it), but ok as we won't enable ints until it
        ; really is ok.
        LogInfo "except.s", 1031, LOG_EXCEPT, "UnrecognisedSWI Givelock\n"
        
        GIVELOCK angel_ComplexSWILock, r0, r2
        
        LDR     r0, =Angel_GlobalRegBlock + (RB_SWI * Angel_RegBlockSize)
        LDR     r2, =ADP_Stopped_SoftwareInterrupt
        BL      CallSerialiseTask

      ENDIF

        END
