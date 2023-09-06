        SUBT    Run-time support macros for assembler source         > macros.s
        ; ---------------------------------------------------------------------
        ;
        ; $Revision: 1.4.2.1 $
        ;   $Author: rivimey $
        ;     $Date: 1998/09/22 13:46:20 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ; ---------------------------------------------------------------------

        ASSERT  (listopts_s)
old_opt SETA    {OPT}
        OPT     (opt_off)   ; disable listing of include files

        ; ---------------------------------------------------------------------

        [       (:LNOT: :DEF: macros_s)

                GBLL    macros_s
macros_s        SETL    {TRUE}



sl      RN      r10
fp      RN      r11


        ; ---------------------------------------------------------------------
        ; LogInfo
        ; -------
        ;
        ; LogInfo file, line, id, text
        ;
        ; Simulates the LogInfo "C" macro, entering the indicated string (and
        ; up to 3 parameters from r1-3) into the debug log buffer. The mapping
        ; of the position of parameters to registers is fixed, however -- the
        ; first parameter in the text must be in r1, the second in r2, etc.
        ;
        ; For information about the actual action of the calls, refer to the
        ; logging code in logging.h / logging.c
        ;
        ; The macro currently ignores the 'file' arg, althogh for completeness
        ; current callers do provide it.

        MACRO
        LogInfo $file, $line, $id, $text
        
      IF DEBUG <> 0
        IMPORT  Log_logmsginfo
        IMPORT  log_loginfo     
        
        STMFD   sp!, {r0-r3,r12,r14}
        MOV     r0, #0
        LDR     r1, =$line
        MOV     r2, #0
        BL      Log_logmsginfo
        LDMFD   sp, {r0-r3,r12,r14}
        
        LDR     r0, =%F1
        BL      log_loginfo
        LDMFD   sp!, {r0-r3,r12,r14}
        
        B       %F2

1
        DCB     "$text", 0
        ALIGN
2
                
      ENDIF
        
        MEND

        ; ---------------------------------------------------------------------
        ; FatalError
        ; ----------
        ;
        ; FatalError text
        ;
        ; Wrapper for calls to the fatal error handler, __rt_asm_fatalerror,
        ; which in turn calls the logging system, iff debug is enabled.
        ; 'text' is a printf-style formst string, The mapping
        ; of the position of parameters to registers is fixed, however -- the
        ; first parameter in the text must be in r1, the second in r2, etc.
        ;
        ; For information about the actual action of the calls, refer to the
        ; logging code in logging.h / logging.c

        MACRO
        FatalError $text
        
      IF DEBUG <> 0
        ADR     r0, %F1
      ENDIF
        
        B       __rt_asm_fatalerror

      IF DEBUG <> 0
1
        DCB     "$text", 0
        ALIGN
      ENDIF
                
        MEND

        
        ; ---------------------------------------------------------------------
        ; bit
        ; ---
        ; This provides a shorthand method of describing individual bit
        ; positions.
        ;
        MACRO
$label  bit     $index
$label  *       (1 :SHL: $index)
        MEND

        ; ---------------------------------------------------------------------
        ; NPOW2
        ; -----
        ; Calculate the next-power-of-2 number above the value given.
        ;
        MACRO
$label  NPOW2   $value
        LCLA    newval
newval  SETA    1
        WHILE   (newval < $value)
newval  SETA    (newval :SHL: 1)
        WEND
$label  EQU     (newval)
        ; Allow the user to see how much "wasted" space is being
        ; generated within the object:
        !       0,"NPOW2: original &" :CC: (:STR: $value) :CC: " new &" :CC: (:STR: newval) :CC: " wasted &" :CC: (:STR: (newval - $value))
        MEND

        ; ---------------------------------------------------------------------

        ]       ; macros_s

        OPT     (old_opt)   ; restore previous listing options

        ; ---------------------------------------------------------------------
        END     ; EOF macros.s
