/* dlist.h
 * Copyright (c) 1997 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1997/07/23 13:59:08 $
 * Revising $Author: clavende $
 */

#ifndef I_DNODE
        #include "class.h"

        #define I_SNODE(type) struct type *succ
        #define SUCC(a) ((a)->succ)
        #define sINIT_NODE(l) SUCC(l)=l
        #define sADD_HEAD(l,e) (SUCC(e)=SUCC(l)),(SUCC(l)=e)
        #define sREM_HEAD(l,e) (e=SUCC(l)),(SUCC(l)=SUCC(e)),e

        #define sAPPLY_FUNC_TO_LIST(l,f)        { snode *tmp,*node; \
                for(node=SUCC((snode*)l);(tmp=node)!=(snode*)l;f((void*)tmp)) \
                { node = SUCC(tmp); } }

        typedef struct SNODE {
                I_SNODE( SNODE );
                        } snode;

        snode *sNewList(void);
        snode *sInitNode(snode *d);
        snode *sAddHead(snode *l,snode *e);
        snode *sRemHead(snode *l);
        void  sApplyFuncToList(snode *l,void (*func)());

        #define I_DNODE(type) struct type *succ,*pred
        #define PRED(a) ((a)->pred)
        #define dINIT_NODE(l) PRED(l)=SUCC(l)=l
        #define dADD_HEAD(l,e) SUCC(l)=PRED(SUCC(e)=SUCC(PRED(e)=l))=e
        #define dADD_TAIL(l,e) PRED(l)=SUCC(PRED(e)=PRED(SUCC(e)=l))=e
        #define dREM_HEAD(l,e) PRED(SUCC(l)=SUCC(e=SUCC(l)))=l
        #define dREM_TAIL(l,e) SUCC(PRED(l)=PRED(e=PRED(l)))=l
        #define dREM_NODE(e) PRED(SUCC(PRED(e))=SUCC(e))=PRED(e)
        #define dApplyFuncToList(l,f)   sApplyFuncToList((snode*)l,f)

        typedef struct DNODE {
                I_DNODE( DNODE );
                        } dnode;

        dnode *dInitNode(dnode *d);
        dnode *dNewList(void);
        dnode *dAddHead(dnode *l,dnode *e);
        dnode *dAddTail(dnode *l,dnode *e);
        dnode *dRemNode(dnode *e);
        dnode *dRemHead(dnode *l);
        dnode *dRemTail(dnode *l);
        void dRemDealloc(void *d);

        typedef struct D_REFERENCE {
                I_DNODE( D_REFERENCE );
                void    *ref;
                        } dReference;

        #define APPLY_FUNC_TO_LIST(l,f) { dReference *tmp,*node; \
                for(node=SUCC((dReference*)l);(tmp=node)!=(dReference*)l; \
                        f((void*)(tmp->ref))) { node = SUCC(tmp); } }

        void  applyFuncToList(dnode*dr,void (*func)());
#endif
