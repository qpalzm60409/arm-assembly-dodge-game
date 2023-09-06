;;; RCS $Revision: 1.2 $
;;; Checkin $Date: 1998/03/19 15:04:17 $
;;; Revising $Author: wdijkstr $

                GET     objmacs.s

                CodeArea

                CODE16

                EXPORT  __call_via_r0
                EXPORT  __call_via_r1
                EXPORT  __call_via_r2
                EXPORT  __call_via_r3
                EXPORT  __call_via_r4
                EXPORT  __call_via_r5
                EXPORT  __call_via_r6
                EXPORT  __call_via_r7

__call_via_r0   BX      r0
__call_via_r1   BX      r1
__call_via_r2   BX      r2
__call_via_r3   BX      r3
__call_via_r4   BX      r4
__call_via_r5   BX      r5
__call_via_r6   BX      r6
__call_via_r7   BX      r7

                END
