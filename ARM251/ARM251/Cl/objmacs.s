;;; h_objmacro.s
;;; Copyright (C) Advanced RISC Machines Ltd., 1991

;;; RCS $Revision: 1.18.2.1 $
;;; Checkin $Date: 1998/09/16 10:42:29 $
;;; Revising $Author: wdijkstr $

 [ :LNOT: :DEF: THUMB
        GBLL    THUMB
    [ :DEF: thumb
THUMB   SETL {TRUE}
    |
    [ {CONFIG} = 16
THUMB   SETL    {TRUE}
    |
THUMB   SETL    {FALSE}
    ]
    ]
 ]
 [ THUMB
        CODE32
 ]
 [ :LNOT: :DEF: INTERWORK
        GBLL    INTERWORK
INTERWORK SETL {FALSE}
 ]

 [ :LNOT::DEF:EMBEDDED_CLIB
; If EMBEDDED_CLIB is not defined we are going to define it
        GBLL    EMBEDDED_CLIB
; If 'embedded' is defined EMBEDDED_CLIB defaults to {TRUE} else {FALSE}
    [ :DEF:embedded
EMBEDDED_CLIB   SETL    {TRUE}
    |
EMBEDDED_CLIB   SETL    {FALSE}
    ]
 ]

 [ :LNOT: :DEF: LDM_MAX
        GBLA    LDM_MAX
LDM_MAX SETA    16
 ]

 [ :LNOT: :DEF: make
        GBLS    make
make    SETS    "all"
 ]

 [ :LNOT: :DEF: FPIS
        GBLA    FPIS
FPIS    SETA    3
 ]

        GBLL    FPE2
FPE2    SETL    FPIS=2

        GBLS    VBar
        GBLS    UL
        GBLS    XXModuleName

VBar    SETS    "|"
UL      SETS    "_"


        MACRO
        Module  $name
XXModuleName SETS UL:CC:"$name":CC:UL
        MEND

        MACRO
$Label  Variable $Size
        LCLS    Temps
        LCLA    Tempa
 [ "$Size"=""
Tempa   SETA    1
 |
Tempa   SETA    $Size
 ]
Temps   SETS    VBar:CC:XXModuleName:CC:"$Label":CC:VBar
        KEEP    $Temps
        ALIGN
O_$Label *      .-StaticData
$Temps  %       &$Tempa*4
        MEND

        MACRO
$Label  ExportedVariable $Size
        LCLS    Temps
        LCLA    Tempa
 [ "$Size"=""
Tempa   SETA    1
 |
Tempa   SETA    $Size
 ]
Temps   SETS    VBar:CC:"$Label":CC:VBar
        EXPORT  $Temps
        ALIGN
O_$Label *      .-StaticData
$Temps  %       &$Tempa*4
        MEND

        MACRO
$Label  ExportedWord $Value
        LCLS    Temps
Temps   SETS    VBar:CC:"$Label":CC:VBar
        EXPORT  $Temps
        ALIGN
O_$Label *      .-StaticData
$Temps   &      $Value
        MEND

        MACRO
$Label  ExportedVariableByte $Size
        LCLS    Temps
        LCLA    Tempa
 [ "$Size"=""
Tempa   SETA    1
 |
Tempa   SETA    $Size
 ]
Temps   SETS    VBar:CC:"$Label":CC:VBar
        EXPORT  $Temps
O_$Label *      .-StaticData
$Temps  %       &$Tempa
        MEND

        MACRO
$Label  VariableByte $Size
        LCLS    Temps
        LCLA    Tempa
 [ "$Size"=""
Tempa   SETA    1
 |
Tempa   SETA    $Size
 ]
Temps   SETS    VBar:CC:XXModuleName:CC:"$Label":CC:VBar
        KEEP    $Temps
O_$Label *      .-StaticData
$Temps  %       &$Tempa
        MEND

        MACRO
$Label  InitByte $Value
$Label  =        $Value
        MEND

        MACRO
$Label  InitWord $Value
$Label  &        $Value
        MEND

        MACRO
$Label  Keep    $Arg
        LCLS    Temps
$Label  $Arg
Temps   SETS    VBar:CC:XXModuleName:CC:"$Label":CC:VBar
        KEEP    $Temps
