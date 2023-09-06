/* assert.c: support for assert() macro              */
/* Copyright (C) Advanced Risc Machines Ltd., 1991    */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1997/09/02 11:11:19 $
 * Revising $Author: achapman $
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void __assert(const char *expr, const char *file, int line)
{   fprintf(stderr, "*** assertion failed: %s, file %s, line %d\n",
            expr, file, line);
    abort();
}

