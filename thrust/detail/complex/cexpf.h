/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *  Copyright 2013 Filipe RNC Maia
 *  Modifications Copyright© 2019-2024 Advanced Micro Devices, Inc. All rights reserved. 
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-
 * Copyright (c) 2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* adapted from FreeBSD:
 *    lib/msun/src/s_cexpf.c
 *    lib/msun/src/k_exp.c
 *
 */

#pragma once

#include <thrust/detail/config.h>

#include <thrust/complex.h>
#include <thrust/detail/complex/math_private.h>

THRUST_NAMESPACE_BEGIN
namespace detail{
namespace complex{

THRUST_HOST_DEVICE inline float frexp_expf(float x, int* expt)
{
  const uint32_t k = 235; /* constant for reduction */
  const float kln2 = 162.88958740F; /* k * ln2 */

  // should this be a double instead?
  float exp_x;
  uint32_t hx;

  exp_x = expf(x - kln2);
  get_float_word(hx, exp_x);
  *expt = (hx >> 23) - (0x7f + 127) + k;
  set_float_word(exp_x, (hx & 0x7fffff) | ((0x7f + 127) << 23));
  return (exp_x);
}

THRUST_HOST_DEVICE inline complex<float> ldexp_cexpf(complex<float> z, int expt)
{
  float x, y, exp_x, scale1, scale2;
  int ex_expt, half_expt;

  x = z.real();
  y = z.imag();
  exp_x = frexp_expf(x, &ex_expt);
  expt += ex_expt;

  half_expt = expt / 2;
  set_float_word(scale1, (0x7f + half_expt) << 23);
  half_expt = expt - half_expt;
  set_float_word(scale2, (0x7f + half_expt) << 23);

  return (complex<float>(cos(y) * exp_x * scale1 * scale2,
			 sin(y) * exp_x * scale1 * scale2));
}

THRUST_HOST_DEVICE inline complex<float> cexpf(const complex<float>& z)
{
  float x, y, exp_x;
  uint32_t hx, hy;

  const uint32_t
    exp_ovfl  = 0x42b17218,		/* MAX_EXP * ln2 ~= 88.722839355 */
    cexp_ovfl = 0x43400074;		/* (MAX_EXP - MIN_DENORM_EXP) * ln2 */

  x = z.real();
  y = z.imag();

  get_float_word(hy, y);
  hy &= 0x7fffffff;

  /* cexp(x + I 0) = exp(x) + I 0 */
  if (hy == 0)
    return (complex<float>(exp(x), y));
  get_float_word(hx, x);
  /* cexp(0 + I y) = cos(y) + I sin(y) */
  if ((hx & 0x7fffffff) == 0){
    return (complex<float>(cos(y), sin(y)));
  }
  if (hy >= 0x7f800000) {
    if ((hx & 0x7fffffff) != 0x7f800000) {
      /* cexp(finite|NaN +- I Inf|NaN) = NaN + I NaN */
      return (complex<float>(y - y, y - y));
    } else if (hx & 0x80000000) {
      /* cexp(-Inf +- I Inf|NaN) = 0 + I 0 */
      return (complex<float>(0.0, 0.0));
    } else {
      /* cexp(+Inf +- I Inf|NaN) = Inf + I NaN */
      return (complex<float>(x, y - y));
    }
  }

  if (hx >= exp_ovfl && hx <= cexp_ovfl) {
    /*
     * x is between 88.7 and 192, so we must scale to avoid
     * overflow in expf(x).
     */
    return (ldexp_cexpf(z, 0));
  } else {
    /*
     * Cases covered here:
     *  -  x < exp_ovfl and exp(x) won't overflow (common case)
     *  -  x > cexp_ovfl, so exp(x) * s overflows for all s > 0
     *  -  x = +-Inf (generated by exp())
     *  -  x = NaN (spurious inexact exception from y)
     */
    exp_x = exp(x);
    return (complex<float>(exp_x * cos(y), exp_x * sin(y)));
  }
}

} // namespace complex

} // namespace detail

template <>
THRUST_HOST_DEVICE inline complex<float> exp(const complex<float>& z)
{
  return detail::complex::cexpf(z);
}    
  
THRUST_NAMESPACE_END