$Temps
        MEND


        MACRO
        PopFrame  $Regs,$Caret,$CC,$Base
 [ {CONFIG} = 16
 ! 1, "MACRO PopFrame not implemented for 16 bit code"
 ]
        LCLS    BaseReg
 [ "$Base" = ""
BaseReg SETS    "fp"
 |
BaseReg SETS    "$Base"
 ]
        LCLA    count
        LCLS    s
        LCLS    ch
s       SETS    "$Regs"
count   SETA    1
        WHILE   s<>""
ch        SETS    s:LEFT:1
s         SETS    s:RIGHT:(:LEN:s-1)
          [ ch = ","
count       SETA    count+1
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            ]
          ]
        WEND
 [ LDM_MAX < count
        LCLA    r
        LCLS    subs
s       SETS    "$Regs"
subs    SETS    ""
r       SETA    0
        SUB     ip, $BaseReg, #4*&$count
        WHILE   s<>""
ch        SETS    s:LEFT:1
s         SETS    s:RIGHT:(:LEN:s-1)
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
              LDM$CC.FD ip!,{$subs}
subs          SETS    ""
r             SETA    0
            |
subs          SETS    subs:CC:ch
            ]
          |
            [ ch = "-"
              ! 1, "Push can't handle register list including '-'"
            |
subs          SETS    subs:CC:ch
            ]
          ]
        WEND
        LDM$CC.FD   ip,{$subs}$Caret
 |
        LDM$CC.DB   $BaseReg, {$Regs}$Caret
 ]
        MEND

        MACRO
        Pop     $Regs,$Caret,$CC,$Base
 [ {CONFIG} = 16
  [ "$Caret" <> ""
  ! 1, "Cannot use writeback in 16 bit code"
  ]
  [ "$Base" <> ""
  ! 1, "Cannot specify a base register in 16 bit code"
  ]
 ]
        LCLS    BaseReg
 [ "$Base" = ""
BaseReg SETS    "sp"
 |
BaseReg SETS    "$Base"
 ]
 [ {CONFIG} = 16
  [ "$CC" <> "" :LAND: "$CC" <> "AL"
    BN$CC       %FT0
  ]
 ]
 [ LDM_MAX < 16
        LCLA    r
        LCLS    s
        LCLS    subs
        LCLS    ch
s       SETS    "$Regs"
subs    SETS    ""
r       SETA    0
        WHILE   s<>""
ch        SETS    s:LEFT:1
s         SETS    s:RIGHT:(:LEN:s-1)
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
             [ {CONFIG} = 16
              POP       {$subs}
             |
              LDM$CC.FD   $BaseReg!,{$subs}
             ]
subs          SETS    ""
r             SETA    0
            |
subs          SETS    subs:CC:ch
            ]
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            |
subs          SETS    subs:CC:ch
            ]
          ]
        WEND
  [ {CONFIG} = 16
        POP         {$subs}
0
  |
        LDM$CC.FD   $BaseReg!,{$subs}$Caret
  ]
 |
  [ {CONFIG} = 16
        POP         {$Regs}
0
  |
        LDM$CC.FD   $BaseReg!, {$Regs}$Caret
  ]
 ]
        MEND

        MACRO
        Return  $UsesSb, $ReloadList, $Base, $CC
 [ {CONFIG} = 16
  [ "$UsesSb" <> ""
        !       1, "Cannot use SB in 16 bit code"
  ]
  [ "$Base" = "fpbased"
        !       1, "Cannot use fpbased in 16 bit code"
  ]
 ]
        LCLS    Temps
