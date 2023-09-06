/* class.h
 * Copyright (c) 1997 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1997/07/23 13:57:58 $
 * Revising $Author: clavende $
 */

#ifndef decl
        #include        <string.h>
        /*#include      <malloc.h>*/
        #include        <stdlib.h>

        #define decl(type,var)          type *var
        #define alloc(type)                     calloc(1,sizeof(type))
        #define assign(type,a,b)        memcpy(a,b,sizeof(type))
        #define ASSIGN(a,b)                     memcpy(a,b,sizeof(*b))
        #define dealloc(type,var)       free(var)
        #define assoc(var,field)        var->field=field
        #define class(type,var)         ((type*)var)

        #define saveStr(s)                      ((s)?strcpy(malloc(strlen((s))+1),(s)):0)
        #define strSave(s)                      ((s)?strcpy(malloc(strlen((s))+1),(s)):0)
        #define saveBlock(a,l)          ((l)?memcpy(malloc(l),(a),(l)):0)
        #define blockSave(a,l)          ((l)?memcpy(malloc(l),(a),(l)):0)

/*      typedef long    time,date; */

        int failed(void);
        int passed(void);

#endif
