/* Complex tangent function for long double.  IBM extended format version.
   Copyright (C) 1997-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <complex.h>
#include <fenv.h>
#include <math.h>
#include <math_ldbl_opt.h>
#include <float.h>

#include <math_private.h>

/* IBM long double GCC builtin sets LDBL_EPSILON == LDBL_DENORM_MIN  */
static const long double ldbl_eps = 0x1p-106L;

__complex__ long double
__ctanl (__complex__ long double x)
{
  __complex__ long double res;

  if (!isfinite (__real__ x) || !isfinite (__imag__ x))
    {
      if (__isinfl (__imag__ x))
	{
	  __real__ res = __copysignl (0.0, __real__ x);
	  __imag__ res = __copysignl (1.0, __imag__ x);
	}
      else if (__real__ x == 0.0)
	{
	  res = x;
	}
      else
	{
	  __real__ res = __nanl ("");
	  __imag__ res = __nanl ("");

	  if (__isinf_nsl (__real__ x))
	    feraiseexcept (FE_INVALID);
	}
    }
  else
    {
      long double sinrx, cosrx;
      long double den;
      const int t = (int) ((LDBL_MAX_EXP - 1) * M_LN2l / 2.0L);

      /* tan(x+iy) = (sin(2x) + i*sinh(2y))/(cos(2x) + cosh(2y))
        = (sin(x)*cos(x) + i*sinh(y)*cosh(y)/(cos(x)^2 + sinh(y)^2). */

      __sincosl (__real__ x, &sinrx, &cosrx);

      if (fabsl (__imag__ x) > t)
	{
	  /* Avoid intermediate overflow when the real part of the
	    result may be subnormal.  Ignoring negligible terms, the
	    imaginary part is +/- 1, the real part is
	    sin(x)*cos(x)/sinh(y)^2 = 4*sin(x)*cos(x)/exp(2y).  */
	  long double exp_2t = __ieee754_expl (2 * t);

	  __imag__ res = __copysignl (1.0L, __imag__ x);
	  __real__ res = 4 * sinrx * cosrx;
	  __imag__ x = fabsl (__imag__ x);
	  __imag__ x -= t;
	  __real__ res /= exp_2t;
	  if (__imag__ x > t)
	    {
	      /* Underflow (original imaginary part of x has absolute
		 value > 2t).  */
	      __real__ res /= exp_2t;
	    }
	  else
	    __real__ res /= __ieee754_expl (2.0L * __imag__ x);
	}
      else
	{
	  long double sinhix, coshix;
	  if (fabsl (__imag__ x) > LDBL_MIN)
	    {
	      sinhix = __ieee754_sinhl (__imag__ x);
	      coshix = __ieee754_coshl (__imag__ x);
	    }
	  else
	    {
	      sinhix = __imag__ x;
	      coshix = 1.0L;
	    }

	  if (fabsl (sinhix) > fabsl (cosrx) * ldbl_eps)
	    den = cosrx * cosrx + sinhix * sinhix;
	  else
	    den = cosrx * cosrx;
	  __real__ res = sinrx * (cosrx / den);
	  __imag__ res = sinhix * (coshix / den);
	}

      /* __gcc_qmul does not respect -0.0 so we need the following fixup.  */
      if ((__real__ res == 0.0L) && (__real__ x == 0.0L))
        __real__ res = __real__ x;

      if ((__real__ res == 0.0L) && (__imag__ x == 0.0L))
        __imag__ res = __imag__ x;
    }

  return res;
}
long_double_symbol (libm, __ctanl, ctanl);
