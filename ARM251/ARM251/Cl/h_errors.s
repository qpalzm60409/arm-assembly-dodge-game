;;; RCS $Revision: 1.2 $
;;; Checkin $Date: 1995/02/01 16:37:42 $
;;; Revising $Author: enevill $

;;; h_errors.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

        MACRO
        ErrorBlock $name, $string
E_$name
        &       Error_$name
        =       "$string", 0
        ALIGN
        MEND

MemoryBase      *       &8000           ; for rd & wr checks: require
MemoryLimit     *       &00400000       ; pointer to be in this range

; Arthur error numbers
Error_NameNotFound              *       &124
Error_ValueTooLong              *       &125

Error_IllegalInstruction        *       &80000000
Error_PrefetchAbort             *       &80000001
Error_DataAbort                 *       &80000002
Error_AddressException          *       &80000003
Error_UnknownIRQ                *       &80000004
Error_BranchThroughZero         *       &80000005

Error_FPBase                    *       &80000200

Error_FP_IVO                    *       Error_FPBase + 0
Error_FP_OFL                    *       Error_FPBase + 1
Error_FP_DVZ                    *       Error_FPBase + 2
Error_FP_UFL                    *       Error_FPBase + 3
Error_FP_INX                    *       Error_FPBase + 4

Error_FPLimit                   *       &80000300

; Arthur errors generated by the library
CLib_Error_Base                 *       &800e80
CLib_Error_Range                *       &80

Error_BadMemory                 *       &800e80
Error_UnknownLib                *       &800e81
Error_StubCorrupt               *       &800e82
Error_StaticSizeWrong           *       &800e83
Error_StaticOffsetInconsistent  *       &800e84
Error_UnknownSWI                *       &800e85

Error_SharedLibraryNeeded       *       &800e90
Error_OldSharedLibrary          *       &800e91
Error_NoVeneer                  *     &80800e92

Error_ReadFail                  *     &80800ea0
Error_WriteFail                 *     &80800ea1

Error_RecursiveTrap             *       &800e00
Error_UncaughtTrap              *       &800e01
Error_NoMainProgram             *       &800e02
Error_NotAvailable              *       &800e03
Error_NoEnvFile                 *       &800e04
Error_NoRoomForEnv              *       &800e05
Error_BadReturnCode             *       &800e06
Error_NoStackForTrapHandler     *       &800e07
Error_Exit                      *       &800e08   ; in non-user mode
Error_WrongCPU                  *       &800e09

Error_ReservedForOverlayManager1 *      &800efe
Error_ReservedForOverlayManager2 *      &800eff

Error_DivideByZero              *     &80000020
Error_StackOverflow             *     &80000021

        END
