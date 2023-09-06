/*
 * heapalloc.h
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1997/12/10 17:49:20 $
 * Revising $Author: hmeeking $
 */

#ifndef __alloc__h
#define __alloc__h

#ifndef __stddef__h
#include <stddef.h>
#endif

extern void _init_alloc(void);
/*
 * Initialise allocate (not to be called by anyone outside the M2Core!)
 */

extern size_t _byte_size(void *p);
/*
 * Return an approximation to the allocated size of the object pointed
 * to by p. The only guarantee is that:-
 *   requested_size(p) <= _byte_size(p) <= allocated_size(p).
 */

/* MAXBYTES should be something outrageously big */
#define MAXBYTES     0x01000000

extern int _primitive_dealloc(BlockP block);

#ifdef STATS
extern void ShowStats(void);
#endif

#endif

/* End of file heapalloc.h */
