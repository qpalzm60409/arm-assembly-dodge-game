        TTL     Angel assembler support routines                    > suppasm.s
        ; ---------------------------------------------------------------------
        ; This source files holds the general assembler routines
        ; needed by Angel.
        ;
        ; $Revision: 1.3.2.3 $
        ;   $Author: rivimey $
        ;     $Date: 1998/10/23 15:57:46 $
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

        AREA    |C$$Code$$InterruptSupport|,CODE,READONLY
        KEEP

        [       ({CONFIG} = 26)
        ASSERT  (1 = 0) ; This code has been written for 32bit mode only
        ]


        ; ---------------------------------------------------------------------

        IMPORT  angel_IntHandler        ; Table of Interrupt handlers
        IMPORT  Angel_GlobalRegBlock
        IMPORT  angel_GhostCount
        IMPORT  angel_InterruptCount
        IMPORT  HandlerSWI
        IMPORT  Angel_StackBase
        IMPORT  __rt_asm_fatalerror
        
        EXPORT  angel_FIQInterruptHandler
        EXPORT  angel_IRQInterruptHandler
        
        IMPORT  HandlerUndef
        

        ; ---------------------------------------------------------------------
        ; The following is a word which gets set to either ADP_CPU_LE or
        ; ADP_CPU_BE at assemble time.  It can then be used by Angel to
        ; tell the host what the endianess of the system is.

        EXPORT  angel_Endianess

angel_Endianess
        IF {ENDIAN} = "big"
          DCD ADP_CPU_BE
          DCD ADP_CPU_BigEndian
        ELSE
          IF {ENDIAN} = "little"
            DCD ADP_CPU_LE
            DCD 0
          ELSE
            ! ERROR - could not detect endianess
          ENDIF
        ENDIF

        
        ; ---------------------------------------------------------------------

        
angel_FIQInterruptHandler

      IF HANDLE_INTERRUPTS_ON_FIQ = 0
        
        ;; we're not supposed to be handling this, so don't
        SUBS  pc, lr, #4
        
      ELSE
        ; Nasty case code - check to see if we have interrupted the first
        ; few instructions of the SWI handler or UND handler!!!
        ; which includes the vectors themselves !!!  This is necessary
        ; because on entry to SWI and UNDEF handlers FIQ is still enabled.

        ; It is therefore highly recommendsed that FIQ is not used to
        ; handle Angel Device interrupts, as this is a significant overhead
        ; and is also really quite unpleasant!
        STMFD sp!, {r0}       ; need r0 as temp -- save it.
        
        CMP   lr, #0x1c       ; 0x1c is the last vector. If LR is less than
                                ; this, we're in trouble...
        BCC   ReallyNastyCase ; taken if FIQ from Reset, Und, Swi, Pre/Dat Abort
                                ; but not from IRQ/FIQ

        LDR   r0, =HandlerSWI
        CMP   lr, r0
        BCC   CheckUNDHandler ; interrupted pc < HandlerSWI
        ADD   r0, r0, #60
        CMP   lr, r0
        BHI   CheckUNDHandler ; interrupted pc >> HandlerSWI
        B     ReallyNastyCase

CheckUNDHandler
       IF MINIMAL_ANGEL <> 0
        B     ContinueFIQHandler   ; don't worry about UND on Minimal
                                   ; as we don't manage breakpoints here.
       ELSE
        
        LDR   r0, =HandlerUndef
        CMP   lr, r0
        BCC   ContinueFIQHandler   ; interrupted pc < HandlerUndef
        ADD   r0, r0, #60
        CMP   lr, r0
        BHI   ContinueFIQHandler   ; interrupted pc >> HandlerUndef
        
       ENDIF

ReallyNastyCase
        ; This is the nasty case - ignore the interrupt
        ; and resume the SWI Handler with IRQ and FIQ disabled.
        EnsureFIQDisabled spsr, r3
        
        LDMFD sp!, {r0}       ; restore our temp register,
        SUBS  pc, lr, #4      ;  and return...

ContinueFIQHandler

        LDMFD sp!, {r0}       ; restore our temp register,
        EXCEPTENTRY FIQmode, Angel_GlobalRegBlock + \
                        (RB_Interrupted * Angel_RegBlockSize)
        
      IF :DEF: R10_IS_SL
        ; Set up sl for the appropriate stack (depends on mode). This is
        ; NOT a banked register, so it won't be remembered anyway.
        LDR     sl, =Angel_StackBase
        LDR     sl, [sl]
        ADD     sl, sl, #Angel_FIQStackLimitOffset
      ENDIF

        B       AngelInterruptHandler
        
      ENDIF   ; HANDLE_INTERRUPTS_ON_FIQ <> 0

        ; ---------------------------------------------------------------------

        
      IF :DEF: R10_IS_SL
irqstackoverflow
        FatalError "IRQ: Stack Overflow."
      ENDIF
                
        
