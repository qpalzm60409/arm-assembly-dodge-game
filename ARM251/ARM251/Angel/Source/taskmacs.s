        SUBT    Macros for serialiser/exception handling                > taskmacs.s
        ; ---------------------------------------------------------------------
        ;
        ; $Revision: 1.2.2.8 $
        ;   $Author: rivimey $
        ;     $Date: 1998/10/23 15:52:41 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ; ---------------------------------------------------------------------

        ASSERT  (listopts_s)
old_opt SETA    {OPT}
        OPT     (opt_off)   ; disable listing of include files

        ; ---------------------------------------------------------------------

        [       (:LNOT: :DEF: taskmacs_s)

                GBLL    taskmacs_s
taskmacs_s      SETL    {TRUE}


        ; define the register block structure:
                ^ 0

RegOffsR0       # 4             ; offs: 0
RegOffsPC       # 4

RegOffsSPSRfiq  # 4             ; offs: 8
RegOffsR8fiq    # 4
RegOffsR9fiq    # 4
RegOffsR10fiq   # 4
RegOffsR11fiq   # 4
RegOffsR12fiq   # 4
RegOffsR13fiq   # 4
RegOffsR14fiq   # 4

RegOffsSPSRirq  # 4             ; offs: 40 (x28)
RegOffsR13irq   # 4
RegOffsR14irq   # 4

RegOffsSPSRsvc  # 4             ; offs: 52 (x34)
RegOffsR13svc   # 4
RegOffsR14svc   # 4

RegOffsSPSRabt  # 4             ; offs: 64 (x40)
RegOffsR13abt   # 4
RegOffsR14abt   # 4

RegOffsSPSRund  # 4             ; offs: 76 (x4c)
RegOffsR13und   # 4
RegOffsR14und   # 4

RegOffsCPSR     # 4             ; offs: 88 (x58)
RegOffsR1       # 4
RegOffsR2       # 4
RegOffsR3       # 4
RegOffsR4       # 4
RegOffsR5       # 4
RegOffsR6       # 4
RegOffsR7       # 4
RegOffsR8usr    # 4             ; offs: 120 (x78)
RegOffsR9usr    # 4
RegOffsR10usr   # 4
RegOffsR11usr   # 4
RegOffsR12usr   # 4
RegOffsR13usr   # 4
RegOffsR14usr   # 4

RegBlockSize    # 0             ; size: 148 (x94)
        
                ^ 0


        ; Codes for the Angel_DebugLog code
        IF DEBUG <> 0
DL_StartTask    EQU 1
DL_QueueTask    EQU 2
DL_IntHandler   EQU 3
DL_SaveTask     EQU 4
DL_YieldSave    EQU 5
DL_SWISave      EQU 6
DL_UndefSave    EQU 7
DL_WaitSave     EQU 8
DL_AbortSave    EQU 9
DL_DeleteTask   EQU 10
        ENDIF
        
        MACRO
        R13LookupTable

        ; r13 offsets by mode, offset by 2 words (see code in EXCEPTEXIT)
        DCB     RegOffsR13usr - RegOffsSPSRfiq
        DCB     RegOffsR13fiq - RegOffsSPSRfiq
        DCB     RegOffsR13irq - RegOffsSPSRfiq
        DCB     RegOffsR13svc - RegOffsSPSRfiq
        DCB     0, 0, 0
        DCB     RegOffsR13abt - RegOffsSPSRfiq
        DCB     0, 0, 0
        DCB     RegOffsR13und - RegOffsSPSRfiq
        DCB     0, 0, 0
        DCB     RegOffsR13usr - RegOffsSPSRfiq  ; sys mode

        MEND

        ; ---------------------------------------------------------------------
        ; STOREMODE 
        ; -----------
        ;
        ; STOREMODE  modenum
        ;
        ; Using $temp1, $temp2 as scratch, transfer a register frame {spsr, r13, r14}
        ; to the memory at $basereg from the registers for mode $modenum. Increment
        ; r0 over the register block. $cpsrreg has a 'blank' copy of the CPSR, which
        ; should have ints disabled and the mode bits zeroed.
        ;
        MACRO
        STOREMODE $modenum, $basereg, $cpsrreg, $temp1, $temp2

        ORR     $temp1, $cpsrreg, $modenum
        MSR     cpsr_cf, $temp1
        MRS     $temp2, spsr
        STMIA   $basereg!, {$temp2, r13, r14}
        
        MEND


        ; ---------------------------------------------------------------------
        ; EXCEPTENTRY_PART1
        ; -----------
        ; EXCEPTENTRY_PART1 $handler, $regblock
        ;
        ; Code to handle exception handler entry. Parameters include the handler
        ; mode (e.g. IRQmode) and the address to store the registers.
        ;
        ; On Entry:
        ;    sp_<entry mode> should point at an FD stack of a few words.
        ;    CPSR is assumed to have IRQ disabled (as will be the case for
        ;         all exception handlers).
        ;
        ; On Exit:
        ;    r0 will be left pointing into the regblock. r7 contains the original
        ;    value of the CPSR; both are needed by EXCEPTENTRY_PART2.
        ;
        ; After this call, code may use r4-r6 to pass information through to the
        ; code after the EXCEPTENTRY_PART2. Other registers, especially other
        ; modes registers, should be left well alone. Be aware that the whole
        ; sequence is performed interrupts-disabled.
        ;
        
        MACRO
        EXCEPTENTRY_PART1 $handler, $regblock

        STMFD   sp!, {r0}               ; save r0 ...
        MRS     r0, cpsr

      IF $handler <> FIQmode
        ORR     r0, r0, #FIQDisable     ;  disable FIQ's for the moment.
        MSR     cpsr_cf, r0
      ENDIF
        
        STMFD   sp!, {r0, r8}           ; cpsr and r8 on exception handler stack

        ;
        ; save the unbanked regs -- use r8 to address the save area while
        ; we're saving r0 to r7, then move to use r0 for the banked registers
        ;
        LDR     r8, =$regblock + RegOffsCPSR
        MRS     r0, spsr                ; use the SPSR as the saved CPSR value.
        STMIA   r8, {r0-r7}             ; save cpsr along with r1-7 
        SUB     r0, r8, #RegOffsCPSR    ; move base from banked r8 into unbanked r0
                                        ; r0 now addresses RegOffsR0
        LDMFD   sp!, {r7, r8}           ; get orig cpsr, r8 from stack
        LDMFD   sp!, {r1}               ; get orig r0 from stack

        ;
        ; arrange for r14 to point to the 'right' place to return to; it is stored
        ; in the location used to restore PC from, as well as in the 'r14' slot.
        ;
      IF $handler = FIQmode :LOR: $handler = IRQmode
        SUB     r14, r14, #4
      ENDIF
        
      IF  $handler = UNDmode
        SUB     r14, r14, #4    ; This is correct for ARM state only, Thumb State requires
                                ; the LR readjusting for the Thumb increments ( +2)
        
        IF :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
        
          MRS   r2, spsr        ; Test for Thumb state in the saved status register
          TST   r2, #Tbit
          ADDNE r14, r14, #2    ; If Thumb set, add 2 to the lr

        ENDIF                   ; Thumb Support
        
      ENDIF                     ; Undef Handler
        
      IF $handler = ABTmode
        ; The offset of 4 is ok assuming that we accept the abort and continue rather
        ; than trying to fix it, which is OK in Angel as it stands.
        ;
        ; If Angel is changed such that data aborts must be retried, code must be
        ; inserted to distinguish the data abort from a prefetch abort and subtract
        ; #8 rather than #4 before retrying the instruction. 
        SUB     r14, r14, #4
      ENDIF

        MEND
        
        ; ---------------------------------------------------------------------
        ; EXCEPTENTRY_PART2
        ; -----------
        ; EXCEPTENTRY_PART2
        ;
        ; Code to complete exception handler entry. Theis code performs the
        ; handler-independent part of the sequence, saving the various modes'
        ; registers to the regblock. USR mode has already been done. This macro
        ; must be entered in a privileged mode, and EXCEPTENTRY_PART1 must have
        ; been called to set up 'r0' and the other parts of the regblock.
        ;
        ; On Entry:
        ;    r0 must point to the start of the FIQ register block in the regblock
        ;    structure. r0 - r14usr have been saved. r7 contains the original
        ;    value of the CPSR. r4 - r6 may contain useful info and must be
        ;    preserved.
        ;
        ; On Exit:
        ;    r0 will be left pointing at the base of the regblock, which will.
        ;    contain the CPU register state on entry to ENTEREXCEPT_PART1.
        
        MACRO
        EXCEPTENTRY_PART2

        STMIA   r0!, {r1, r14}          ; save r0, lr (as return PC) in memory

        ADD     r2, r0, #RegOffsR8usr - RegOffsSPSRfiq
        STMIA   r2, {r8-r14}^           ; save USR mode registers
        
        ; have now saved unbanked registers. Cycle through the modes and save
        ; the banked ones

        MRS     r1, cpsr                ; make a copy of cpsr with mode bits zero
        BIC     r1, r1, #ModeMask       ; so we can OR in the right value.

        ORR     r2, r1, #FIQmode        ; 
        MSR     cpsr_cf, r2                ; shift to FIQ mode
        MRS     r3, spsr                ; get SPSRfiq
        STMIA   r0!, {r3, r8-r14}       ; save FIQ's banked registers

        ; save the state for the other modes... (input r0, r1, uses r2, r3)
        STOREMODE #IRQmode, r0, r1, r2, r3
        STOREMODE #SVCmode, r0, r1, r2, r3
        STOREMODE #ABTmode, r0, r1, r2, r3
        STOREMODE #UNDmode, r0, r1, r2, r3

        ; restore the mode back to that on entry to the handler
        ; (we kept the cpsr we saved in r7 in the LDMFD)
        MSR     cpsr_cf, r7
        
        ; restore r0 to point at the start of the regblock again.
        SUB     r0, r0, #RegOffsCPSR
        
        MEND

        ; ---------------------------------------------------------------------
        ; EXCEPTENTRY
        ; -----------
        ; EXCEPTENTRY $handler, $regblock
        ;
        ; Code to handle exception handler entry. Parameters include the handler
        ; mode (e.g. IRQmode) and the address to store the registers.
        ;
        ; On Entry:
        ;    sp_<entry mode> should point at an FD stack of a few words.
        ;    CPSR is assumed to have IRQ disabled (as will be the case for
        ;         all exception handlers).
        ;
        ; On Exit:
        ;    r0 will be left pointing at the base of the regblock.
        ;
        
        MACRO
        EXCEPTENTRY $handler, $regblock
        
        EXCEPTENTRY_PART1 $handler, $regblock
        EXCEPTENTRY_PART2
        
        MEND

        ; ---------------------------------------------------------------------
        ; SAVETASK 
        ; -----------
        ; SAVETASK  $regblock, $inregzero
        ;
        ; Code to save the task context. This differs from the EXCEPTENTRY
        ; macro in that it assumes that we start in SVC mode. The task
        ; resumes at the address specified in LR on entry.
        ;
        ; On Entry:
        ;    sp_<entry mode> should point at an FD stack of a few words.
        ;    CPSR is assumed to specify a privileged mode.
        ;    $regblock  is the address to store the registers, if a constant.
        ;    $inregzero is a flag -
        ;        if non-zero, the regblock address is in R0 on entry
        ;        if zero, the regblock address is defined by parameter 1
        ;
        ; On Exit:
        ;    r0 will be left pointing at the base of the regblock.
        ;
        
        MACRO
        SAVETASK $regblock, $inregzero

        STMFD   sp!, {r0}               ; save r0 ...
        MRS     r0, cpsr
        STMFD   sp!, {r0, r8}           ; cpsr and r8 on current mode's stack
        
        ;; ENTER CRITICAL SECTION!
        ORR     r0, r0, #IRQDisable + FIQDisable
        MSR     cpsr_cf, r0        

        ;
        ; save the unbanked regs -- use r8 to address the save area while
        ; we're saving r0 to r7, then move to use r0 for the banked registers
        ;
      IF $inregzero <> 0
        LDR     r8, [sp, #8]            ; load original r0, the ptr to regblock,
        ADD     r8, r8, #RegOffsCPSR    ; from stack and add offset...
      ELSE
        LDR     r8, =$regblock + RegOffsCPSR ; else it's a macro parameter
      ENDIF
        
        MRS     r0, spsr                ; use the SPSR as the saved CPSR value.
        STMIA   r8, {r0-r7}             ; save cpsr along with r1-7 
        SUB     r0, r8, #RegOffsCPSR    ; move base from banked r8 into r0
                                        ; r0 now addresses RegOffsR0
        LDMFD   sp!, {r7, r8}           ; get orig cpsr, r8 from stack
        LDMFD   sp!, {r1}               ; get orig r0 from stack

        ; in the following code, we use r2 temporarily as a base register,
        ; as r0 will, after saving (r0, pc), be pointing at the right place
        ; for the FIQ save.
        
        STMIA   r0!, {r1, r14}          ; save r0, LR to return PC
        ADD     r2, r0, #RegOffsR8usr - RegOffsSPSRfiq
        STMIA   r2, {r8-r14}^           ; save USR mode registers

        ; have now saved unbanked registers. Cycle through the modes and save
        ; the banked ones

        MRS     r1, cpsr                ; copy cpsr with mode bits zero
        BIC     r1, r1, #ModeMask       ; so we can OR in the right value.

        ORR     r2, r1, #FIQmode        ; 
        MSR     cpsr_cf, r2                ; shift to FIQ mode
        MRS     r3, spsr                ; get SPSRfiq
        STMIA   r0!, {r3, r8-r14}       ; save FIQ's banked registers

        ; save the state for the other modes... (input r0, r1, uses r2, r3)
        STOREMODE #IRQmode, r0, r1, r2, r3
        STOREMODE #SVCmode, r0, r1, r2, r3
        STOREMODE #ABTmode, r0, r1, r2, r3
        STOREMODE #UNDmode, r0, r1, r2, r3

        ; restore the mode (incl. interrupt flags) back to that on entry
        MSR     cpsr_cf, r7
        ;; LEFT CRITICAL SECTION!

        ; restore r0 to point at the start of the regblock again.
        SUB     r0, r0, #RegOffsCPSR
        
        MEND

        ; ---------------------------------------------------------------------
        ; RESTOREMODE 
        ; -----------
        ;
        ; RESTOREMODE  modenum, $basereg, $cpsrreg, $temp1, $temp2
        ; 
        ; Using two regs as scratch, transfer a register frame {spsr, r13, r14}
        ; from the memory at r0 to the registers for mode $modenum. Increment
        ; $basereg over the register block transferred. $cpsrreg must have a
        ; 'blank' copy of the CPSR.
        ; 
        MACRO
        RESTOREMODE $modenum, $basereg, $cpsrreg, $temp1, $temp2

        ORR     $temp1, $cpsrreg, $modenum
        MSR     cpsr_cf, $temp1
        LDMIA   $basereg!, {$temp2, r13, r14}
        MSR     spsr_cf, $temp2

        MEND


        ; ---------------------------------------------------------------------
        ; EXCEPTEXIT 
        ; -----------
        ; EXCEPTEXIT
        ; 
        ; Code to handle exception handler exit. The macro expects that r0
        ; points to the register context block to restore into the processor,
        ; interrupts are disabled, and the current mode is privileged.
        ;
        ; It assumes that in the context being returned to, there are 2 words
        ; of unused memory at *r13 and *(r13 - 4).

        MACRO
        EXCEPTEXIT
        
        ; firstly, we must modify the SP value to account for the two words
        ; we put on the task's stack when we exit. Which means finding the
        ; right SP first...
        LDR     r1, [r0, #RegOffsCPSR]          ; mode being returned to
        LDR     r4, =R13ContextLookuptable      ; get the address of the r13
                                                ;  lookup table
        AND     r1, r1, #0xF                    ; Mode Mask, 32/26 insensitive
        ; get the SP regblock offset for desired mode
        LDRB    r4, [r4, r1]

        ; now we have SP's offset. Get the two values we want on the task's
        ; stack and read SP into r1. Note that we use the incremented r0, so:
        ; THE R13 LOOKUP TABLE IS OFFSET BY 2 WORDS (see SERLASM)!
        LDMIA   r0!, {r2, r3}
        LDR     r1, [r0, r4]

        ; Now:  r0 -> regblock+8,
        ;       r1 == task's SP,
        ;       r2 == task's R0,
        ;       r3 == task's PC.
        ; Store the R0, PC on the task's stack, incrementing SP, and save this
        ; SP back into the regblock.
        STMFD   r1!, {r2, r3}
        STR     r1, [r0, r4]

        ; get the current CPSR and clear the mode bits so we can ORR in
        ; the desired mode value later.
        MRS     r1, cpsr
        BIC     r1, r1, #ModeMask

        ; Restore first the FIQ state (incl. r8-12), then IRQ, SVC, ABT and UND
        ; states (via the macro) into the regblock
        ORR     r2, r1, #FIQmode
        MSR     cpsr_cf, r2
        LDMIA   r0!, {r3, r8-r14}
        MSR     spsr_cf, r3

        RESTOREMODE #IRQmode, r0, r1, r2, r3
        RESTOREMODE #SVCmode, r0, r1, r2, r3
        RESTOREMODE #ABTmode, r0, r1, r2, r3
        RESTOREMODE #UNDmode, r0, r1, r2, r3

        ; nearly there... restore the USR mode registers (except that r0 isn't
        ; r0, but CPSR) and check for Thumb state
        LDMIA   r0, {r0-r14}^
        [ :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
        TST     r0, #Tbit
        BNE     %F1
        ]
        ; not Thumb, so shift to the right mode and return, via the registers
        ; we stacked earlier.
        ; NOTE: Interrupts may (and can) happen after the MSR!
        
        ; NOP is inserted to ensure no assembler warning for MSR after banked
        ; register operation
                
        NOP     
        MSR     cpsr_cf, r0
        LDMFD   r13!, {r0, pc}

        ; We need to return to Thumb state. Clear the T bit as we're not
        ; allowed to change it with MSR, shift to the desired mode (where,
        ; again, we may end up taking an interrupt) ...
1
        [ :DEF: THUMB_SUPPORT :LAND: THUMB_SUPPORT<>0
        BIC     r0, r0, #Tbit
        MSR     cpsr_cf, r0
        
        ; and BX to a POP which will return to the task.
        ADR     r0, %F2+1
        ;ADD     r0, r0, #1 ; Don't forget we need to set bit 0 for a return
                           ; to Thumb state 
        BX      r0
        CODE16
2
        POP     {r0, pc}

        CODE32
        ]
        MEND


        ; ---------------------------------------------------------------------
        ; RESUMETASK
        ; -----------
        ; RESUMETASK
        ; 
        ; Code to handle task return. The macro expects that r0 points
        ; to the register context block to restore into the processor, that
        ; interrupts are disabled, and that the current mode is privileged.
        ;
        ; It assumes that in the context being returned to, there are 2 words of
        ; unused memory at *r13 and *(r13 - 4).
        ;
        ; It is implemented using EXCEPTEXIT because the code is currently
        ; identical.
        ;

        MACRO
        RESUMETASK

        EXCEPTEXIT

        MEND


        ; ---------------------------------------------------------------------
        ; TAKELOCK
        ; -----------
        ; TAKELOCK $lockvar, $reg1, $reg2
        ; 
        ; Set the lock value addressed by 'lockvar' by adding 1 to it's existing
        ; value. Leave the stored value in $reg2. $reg1 is temporary.
        ;

        MACRO
        TAKELOCK $lockvar, $reg1, $reg2
        LDR     $reg1, =$lockvar 
        LDR     $reg2, [$reg1]
        ADD     $reg2, $reg2, #1
        STR     $reg2, [$reg1]
        MEND
                
        ; ---------------------------------------------------------------------
        ; GIVELOCK
        ; -----------
        ; GIVELOCK $lockvar, $reg1, $reg2
        ; 
        ; Return the lock value addressed by 'lockvar' by subtracting 1 from
        ; it's existing value. Leave the stored value in $reg2. $reg1 is temporary.
        ;

        MACRO
        GIVELOCK $lockvar, $reg1, $reg2
        LDR     $reg1, =$lockvar 
        LDR     $reg2, [$reg1]
        CMP     $reg2, #0
        BNE     %F1
        
        FatalError "Givelock 0."
1       
        SUB     $reg2, $reg2, #1
        STR     $reg2, [$reg1]
        MEND

        ; ---------------------------------------------------------------------
        ; SetStackAndLimit
        ; -----------
        ; SetStackAndLimit $offset, $limit
        ; 
        ; Set up the current mode's SP and SL to the given offsets from the Angel
        ; stack base. $offset should be a value such as Angel_SVCStackOffset and
        ; $limit a value such as Angel_SVCStackLimitOffset.
        ;
        
        MACRO
        SetStackAndLimit $offset, $limit
        [ $offset <> 0
        LDR     sl, =Angel_StackBase
        LDR     sl, [sl]
        ADD     sp, sl, #$offset
        ADD     sl, sl, #$limit
        |       
        LDR     sl, =Angel_StackBase
        LDR     sp, [sl]
        ADD     sl, sp, #$limit
        ]
        MEND
        
        ; ---------------------------------------------------------------------
        ; SetStack
        ; --------
        ; SetStack $offset
        ; 
        ; Set up the current mode's SP to the given offset from the Angel
        ; stack base. $offset should be a value such as Angel_SVCStackOffset
        ;

        MACRO
        SetStack $offset
        LDR     sp, =Angel_StackBase
        LDR     sp, [sp]
        [ $offset <> 0
        ADD     sp, sp, #$offset
        ]
        MEND


       
        ; ---------------------------------------------------------------------
        ; EnsureFIQDisabled
        ; -----------------
        ; EnsureFIQDisabled $psr, $temp
        ; 
        ; This macro is used to disable FIQ in those circumstances, such as
        ; exception handlers, where we already know IRQ is disabled. At the
        ; moment, this only needs to happen when we are servicing interrupts
        ; on FIQ (possibly in addition to IRQ).
        ;

        MACRO
        EnsureFIQDisabled $psr, $temp
        
      IF (HANDLE_INTERRUPTS_ON_FIQ = 1) 
        
        MRS     $temp, $psr
        ORR     $temp, $temp, #FIQDisable
        MSR     $psr._cf, $temp
        
      ENDIF

        MEND    
        
        ; ---------------------------------------------------------------------
        ; DisableAngelInts
        ; ----------------
        ; DisableAngelInts $cpsr_copy, $res_reg
        ; 
        ; On the assumption that $cpsr_copy is an ordinary register with a
        ; copy of the CPSR register for some mode, change it's value by
        ; disabling (setting) those interrupt bits which Angel 'cares' about.
        ;


        MACRO
        DisableAngelInts $cpsr_copy, $res_reg
        
        ORR   $res_reg, $cpsr_copy, #AngelInterruptMask
        
        MEND

        ; ---------------------------------------------------------------------
        ; EnableAngelInts
        ; ---------------
        ; EnableAngelInts $cpsr_copy, $res_reg
        ; 
        ; On the assumption that $cpsr_copy is an ordinary register with a
        ; copy of the CPSR register for some mode, change it's value by
        ; enabling (clearing) those interrupt bits which Angel requires.
        
        MACRO
        EnableAngelInts $cpsr_copy, $res_reg
        
        BIC     $res_reg, $cpsr_copy, #AngelInterruptMask       
        
        MEND
        
        ; ---------------------------------------------------------------------

        ]       ; taskmacs_s

        OPT     (old_opt)   ; restore previous listing options

        ; ---------------------------------------------------------------------
        END     ; EOF taskmacs_s

