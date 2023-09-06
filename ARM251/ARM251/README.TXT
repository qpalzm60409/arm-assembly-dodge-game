
                         SDT 2.51 Release Notes
                         ----------------------


This CD contains ARM's Software Development Toolkit (SDT) version
2.51.

SDT 2.51 is functionally equivalent to SDT 2.50, but contains a number
of bug fixes, and introduces support for Windows 2000 and Solaris 2.7.
The documentation is not changed from SDT 2.50, and the manuals will
continue to say SDT 2.50, although an errata document has been
produced to correct documentation errors.  This errata document also
describes improvements which have been made to the Gateway DLL.

This release of the SDT is supported on Windows 95, 98, 2000, Windows
NT 4, Solaris 2.5.1, 2.6 and 2.7, HP/UX-10.

Full details of the changes introduced between SDT 2.11a and SDT
2.50/2.51 can be found in Chapter 1 of the SDT User Guide and the SDT
Reference Guide, which are also supplied in PDF form on the SDT 2.51
CD.

Please refer to the Software Development Toolkit FAQ for the latest
information on the toolkit, which can be found on ARM's Web site at:

http://www.arm.com/DevSupp/Sales+Support/faq.html


Additional information is provided in Application notes which can be
found on ARM's Web site at:

http://www.arm.com/Documentation/AppNotes


General information of ARM's Development Systems can be found on ARM's
Web site at:

http://www.arm.com/DevSupp



The major bugs fixed in this release are:


C Compilers:

1.  tcc fatal error when accessing unaligned signed short in packed struct
    [Defect 26387]

2.  tcc peephole code generation problem may give bad code
    [Defect 26463]

3.  -W+s warning message does not appear for typedef'd structs
    [Defect 26735]

4.  Complex function calls with more than 4 args may give bad code
    [Defect 26949]

5.  tcc optimisation doesn't check conditional expressions for side effects
    [Defect 26955]

6.  Need to relax ANSI compliance w.r.t. static non-constant initialisers
    [Defect 26997]

7.  Compiler switch -fx is documented, but not enabled
    [Defect 26247]

8.  -fd compiler option does not exist in SDT 2.50
    [Defect 26793]

9.  -za1 should prevent unaligned LDMs
    [Defect 25837]

10. First armcc literal pool may encompass entire function
    [Defect 25715]

11. CSE optimisation bug when % is used multiple times
    [Defect 26951]

12. typedef'd __packed structs lose '__packed', so generate unaligned LDR
    [Defect 27801]

13. Thumb compiler error "displacement out of range 256" when
    compiling very large Thumb functions
    [Defect 27977]

14. Instruction scheduling re-orders loads even when volatile is used
    [Defect 32061]

15. Code generation fault for non-word aligned sub structure copies
    [Defect 32729]


armlink:

1.  armlink: Scatterloaded ELF/DWARF2 image regions sometimes not built correctly
    [Defect 26385]


armlib:

1.  armlib does not accept full pathnames (e.g. /work/project/filename)
    [Defect 26855]


C Libraries:

1.  status.o & vsprintf.o missing from Embedded C Library
    [Defect 25947]

2.  Embedded C library: register corruption in __fp_status_arm
    [Defect 31409]


ARMulator:

1.  ARMulator undefined instruction exception for MSR cpsr_cxsf
    [Defect 33779]

2.  Bigendian images on cached processors using ARMulator
    [Defect 35431]


ADW:

1.  "go" in ADW script does not enable "stop" button
    [Defect 35159]

2.  ADW fixed unnecessary access to memory (relative to frame pointer)
    [Defect 837/25875]

3.  ADW improved search path handling
    [Defect 34253]

4.  ADW command window now supports 'quit' command
    [Defect 31739]

5.  ADW objects with same name no longer give 'Ambiguous reference'
    [Defect 33185/34721]