Temps   SETS    "$ReloadList"
 [ "$UsesSb" <> "" :LAND: make="shared-library"
   [ Temps = ""
Temps   SETS    "sb"
   |
Temps   SETS    Temps:CC:", sb"
   ]
 ]

 [ {CONFIG} = 26
   [ "$Base" = "LinkNotStacked" :LAND: "$ReloadList"=""
          MOV$CC.S pc, lr
   |
     [ Temps <> ""
Temps   SETS    Temps :CC: ","
     ]
     [ "$Base" = "fpbased"
       PopFrame "$Temps.fp,sp,pc",^,$CC
     |
       Pop      "$Temps.pc",^,$CC
     ]
   ]
 |
  [ {CONFIG} = 16
      [ "$CC" <> "" :LAND: "$CC" <> "AL"
        BN$CC   %FT0
      ]
   [ "$Base" = "LinkNotStacked" :LAND: "$ReloadList"=""
     [ INTERWORK
        BX      lr
     |
        MOV     pc, lr
     ]
   |
     [ Temps <> ""
Temps   SETS    Temps :CC: ","
     ]
     [ INTERWORK
       BX       pc
       CODE32
       Pop      "$Temps.lr",,
       BX       lr
       CODE16
     |
       Pop      "$Temps.pc",,
     ]
   ]
0
  |
   [ "$Base" = "LinkNotStacked" :LAND: "$ReloadList"=""
    [ INTERWORK :LOR: THUMB
        BX$CC lr
    |
        MOV$CC pc, lr
    ]
   |
     [ Temps <> ""
Temps   SETS    Temps :CC: ","
     ]
    [ INTERWORK :LOR: THUMB
     [ "$Base" = "fpbased"
       PopFrame "$Temps.fp,sp,lr",,$CC
     |
       Pop      "$Temps.lr",,$CC
     ]
     BX$CC lr
    |
     [ "$Base" = "fpbased"
       PopFrame "$Temps.fp,sp,pc",,$CC
     |
       Pop      "$Temps.pc",,$CC
     ]
    ]
   ]
  ]
 ]
        MEND

        MACRO
        PushA   $Regs,$Base
 [ {CONFIG} = 16
        !       1, "MACRO PushA not implemented for 16 bit code"
 ]
        ; Push registers on an empty ascending stack, with stack pointer $Base
        ; (no default for $Base, won't be sp)
 [ LDM_MAX < 16
        LCLA    r
        LCLS    s
        LCLS    subs
        LCLS    ch
s       SETS    "$Regs"
subs    SETS    ""
r       SETA    0
        WHILE   s<>""
ch        SETS    s:LEFT:1
s         SETS    s:RIGHT:(:LEN:s-1)
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
              STMIA   $Base!,{$subs}
subs          SETS    ""
r             SETA    0
            |
subs          SETS    subs:CC:ch
            ]
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            |
subs          SETS    subs:CC:ch
            ]
          ]
        WEND
        STMIA   $Base!,{$subs}
 |
        STMIA   $Base!, {$Regs}
 ]
        MEND

        MACRO
        PopA    $Regs,$Base
  [ {CONFIG} = 16
        !       1, "MACRO PopA not implemented for 16 bit code"
  ]
        ; Pop registers from an empty ascending stack, with stack pointer $Base
        ; (no default for $Base, won't be sp)
 [ LDM_MAX < 16
        LCLA    n
        LCLA    r
        LCLS    s
        LCLS    subs
        LCLS    ch
s       SETS    "$Regs"
n       SETA    :LEN:s
subs    SETS    ""
r       SETA    0
        WHILE   n<>0
n         SETA    n-1
ch        SETS    s:RIGHT:1
s         SETS    s:LEFT:n
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
              LDMDB   $Base!,{$subs}
subs          SETS    ""
r             SETA    0
            |
subs          SETS    ch:CC:subs
            ]
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            |
subs          SETS    ch:CC:subs
            ]
          ]
        WEND
        LDMDB   $Base!,{$subs}
 |
        LDMDB   $Base!, {$Regs}
 ]
        MEND

        MACRO
        Push    $Regs, $Base
        LCLS    BaseReg
 [ "$Base" = ""
BaseReg SETS    "sp"
 |
BaseReg SETS    "$Base"
 ]
 [ LDM_MAX < 16
        LCLA    n
        LCLA    r
        LCLS    s
        LCLS    subs
        LCLS    ch
s       SETS    "$Regs"
n       SETA    :LEN:s
subs    SETS    ""
r       SETA    0
        WHILE   n<>0
n         SETA    n-1
ch        SETS    s:RIGHT:1
s         SETS    s:LEFT:n
          [ ch = ","
r           SETA    r+1
            [ r >= LDM_MAX
             [ {CONFIG} = 16
              PUSH    {$subs}
             |
              STMFD   $BaseReg!,{$subs}
             ]
subs          SETS    ""
r             SETA    0
            |
subs          SETS    ch:CC:subs
            ]
          |
            [ ch = "-"
              ! 1, "Can't handle register list including '-'"
            |
subs          SETS    ch:CC:subs
            ]
          ]
        WEND
  [ {CONFIG} = 16
        PUSH    {$subs}
  |
        STMFD   $BaseReg!,{$subs}
  ]
 |
  [ {CONFIG} = 16
        PUSH    {$Regs}
  |
        STMFD   $BaseReg!, {$Regs}
  ]
 ]
        MEND

        MACRO
        FunctionEntry $UsesSb, $SaveList, $MakeFrame
  [ {CONFIG} = 16
    [ "$UsesSb" <> ""
        !       1, "Cannot use SB in 16 bit code"
    ]
    [ "$MakeFrame" <> ""
        !       1, "Cannot make frame in 16 bit code"
    ]
  ]
        LCLS    Temps
        LCLS    TempsC
