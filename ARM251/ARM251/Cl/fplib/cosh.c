/* cosh.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */
/* Copyright (C) ARM Limited, 1994-1998. All rights reserved. */

/*
 * RCS $Revision: 1.8.38.1 $
 * Checkin $Date: 1998/11/09 15:21:23 $
 * Revising $Author: ijohnson $
 */

#include "mathlib.h"
#include "constant.h"

double cosh(double x)
{
  ip_number ix=_d2e(x);

  if (fp_uncommon(ix)) {
    goto uncommon;
  }

  fp_abs(ix);                   /* cosh(x) == cosh(-x) */

  if (fp_grpow(ix,0)) {         /* if (x > 1) */
    ip_number ix2;

    ix=_esub(ix,&sinh_lnv);
    ix=_exp(ix);
    if (fp_error(ix)) goto error;
    if (!fp_grpow(ix,32)) {
      ix2=ix;
      ix=_eadd(_erdv(ix,&sinh_vm2),&ix2);
    }
    ix2=ix;
    ix=_eadd(_emul(ix,&sinh_v2m1),&ix2);
  } else {                      /* use normal rule */
    ip_number ix2;
    ix2=_exp(ix);
    ix=_emul(_eadd(_erdv(ix2,&one),&ix2),&half);
  }
  /* check for overflow */
  if (fp_error(ix)) {
  error:
    return __fp_erange(0, TRUE); /* always return +ve */
  }

  return _e2d(ix);

 uncommon:
  /* Defined for infinities */
  if (!fp_infinity(ix)) {
    return __fp_edom(0, TRUE);  /* always +ve */
  }

  /* Return Inf with sign bit cleared */
  return fabs(x);
}

/* EOF cosh.c */
