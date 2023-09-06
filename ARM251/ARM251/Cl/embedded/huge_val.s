;;; RCS $Revision: 1.1 $
;;; Checkin $Date: 1998/02/13 11:47:13 $
;;; Revising $Author: wdijkstr $

        GET     objmacs.s

        CodeArea

        EXPORT  __huge_val

__huge_val DATA
        DCD     0x7fefffff
        DCD     0xffffffff

        END
