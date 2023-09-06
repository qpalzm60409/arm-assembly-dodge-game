; fpe.s
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.20 $
; Checkin $Date: 1998/07/28 12:46:06 $
; Revising $Author: wdijkstr $

        GET     objmacs.s
        GET     defaults.s

        GBLL    FPEWanted
        GBLL    FPASCWanted
        GBLL    EnableInterrupts
        GBLA    CoreDebugging

FPEWanted       SETL    {FALSE}
FPASCWanted     SETL    {FALSE}
EnableInterrupts        SETL    {FALSE}
CoreDebugging   SETA    0


;==============================================================================

; 
; Allow some control over the code/speed of code produced.
; 0 = fastest -> 2 = smallest (overall)
; 

        GBLA    CodeSize
CodeSize        SETA    0

        MACRO
        ImportCodeSize $name
        [ CodeSize <> 0
        IMPORT  $name
        ]
        MEND

        MACRO
        ExportCodeSize $name
        [ CodeSize <> 0
        EXPORT  $name
        ]
        MEND

;==============================================================================

        MACRO
$label  CDebug4 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND
        MACRO
$label  CDebug3 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND
        MACRO
$label  CDebug2 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND
        MACRO
$label  CDebug1 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND
        MACRO
$label  CDebug0 $label,$message,$reg1,$reg2,$reg3,$reg4
        MEND

; File for setting up THUMB macros for entry and exit from the THUMB
; versions of the functions.

        GBLA    V_N

        GET     regnames.s
        GET     armdefs.s
        GET     fpadefs.s
        GET     macros.s

sp      RN      R13
lr      RN      R14
pc      RN      R15

dOP1h   RN      R0      ;Double OP1 hi-reg ("First word") - sign,expn,etc.
dOP1l   RN      R1      ;Double OP1 lo-reg ("Second word")
dOPh    RN      R0      ;Double OP hi-reg (unary ops)
dOPl    RN      R1      ;Double OP lo-reg
dOP2h   RN      R2      ;Double OP2 hi-reg ("First word")
dOP2l   RN      R3      ;Double OP2 lo-reg ("Second word")

fOP1    RN      R0      ;Float OP1
fOP     RN      R0      ;Float OP for unary ops
fOP2    RN      R1      ;Float OP2

utmp1   RN      R2      ;Temporary register fo unary operations
utmp2   RN      R3      ;    "

ip      RN      R12
tmp     RN      R12     ;A temporary register

ll      RN      R0      ; long long low reg
lh      RN      R1      ; long long high reg

SignBit         EQU     &80000000
fSignalBit      EQU     &00400000
dSignalBit      EQU     &00080000
Internal_mask   EQU     &00000000
Single_pos      EQU     0
Double_pos      EQU     1
Single_mask     EQU     1:SHL:Single_pos
Double_mask     EQU     1:SHL:Double_pos
Reverse         EQU     0x4     ; Used to signal a reverse divide

;;Error flags - an extension to the normal internal format

Error_pos       EQU     29
Error_bit       EQU     1:SHL:Error_pos

Except_len      EQU     5
Except_pos      EQU     Error_pos-Except_len

        ASSERT  IOC_pos < DZC_pos
        ASSERT  DZC_pos < OFC_pos
        ASSERT  OFC_pos < UFC_pos
        ASSERT  UFC_pos < IXC_pos
FPExceptC_pos   EQU     IOC_pos
        ASSERT  IOE_pos < DZE_pos
        ASSERT  DZE_pos < OFE_pos
        ASSERT  OFE_pos < UFE_pos
        ASSERT  UFE_pos < IXE_pos
FPExceptE_pos   EQU     IOE_pos

INX_pos         EQU     Except_pos-FPExceptC_pos+IXC_pos
INX_bit         EQU     1:SHL:INX_pos
INX_bits        EQU     INX_bit :OR: Error_bit :OR: Uncommon_bit

UNF_pos         EQU     Except_pos-FPExceptC_pos+UFC_pos
UNF_bit         EQU     1:SHL:UNF_pos
UNF_bits        EQU     UNF_bit :OR: Error_bit :OR: Uncommon_bit

OVF_pos         EQU     Except_pos-FPExceptC_pos+OFC_pos
OVF_bit         EQU     1:SHL:OVF_pos
OVF_bits        EQU     OVF_bit :OR: Error_bit :OR: Uncommon_bit

DVZ_pos         EQU     Except_pos-FPExceptC_pos+DZC_pos
DVZ_bit         EQU     1:SHL:DVZ_pos
DVZ_bits        EQU     DVZ_bit :OR: Error_bit :OR: Uncommon_bit

IVO_pos         EQU     Except_pos-FPExceptC_pos+IOC_pos
IVO_bit         EQU     1:SHL:IVO_pos
IVO_bits        EQU     IVO_bit :OR: Error_bit :OR: Uncommon_bit

; Following fields are used to identify the originator function and the return type so that the
; error handler can return the correct value.

Fn_pos          EQU     20
Fn_mask         EQU     15 << Fn_pos

ArithFN         EQU     0 << Fn_pos
CompareFalseFn  EQU     1 << Fn_pos
CompareTrueFn   EQU     2 << Fn_pos
CmpEqualFn      EQU     3 << Fn_pos
CmpGreaterFn    EQU     4 << Fn_pos
CmpLessFn       EQU     5 << Fn_pos  
FixFn           EQU     6 << Fn_pos
FixuFn          EQU     7 << Fn_pos
FixZeroFn       EQU     8 << Fn_pos
AddFn           EQU     9 << Fn_pos
SubFn           EQU     10 << Fn_pos
MulFn           EQU     11 << Fn_pos
DivFn           EQU     12 << Fn_pos
ModFn           EQU     13 << Fn_pos

Double_bit      EQU     1 << 18
LongLong_bit    EQU     1 << 19

EDOM            EQU     1
ERANGE          EQU     2
ESIGNUM         EQU     3

veneer_s        RLIST   {r4-r9,r11,lr}
veneer_l        RLIST   {r4-r9,r11,pc}


        END