Temps   SETS    "$SaveList"
TempsC  SETS    ""
 [ Temps <> ""
TempsC SETS Temps :CC: ","
 ]

 [ "$UsesSb" <> "" :LAND: make="shared-library"
   [ "$MakeFrame" = ""
        MOV     ip, sb  ; intra-link-unit entry
        Push    "$TempsC.sb,lr"
        MOV     sb, ip
   |
        MOV     ip, sb  ; intra-link-unit entry
        Push    "sp,lr,pc"
        ADD     lr, sp, #8
        Push    "$TempsC.sb,fp"
        MOV     fp, lr
        MOV     sb, ip
   ]
 |
   [ "$MakeFrame" = ""
        Push    "$TempsC.lr"
   |
        MOV     ip, sp
        Push    "$TempsC.fp,ip,lr,pc"
        SUB     fp, ip, #4
   ]
 ]
        MEND


        GBLA    V_N

        [ INTERWORK :LOR: THUMB

        MACRO
$label  VEnter_16               ; Like VEnter, but declare __16$label as the
        CODE16                  ; THUMB entry point
__16$label
        BX      pc
        CODE32
$label  STMFD   r13!,veneer_s   ; ARM is the declared entry point
        MEND

        MACRO
$label  VEnter                  ; Declare the THUMB entry point as the main
        CODE16                  ; entry point
$label  BX      pc
        CODE32
__32$label                      ; Declare a __32$label entry point for ARM
        STMFD   r13!,veneer_s
        MEND

        MACRO
$label  VReturn $cc
$label  LDM$cc.FD r13!,veneer_s
        BX$cc   lr
        MEND

        MACRO
$label  VPull $cc
$label  LDM$cc.FD r13!,veneer_s
        MEND

        MACRO
$label  ReturnToLR $cc
$label  BX$cc   lr
        MEND

        MACRO
$label  ReturnToLR_flg $cc
$label  BX$cc   lr
        MEND

        MACRO
$label  ReturnToStack $cc
$label  LDM$cc.FD sp!,{lr}
        BX$cc   lr
        MEND

        MACRO
$label  PullFromStack $cc
$label  LDM$cc.FD sp!,{lr}
        MEND

        MACRO
$label  EnterWithLR_16
        [ "$label" <> ""
        EXPORT __16$label
        EXPORT $label
        ]
        CODE16
__16$label
        BX      pc
        CODE32
$label
        MEND


        MACRO
$label  EnterWithLR_Thumb
        [ "$label" <> ""
        EXPORT __16$label
        ]
        [ INTERWORK     ; must create ARM interworking veneer
        CODE32
        EXPORT $label
$label
        ADR ip, __16$label + 1
        BX ip
        ]
        CODE16
__16$label
        MEND

        MACRO
$label  EnterWithStack_16
        [ "$label" <> ""
        EXPORT __16$label
        EXPORT $label
        ]

        CODE16
__16$label
        BX      pc
        CODE32
$label  STMFD   sp!,{lr}
        MEND

        MACRO
$label  EnterWithLR
        [ "$label" <> ""
        EXPORT __32$label
        EXPORT $label
        ]
        CODE16
$label  BX      pc
        CODE32
__32$label
        MEND

        MACRO
$label  EnterWithStack
        [ "$label" <> ""
        EXPORT __32$label
        EXPORT $label
        ]
        CODE16
$label  BX      pc
        CODE32
__32$label
        STMFD   sp!,{lr}
        MEND
        
        MACRO
        Export $name
        EXPORT  $name
        EXPORT  __16$name
        MEND

        MACRO
        Export_32 $name
        EXPORT  __32$name
        EXPORT  $name
        MEND

        MACRO
        Import_32 $name
        IMPORT __32$name
        MEND

        MACRO
$label  B_32 $name
$label  B       __32$name
        MEND

        |

;ARM 32 and 26-bit mode

        MACRO
$label  VEnter_16
$label  STMFD   r13!,veneer_s
        MEND

        MACRO
$label  VEnter
$label  STMFD   r13!,veneer_s
        MEND

        MACRO
$label  VReturn $cc
$label  [ INTERWORK
        LDM$cc.FD r13!,veneer_s
        BX$cc   lr
        |
        [ {CONFIG} = 32 
        LDM$cc.FD r13!,veneer_l
        ]
        [ {CONFIG} = 26
        LDM$cc.FD r13!,veneer_l^
        ]
        ]
        MEND    

        MACRO
$label  VPull $cc
$label  LDM$cc.FD r13!,veneer_s
        MEND

        MACRO
$label  ReturnToLR $cc
$label  [ INTERWORK
        BX$cc   lr
        |
        [ {CONFIG} = 32
        MOV$cc  pc,lr
        ]
        [ {CONFIG} = 26
        MOV$cc.S pc,lr
        ]
        ]
        MEND

        MACRO
$label  ReturnToLR_flg $cc
$label  [ INTERWORK
        BX$cc   lr
        |
        MOV$cc  pc,lr
        ]
        MEND

        MACRO
$label  ReturnToStack $cc
$label  [ INTERWORK
        LDM$cc.FD sp!,{lr}
        BX$cc    lr
        |
        [ {CONFIG} = 32
        LDM$cc.FD sp!,{pc}
        ]
        [ {CONFIG} = 26
        LDM$cc.FD sp!,{pc}^
        ]
        ]
        MEND

        MACRO
$label  PullFromStack $cc
$label  LDM$cc.FD sp!,{lr}
        MEND

        MACRO
$label  EnterWithLR_16
        EXPORT $label
$label
        MEND

        MACRO
$label  EnterWithStack_16
        EXPORT $label
$label  STMFD   sp!,{lr}
        MEND

        MACRO
$label  EnterWithLR
        EXPORT $label
$label
        MEND

        MACRO
$label  EnterWithStack
        EXPORT $label
$label  STMFD   sp!,{lr}
        MEND

        MACRO
        Export $name
        EXPORT  $name
        MEND

        MACRO
        Export_32 $name
        EXPORT  $name
        MEND

        MACRO
        Import_32 $name
        IMPORT $name
        MEND

        MACRO
$label  B_32 $name
$label  B       $name
        MEND

        ]



        MACRO
        DataArea
        AREA    |C$$data|, DATA
        MEND

 [ make <> "shared-library"

        MACRO
        CodeArea $name
    [ INTERWORK
        AREA    |C$$code|, CODE, READONLY, INTERWORK
    |
        AREA    |C$$code|, CODE, READONLY
    ]
        MEND

        MACRO
        LoadStaticAddress $Addr, $Reg, $Ignore
        LDR     $Reg, =$Addr
        MEND

        MACRO
        LoadStaticBase $Reg, $Ignore
        LoadStaticAddress StaticData, $Reg, $Ignore
        MEND

        MACRO
        AdconTable
        MEND

        MACRO
        Function $name, $type   ; should be ARMFunction...
        LCLS    Temps
Temps   SETS    VBar:CC:"$name":CC:VBar
        EXPORT  $Temps      ; oops, forgot to check $type...
  [ THUMB :LAND: {CONFIG} <> 16
        CODE16
$Temps
        BX      pc
        CODE32
; If we are *NOT* building an interworking library we must set bit 0 of lr
; as the caller will not have done it for us.
    [ :LNOT:INTERWORK
        ORR     lr, lr, #1
    ]
  |
$Temps
  ]
        MEND

 |

        MACRO
        CodeArea $name
        AREA    |C$$code|, CODE, READONLY, REENTRANT, PIC
        MEND

        MACRO
        AdconTable
        AREA    |sb$$adcons|, DATA, READONLY, BASED sb
        MEND

        MACRO
        Function $name, $type
        LCLS    Temps
Temps   SETS    VBar:CC:"$name":CC:VBar
 [ "$type" = ""
        EXPORT  $Temps
 |
        EXPORT  $Temps[$type]
 ]
$Temps
        MEND

 ]


        MACRO
        CODE_32
        [ {CONFIG} = 16
        CODE32
        ]
        MEND

        MACRO
        CODE_16
        [ {CONFIG} <> 16
        CODE16
        ]
        MEND

        END
