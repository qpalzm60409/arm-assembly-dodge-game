/* models.c - stubs of all the models linked to ARMulator in large lists.
 * Copyright (C) 1996 Advanced RISC Machines. All Rights Reserved.
 *
 * RCS $Revision: 1.18 $
 * Checkin $Date: 1998/06/19 14:47:03 $
 * Revising $Author: irickard $
 */

#include "armdefs.h"

#define MEMORY(X) extern ARMul_MemStub X ;
#define COPROCESSOR(X) extern ARMul_CPStub X ;
#ifndef NEW_OS_INTERFACE
#define OSMODEL(X) extern ARMul_OSStub X ;
#endif
#define MODEL(X) extern ARMul_ModelStub X ;
#include "models.h"

#undef COPROCESSOR
#ifndef NEW_OS_INTERFACE
#undef OSMODEL
#endif
#undef MODEL

#define COPROCESSOR(X)
#ifndef NEW_OS_INTERFACE
#define OSMODEL(X)
#endif
#define MODEL(X)

ARMul_MemStub *ARMul_Memories[] = {
#undef MEMORY
#define MEMORY(X) & X ,
#include "models.h"
  NULL
  };

#undef MEMORY
#define MEMORY(X)

ARMul_CPStub *ARMul_Coprocessors[] = {
#undef COPROCESSOR
#define COPROCESSOR(X) & X ,
#include "models.h"
  NULL
  };

#undef COPROCESSOR
#define COPROCESSOR(X)

#ifndef NEW_OS_INTERFACE
ARMul_OSStub *ARMul_OSs[] = {
#undef OSMODEL
#define OSMODEL(X) & X ,
#include "models.h"
  NULL
  };

#undef OSMODEL
#define OSMODEL(X)
#endif

ARMul_ModelStub *ARMul_Models[] = {
#undef MODEL
#define MODEL(X) & X ,
#include "models.h"
  NULL
  };

#undef MODEL
#define MODEL(X)

/* Finally, we declare the RDI's that are used internally to the ARMulator. */

/* List of ARMulator RDIs */
extern struct RDI150ProcVec const basic_rdi;
extern struct RDI150ProcVec const sarmul_rdi;
extern struct RDI150ProcVec const armul2_rdi;
extern struct RDI150ProcVec const armul9_rdi;

struct RDI150ProcVec const * const ARMul_RDIProcs[] = {
#ifndef NO_SARMSD
  &sarmul_rdi,
#endif
#ifndef NO_ARM9
  &armul9_rdi,
#endif
#ifdef ARMUL2
  &armul2_rdi,
#else
#ifndef NO_BASIC
  &basic_rdi,
#endif
#endif
  NULL
  };

