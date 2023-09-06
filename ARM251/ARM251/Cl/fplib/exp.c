/* exp.c */
/* Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved. */
/* Copyright (C) ARM Limited, 1994-1998. All rights reserved. */

/*
 * RCS $Revision: 1.9.38.2 $
 * Checkin $Date: 1998/11/09 15:21:25 $
 * Revising $Author: ijohnson $
 */

#include <stdarg.h>
#include <stdlib.h>

#include <limits.h>
#include <errno.h>
#include <math.h>

#include "mathlib.h"
#include "constant.h"

#ifdef _exp_c

static const ip_number exp_range[] = {
  0x00003fff, 0xb8aa3b29, 0x5c17f0bc, /* 1/ln(2) */
  0x00003ffe, 0xb1800000, 0x00000000, /* M1 */
  0x80003ff2, 0xde8082e3, 0x08654362  /* M2 (ln(2)-M1) */
  };

static const ip_number exp_P[] = {
  0x00003ff1, 0x845a2157, 0x3490f106, /* 2*p2 (p1=0.31555192765684646356e-4) */
  0x00003ff8, 0xf83a5f91, 0x50952c99, /* 2*p1 (p2=0.75753180159422776666e-2) */
  0x00003ffe, 0x80000000, 0x00000000  /* 2*p0 (p0=0.25000000000000000000e+0) */
  };

static const ip_number exp_Q[] = {
  0x00003feb, 0xc99b1867, 0x3490f106, /* 2*q3 (q3=0.75104028399870046114e-6) */
  0x00003ff5, 0xa57862e1, 0x46a6fb39, /* 2*q2 (q2=0.63121894374398503557e-3) */
  0x00003ffb, 0xe8b9428e, 0xfecff592, /* 2*q1 (q1=0.56817302698551221787e-1) */
  0x00003fff, 0x80000000, 0x00000000  /* 2*q0 (q0=0.50000000000000000000e-0) */
  };

/* static const ip_number error_word =  { error_bit, 0x0, 0x0 }; */

#ifdef EMBEDDED_CLIB
extern ___weak volatile int *__rt_errno_addr(void);
#endif

__value_in_regs ip_number _exp(ip_number ix)
{
  range_red g;

  /*
   * The algorithm used in what follows comes from Cody & Waite, chapter 6.
   *
   * We will use range reduction to transform the general EXP problem to that
   * of taking a EXP on the range -ln(2)/2 to ln(2)/2, ending by adding an
   * integer to the exponent.
   *   If we get a quotient outside the range -2^16 to 2^16-1, we've definitely
   * got massive overflow/underflow: all we have to do is produce some result
   * that will cause this effect.
   */
  /* MJW: 02-Nov-1998. 
   * I don't understand why -2^16..2^16 -- surely
   * it will overflow for the range -2^14..2^14, since the result of
   * the exp() will be in the range 1/sqrt(2)..sqrt(2). This is the
   * range I check for.  */
  g=__fp_range_red_by_mod(ix,exp_range);
  if (fp_error(g.x)
      || g.n < -(1<<14)
      || g.n >= (1<<14)) {
    goto over_under;
  }

  /*
   * To avoid underflow problems, we separate out the case of g being very
   * small. If it is sufficiently small that adding g + g^2/2 + ... to 1 is
   * not going to make any significant difference to 1, then the answer can be
   * taken as being:
   *
   * * 1.0 with a round bit of 0 and a sticky bit of 1 if g is positive;
   *
   * * 1.0-epsilon with round and sticky bits both 1 if g is negative;
   *
   * This happens if ABS(g + g^2/2 + g^3/6 + ...) < 2^(-65), which will
   * certainly happen if g < 2^(-66).
   */
  if (fp_geqpow(g.x, -66)) {
    range_red g2;
    ip_number z,gpz,qz,r;
    
    /*
     * From here on, we know the calculation will not underflow or overflow.
     * Evaluate z = g^2, then the polynomial 2*Q(z) (See Cody & Waite).
     * Since the base of IEEE arithmetic is 2, we don't need to counteract
     * wobbling precision in the way suggested in Cody & Waite: I have therefore
     * multiplied both these polynomials by 2 to make the code slightly cleaner.
     */

    g2=g;
    /* z = g^2 */
    z=_esquare(g.x);
    
    /* I've re-ordered this calculation to improve the data-flow for the
     * compiler -- gpz can be put straight onto the stack. */
    /* First evaluate 2*g*P(z) */
    gpz=_emul(__fp_poly(z,3,exp_P),&g2.x);

    /* Then 2*Q(z) */
    qz=__fp_poly(z,4,exp_Q);
    
    /* Then the final calculation -- 0.5 + 2*g*P(z)/(2*Q(z) - 2*g*P(z)) */
    r=_eadd(_erdv(_esub(qz,&gpz),&gpz),&half);
    

    /* Having evaluated exp(g), we want exp(n*ln(2) + g)
     * = exp(n*ln(2))exp(g)
     * = 2^nexp(g), i.e. a simple ldexp(exp(g), n)
     * +1 because???
     */
    r.word.hi += g.n+1;
    return r;
  } else {                      /* exponent small */
    ip_number r;

    r.word.hi=0x3ffe; r.word.lo=units_bit; r.word.lo2=0;
    /* Attempt to return 1 - epsilon for small negative numbers.
     * When converted back to double precision this epsilon is lost,
     * anyway.
     */
    if (!dp_ispos(g.x) && !dp_iszero(g.x)) {
      r.word.hi--;
      r.word.lo = r.word.lo2 = 0xffffffff; /* take away epsilon */
    }
    r.word.hi += g.n + 1;       /* scale back */
    return r;
  }

over_under:                     /* either overflow or underflow */
{                               /* -ve => underflow, +ve => overflow */
  ip_number r;

  if (fp_sign(ix)) {
    r.word.hi = 0;
  } else {
    
#ifdef EMBEDDED_CLIB
    if (__rt_errno_addr) {
        *__rt_errno_addr() = ERANGE;
    }
#else
    errno=ERANGE;
#endif
    r.word.hi = error_bit;
  }
  r.word.lo=r.word.lo2=0;

  return r;
}
}

#else
/* declared in mathlib.h */
#endif

#ifdef exp_c

double exp(double x)
{
  ip_number ix;

  ix=_d2e(x);

  if (!fp_uncommon(ix)) {
    ix=_exp(ix);
    if (!fp_error(ix)) return _ANSI_e2d(ix);
    
  {
    dp_number f;
    /* return fp_sign(ix) ? -HUGE_VAL : HUGE_VAL; */
    f.d=HUGE_VAL;
    f.word.hi|=fp_sign(ix);
    return f.d;
  }
  }

  if (fp_infinity(ix)) return __fp_erange(ix.word.hi, TRUE);
  return __fp_edom(ix.word.hi, TRUE);
}

#endif

/* EOF exp.c */
