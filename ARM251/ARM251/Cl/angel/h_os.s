; Purpose: Angel compatable semihosting definitions
;
; Copyright (C) ARM Ltd., 1991-1998.
;
; The terms and conditions under which this software is supplied to you and
; under which you may use it are described in your licence agreement with
; ARM.


; THESE DEFINITIONS ARE DUPLICATED IN "h_os.h". CHANGE AT YOUR PERIL!
        
;;; RCS $Revision: 1.2 $
;;; Checkin $Date: 1998/06/03 18:35:44 $
;;; Revising $Author: mnonweil $

        ; Angel breakpoint *instruction*
angel_BreakPointInstruction_ARM         EQU     0xE7FDDEFE
angel_BreakPointInstruction_THUMB       EQU     0x0000DEFE
        
        ; Angel SWI *code* (i.e. use "SWI  angel_SWI_ARM")
angel_SWI_ARM                           EQU     0x00123456
angel_SWI_THUMB                         EQU     0x000000AB

        ; Clib r0 ranges. See arm.h for changes!
angel_SWIreason_CLibBase                EQU     0x00000001
angel_SWIreason_CLibLimit               EQU     0x00000016
angel_SWIreasonLimit                    EQU     0x00000020
        
        ; Angel extended SWI ops (use swi code, set r0 = )
angel_SWIreason_EnterSVC                EQU     0x00000017
angel_SWIreason_ReportException         EQU     0x00000018
angel_SWIreason_ApplDevice              EQU     0x00000019
angel_SWIreason_LateStartup             EQU     0x00000020

        ; enum angel_LateStartType
AL_CONTINUE             EQU     0x00000000
AL_BLOCK                EQU     0x00000001

        ; ADP_Stopped reason codes: (use with ReportException)
ADP_Stopped_BranchThroughZero           EQU     131072
ADP_Stopped_UndefinedInstr              EQU     131073
ADP_Stopped_SoftwareInterrupt           EQU     131074
ADP_Stopped_PrefetchAbort               EQU     131075
ADP_Stopped_DataAbort                   EQU     131076
ADP_Stopped_AddressException            EQU     131077
ADP_Stopped_IRQ                         EQU     131078
ADP_Stopped_FIQ                         EQU     131079
ADP_Stopped_BreakPoint                  EQU     131104
ADP_Stopped_RunTimeErrorUnknown         EQU     131107
ADP_Stopped_InternalError               EQU     131108
ADP_Stopped_UserInterruption            EQU     131109
ADP_Stopped_ApplicationExit             EQU     131110
ADP_Stopped_StackOverflow               EQU     131111
ADP_Stopped_DivisionByZero              EQU     131112

        ; Definitions for SYS library handler functions.
SYS_OPEN                EQU     1
SYS_CLOSE               EQU     2
SYS_WRITE0              EQU     4
SYS_WRITEC              EQU     3
SYS_WRITE               EQU     5
SYS_READ                EQU     6
SYS_ISERROR             EQU     8
SYS_ISTTY               EQU     9
SYS_SEEK                EQU     10
SYS_ENSURE              EQU     11
SYS_FLEN                EQU     12
SYS_TMPNAM              EQU     13
SYS_REMOVE              EQU     14
SYS_RENAME              EQU     15
SYS_CLOCK               EQU     16
SYS_TIME                EQU     17
SYS_SYSTEM              EQU     18
SYS_ERRNO               EQU     19
SYS_GET_CMDLINE         EQU     21
SYS_HEAPINFO            EQU     22


FPEStart                EQU     &1400
FPEEnd                  EQU     &8000

        END