angel_IRQInterruptHandler

      IF HANDLE_INTERRUPTS_ON_IRQ = 0

        ;; we're not supposed to be handling this, so don't
        SUBS  pc, lr, #4
        
      ELSE
        
        EXCEPTENTRY IRQmode, Angel_GlobalRegBlock + \
                        (RB_Interrupted * Angel_RegBlockSize)

      IF :DEF: R10_IS_SL
        ; The stack limit is not used in standard Angel: this code is here to
        ; show where and how to set up SL.
        ;
        ; Set up sl for the appropriate stack (depends on mode). This is
        ; NOT a banked register, so it won't be remembered anyway.
        LDR     sl, =Angel_StackBase
        LDR     sl, [sl]
        ADD     sl, sl, #Angel_IRQStackLimitOffset
      ENDIF

        ;; fall through...
        ;B      AngelInterruptHandler
        
      ENDIF   ; HANDLE_INTERRUPTS_ON_IRQ <> 0


        ; ---------------------------------------------------------------------

AngelInterruptHandler
        IMPORT  Angel_DebugLog

      IF DEBUG <> 0
        MOV     r1, #0
        MOV     r2, #3
        BL      Angel_DebugLog
      ENDIF     

      IF :DEF: R10_IS_SL
        ; standard overflow check 
        CMP     sp, sl
        BLLO    irqstackoverflow
      ENDIF

      IF DEBUG = 1
        LDR     r1, =angel_InterruptCount
        LDR     r0, [r1]
        ADD     r0, r0, #1
        STR     r0, [r1]
      ENDIF
        
GetSrc  GETSOURCE r0, lr                ; Get the interrupt vector index in r0
        CMN     r0, #1                    ; Check for Ghost Interrupt
        BEQ     GhostInterrupt            ; It's a Ghost
        CMP     r0, #DE_NUM_INT_HANDLERS  ; Check if a valid source index
        BGE     UnrecognisedSource        ; Unrecognised source

        ; The array angel_IntHandler contains pairs (fn, dat) of function
        ; addresses and data values to pass to those functions. Calculate
        ; the offset into the table and load the values into r1 (data) and
        ; r4 (function addr).
        ; The function will be called as:
        ; 
        ;   (void) handler(vector_num, data, empty_stack)
        ;             r4     r0         r1      r2
        ; 
        
        LDR     r2, =angel_IntHandler     ; Reference the handlers vector
        ADD     r2, r2, r0, LSL #3        ; r2 = angel_IntHandler + r0 * 8
        
        LDMIA   r2, {r4, r5}              ; load the required func pointer ...
        MOV     r1, r5                    ;   and data into r4 and r1

        ; r2, r5 are now dead.
        
        TEQ     r4, #0
        BEQ     UnrecognisedSource        ; No handler attached for source

        ; We can now call the APCS-3 handler for this source:
        
        MOV     r2, sp                    ; return this mode's stack here
                                          ; when we're finished.
        
      IF DEBUG <> 0
        MOV     fp, #0                    ; start of call-frame -- for debug backtrace
      ENDIF
        
        ; r0 = interrupt vector index, set up above
        ; r1 = data from table, set up above
        ; r2 = empty_stack (stack is [assumed to be] currently empty)
        ; r3 = 0 [not used]
        ;
        ; r4 = address of Interrupt handler, which may return but
        ;      WON'T return if it calls the serialiser.
        ; r5 - r9 undefined.
        ; 
        ; fp(r11) = 0 => base of frame set
        ; sl = Limit for IRQ/FIQ stack -- calculated above
        ; sp = [probably at base of IRQ/FIQ stack]

        ; It is assumed that the handler function called will execute
        ; within the interrupt handler stack allocation, and the code
        ; will not manipulate the interrupt mask status, or the SPSR
        ; register:
        LDR     lr, =ReturnFromIntHandler  ; generate return address
        MOV     pc, r4                     ; call handler function

      IF ASSERT_ENABLED <> 0
badintmode
        FatalError "Interrupt return in USR mode!\n"
      ENDIF

UnrecognisedSource
        NOP                             ; this way we get two labels in the debugger!
        ;; fall through to...
        
ReturnFromIntHandler
        ; We will only get here if the interrupt handler did not call
        ; SerialiseTask to perform some 'hard' work.
                
      IF ASSERT_ENABLED <> 0
        MRS     r0, CPSR
        AND     r0, r0, #ModeMaskUFIS
        CMP     r0, #USRmode :AND: ModeMaskUFIS
        BEQ     badintmode
      ENDIF
        
        LDR     r0, =Angel_GlobalRegBlock + \
                        (RB_Interrupted * Angel_RegBlockSize)
      IF MINIMAL_ANGEL = 0

        IMPORT  angel_StartTask

        B       angel_StartTask

      ELSE

        IMPORT  R13ContextLookuptable

        EXCEPTEXIT

      ENDIF
        
        ; And now the interuptee executes in whatever mode it was in ..

        
        ; it is possible to get ghost interrupts - ignore these, unless
        ; too many of them stack up
        ;
        ; r1 = &angel_GhostInterrupt
        ;
GhostInterrupt
        LDR     r1, =angel_GhostCount
        LDR     r0, [r1]
        ADD     r0, r0, #1
        CMP     r0, #5
        STR     r0, [r1]
        BLT     ReturnFromIntHandler

TooManyGhosts
        FatalError "Too Many Ghost Interrupts\n"
                                
        END     ; EOF suppasm.s
