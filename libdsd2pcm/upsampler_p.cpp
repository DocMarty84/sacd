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

#include <xmmintrin.h>  // SSE2 inlines
#include <math.h>
#include "dither.h"

//prepare 32-bit versions
#define FIR_DOUBLE_PRECISION 0

// can't use templates due to memory align and SSE use
#define FirHistory FirHistory_32
#define FirFilter FirFilter_32
#define DownsamplerNx DownsamplerNx_32
#define ResamplerNxMx ResamplerNxMx_32

#include "upsampler.cpp"

#undef FirHistory
#undef FirFilter
#undef DownsamplerNx
#undef ResamplerNxMx

#undef FIR_DOUBLE_PRECISION

//prepare 64-bit versions
#define FIR_DOUBLE_PRECISION 1

// can't use templates due to memory align and SSE use
#define FirHistory FirHistory_64
#define FirFilter FirFilter_64
#define DownsamplerNx DownsamplerNx_64
#define ResamplerNxMx ResamplerNxMx_64

#include "upsampler.cpp"

#undef FirHistory
#undef FirFilter
#undef DownsamplerNx
#undef ResamplerNxMx

#undef FIR_DOUBLE_PRECISION

// Dither
Dither::Dither(unsigned int n_bits)
{
    static int last_holdrand = 0;

    unsigned int max_value;

    assert(n_bits <= 31);

    max_value = 2 << n_bits;
    m_rand_max = 1.0 / (double)max_value;

    m_holdrand = last_holdrand++;
}

Dither& Dither::operator=(const Dither &obj)
{
    m_rand_max = obj.m_rand_max;

    return *this;
}

// filter generation
void generateFilter(double *impulse, int taps, double sinc_freq)
{
    int i, int_sinc_freq;
    double x1, y1, x2, y2, y, sum_y, taps_per_pi, center_tap;

    taps_per_pi = sinc_freq / 2.0;
    center_tap = (double)(taps - 1) / 2.0;

    int_sinc_freq = (int)floor(sinc_freq);

    if ((double)int_sinc_freq != sinc_freq)
        int_sinc_freq = 0;

    sum_y = 0;

    for (i = 0; i < taps; i++) {

        // sinc
        x1 = ((double)i - center_tap) / taps_per_pi * M_PI;
        y1 = (x1 == 0.0) ? 1.0 : (sin(x1) / x1);

        if (int_sinc_freq != 0 && ((taps - 1) % 2) == 0 && (int_sinc_freq % 2) == 0 && ((i - ((taps - 1) / 2)) % (int_sinc_freq / 2)) == 0)
        {
            // insert true zero here!
            y1 = 0.0;
            //y1 = ((double)i == center_tap) ? 1.0 : 0.0;
        }

        if ((double)i == center_tap)
            y1 = 1.0;

        // windowing (BH7)
        x2 = (double)i / (double)(taps - 1);   // from [0.0 to 1.0]
        y2 = 0.2712203606 - 0.4334446123 * cos(2.0 * M_PI * x2) + 0.21800412 * cos(4.0 * M_PI * x2) - 0.0657853433 * cos(6.0 * M_PI * x2) + 0.0107618673 * cos(8.0 * M_PI * x2) - 0.0007700127 * cos(10.0 * M_PI * x2) + 0.00001368088 * cos(12.0 * M_PI * x2);
        y = y1 * y2;
        impulse[i] = y;
        sum_y += y;
    }

    // scale
    for (i = 0; i < taps; i++)
        impulse[i] /= sum_y;
}
