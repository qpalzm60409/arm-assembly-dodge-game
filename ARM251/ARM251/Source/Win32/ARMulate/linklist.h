/*
 * list.h - template for a list type
 * Copyright (C) 1996 Advanced RISC Machines Limited, All Rights Reserved.
 *
 * $Revision: 1.2.36.1 $
 *   $Author: mwilliam $
 *     $Date: 1999/12/22 16:19:01 $
 */

#ifndef linklist_h
#define linklist_h

#ifndef ALLOC_QUANTUM
#define ALLOC_QUANTUM 64
#endif

#ifdef LIST_DEBUG
#define listdebug(X) X
#else
#define listdebug(X)
#endif

#ifdef __STDC__
#define linklist_FREE_LIST(T) ll_free__ ## T
#define linklist_new_fn(T) ll_new_ ## T
#define linklist_new(T) ll_new_ ## T ()
#define linklist_free(T,X) ll_free_ ## T ( X )
#define linklist_freelist(T,X) ll_freelist_ ## T ( X )
#define linklist_COUNTER(T) COUNT_ ## T
#else
#define linklist_FREE_LIST(T) ll_free__/**/T
#define linklist_new_fn(T) ll_new_/**/T
#define linklist_new(T) ll_new_/**/T ()
#define linklist_free(T,X) ll_free_/**/T ( X )
#define linklist_freelist(T,X) ll_freelist_/**/T ( X )
#define linklist_COUNTER(T) COUNT_/**/T
#endif

#define itypedef_LIST_var(TYPE,CLASS)  \
  CLASS TYPE * linklist_FREE_LIST(TYPE)

#define itypedef_LIST(TYPE,CLASS) \
  itypedef_LIST_var(TYPE,CLASS);                                   \
  listdebug(static int linklist_COUNTER(TYPE);)                    \
\
  CLASS void linklist_free(TYPE,TYPE *n)                           \
  {                                                                \
    if (n == NULL) return;                                         \
    n->next=linklist_FREE_LIST(TYPE) ;                             \
    linklist_FREE_LIST(TYPE)=n;                                    \
    listdebug(linklist_COUNTER(TYPE)--;)                           \
  }

#define itypedef_LIST_fl(TYPE,CLASS) \
  CLASS void linklist_freelist(TYPE,TYPE *n)                       \
  {                                                                \
    TYPE *p;                                                       \
    if (n == NULL) return;                                         \
    for (p=n; p->next; p=p->next) /* do nothing */;                \
    p->next=linklist_FREE_LIST(TYPE);                              \
    linklist_FREE_LIST(TYPE)=n;                                    \
  }

#define itypedef_LIST_noidx(TYPE,CLASS) \
  CLASS TYPE *linklist_new_fn(TYPE)(void)                          \
  {                                                                \
    TYPE *newnode = linklist_FREE_LIST(TYPE);                      \
    listdebug(linklist_COUNTER(TYPE)++;)                           \
\
    if (newnode != NULL) {                                         \
      linklist_FREE_LIST(TYPE)=linklist_FREE_LIST(TYPE)->next;     \
      return newnode;                                              \
    } else {                                                       \
      int i;                                                       \
      newnode = ( TYPE *)malloc(ALLOC_QUANTUM * sizeof(TYPE));     \
      listdebug(printf("Allocating new " # TYPE " %d in use\n",    \
                       linklist_COUNTER(TYPE)));                   \
      for (i=1;i<ALLOC_QUANTUM-1;i++) {                            \
          newnode[i].next = &newnode[i+1];                         \
      }                                                            \
      newnode[i].next = NULL;                                      \
      linklist_FREE_LIST(TYPE) = &newnode[1];                      \
      return newnode;                                              \
    }                                                              \
  }

#define itypedef_LIST_idx(TYPE,CLASS,IDX) \
  CLASS TYPE *linklist_new_fn(TYPE)(void)                          \
  {                                                                \
    TYPE *newnode = linklist_FREE_LIST(TYPE);                      \
    listdebug(linklist_COUNTER(TYPE)++;)                           \
\
    if (newnode != NULL) {                                         \
      linklist_FREE_LIST(TYPE)=linklist_FREE_LIST(TYPE)->next;     \
      return newnode;                                              \
    } else {                                                       \
      int i;                                                       \
      unsigned index=IDX;                                          \
      newnode = ( TYPE *)malloc(ALLOC_QUANTUM * sizeof( TYPE ));   \
      listdebug(printf("Allocating new " # TYPE " %d in use\n",    \
                       linklist_COUNTER(TYPE)));                   \
      newnode[0].index = index++;                                  \
      for (i=1;i<ALLOC_QUANTUM-1;i++) {                            \
        newnode[i].next = &newnode[i+1];                           \
        newnode[i].index = index++;                                \
      }                                                            \
      newnode[i].next=NULL;                                        \
      newnode[i].index=index++;                                    \
      linklist_FREE_LIST(TYPE)=&newnode[1];                        \
      IDX=index;                                                   \
      return newnode;                                              \
    }                                                              \
  }

#define typedef_LIST(TYPE) \
  itypedef_LIST(TYPE,static)        \
  itypedef_LIST_noidx(TYPE,static)  \
  itypedef_LIST_var(TYPE,static)
#define typedef_LIST_idx(TYPE,IDX) \
  itypedef_LIST(TYPE,static)        \
  itypedef_LIST_idx(TYPE,static,IDX)\
  itypedef_LIST_var(TYPE,static)
#define typedef_LIST_fl(TYPE) \
  itypedef_LIST(TYPE,static)        \
  itypedef_LIST_fl(TYPE,static)  \
  itypedef_LIST_noidx(TYPE,static)  \
  itypedef_LIST_var(TYPE,static)
#define typedef_LIST_flidx(TYPE,IDX) \
  itypedef_LIST(TYPE,static)        \
  itypedef_LIST_fl(TYPE,static)  \
  itypedef_LIST_idx(TYPE,static,IDX)\
  itypedef_LIST_var(TYPE,static)
#define typedef_LIST_export(TYPE) \
  itypedef_LIST(TYPE,)              \
  itypedef_LIST_noidx(TYPE,)        \
  itypedef_LIST_var(TYPE,)
#define typedef_LIST_exportidx(TYPE,IDX) \
  itypedef_LIST(TYPE,)              \
  itypedef_LIST_idx(TYPE,IDX)       \
  itypedef_LIST_var(TYPE,)
#define typedef_LIST_import(TYPE)      \
  itypedef_LIST_var(TYPE,extern);      \
  extern TYPE * linklist_new_fn(TYPE)(void) ;   \
  extern void linklist_free(TYPE, TYPE *n)

#define foreachlist(A,B) \
  for ((B)=(A);(B);(B)=(B)->next)

#endif

/* EOF linklist.h */
