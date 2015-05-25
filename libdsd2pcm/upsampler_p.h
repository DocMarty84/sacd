/*
* Downsampler/Resampler with single/double precision
* Copyright (c) 2012 Vladislav Goncharov <vl-g@yandex.ru>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with FFmpeg; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef _upsampler_p_h_
#define _upsampler_p_h_

/*
 * prepare 32-bit versions
 */

#define FIR_DOUBLE_PRECISION    0

// can't use templates due to memory align and SSE use
#define FirHistory      FirHistory_32
#define FirFilter       FirFilter_32
#define DownsamplerNx   DownsamplerNx_32
#define ResamplerNxMx   ResamplerNxMx_32

#include "upsampler.h"

#undef FirHistory
#undef FirFilter
#undef DownsamplerNx
#undef ResamplerNxMx

#undef FIR_DOUBLE_PRECISION

/*
 * prepare 64-bit versions
 */

#define FIR_DOUBLE_PRECISION    1

// can't use templates due to memory align and SSE use
#define FirHistory      FirHistory_64
#define FirFilter       FirFilter_64
#define DownsamplerNx   DownsamplerNx_64
#define ResamplerNxMx   ResamplerNxMx_64

#include "upsampler.h"

#undef FirHistory
#undef FirFilter
#undef DownsamplerNx
#undef ResamplerNxMx

#undef FIR_DOUBLE_PRECISION

/*
 * common part
 */

#include "dither.h"

// generate windowed sinc impulse response for low-pass filter
void    generateFilter(double *impulse, int taps, double sinc_freq);

#endif
