/*
    Copyright 2015-2016 Robert Tari <robert.tari@gmail.com>
    Copyright 2012 Vladislav Goncharov <vl-g@yandex.ru>

    This file is part of SACD.

    SACD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SACD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SACD.  If not, see <http://www.gnu.org/licenses/gpl-3.0.txt>.
*/

#include <emmintrin.h>
#include <cmath>
#include <cstring>
#include <cassert>
#include "upsampler.h"

// FirHistory
FirHistory::FirHistory(unsigned int fir_size)
{
    m_fir_size = fir_size;

    if (m_fir_size > 0) {
        // init history buffer
        m_x = new double[m_fir_size * 2 + 10]; // leave space for FIR pad (8 SSE block align + 2 memory align)
        m_head = m_fir_size;

        memset(m_x, 0, (m_fir_size * 2 + 10) * sizeof(double));

    } else {
        m_x = nullptr;
        m_head = 0;
    }
}

FirHistory::~FirHistory()
{
    delete[] m_x;
}

FirHistory &FirHistory::operator=(const FirHistory &obj)
{
    delete[] m_x;

    m_fir_size = obj.m_fir_size;

    if (m_fir_size > 0) {
        // init history buffer
        m_x = new double[m_fir_size * 2 + 10]; // leave space for FIR pad (8 SSE block align + 2 memory align)
        m_head = m_fir_size;

        memset(m_x, 0, (m_fir_size * 2 + 10) * sizeof(double));

    } else {
        m_x = nullptr;
        m_head = 0;
    }

    return *this;
}

void FirHistory::pushSample(double x)
{
    // push to head
    if (m_head == 0) {
        // shift
        memcpy(m_x + m_fir_size, m_x, m_fir_size * sizeof(double));

        m_head = m_fir_size;
    }

    m_x[--m_head] = x;
}

// FirFilter
FirFilter::FirFilter(const double *fir, unsigned int fir_size, bool no_history) : m_x(!no_history ? fir_size : 0)
{
    m_org_fir_size = fir_size;

    if ((fir_size % 8) != 0) {
        m_fir_size = (fir_size / 8 + 1) * 8; // align size!
    } else {
        m_fir_size = m_org_fir_size;
    } // already aligned

    m_fir_alloc = new double[m_fir_size + 2]; // reserve some space for pointer align

    // align pointer!
    m_fir = (((size_t) m_fir_alloc & 0x0f) == 0) ? m_fir_alloc : (double *) (((size_t) m_fir_alloc & ~0x0f) + 0x10);

    for (unsigned int i = 0; i < m_fir_size; i++) {
        m_fir[i] = (i < fir_size) ? fir[i] : 0;
    }
}

FirFilter::FirFilter() : m_x(0)
{
    m_fir = nullptr;
    m_fir_alloc = nullptr;
    m_fir_size = 0;
}

FirFilter::~FirFilter()
{
    delete[] m_fir_alloc;
}

FirFilter &FirFilter::operator=(const FirFilter &obj)
{
    delete[] m_fir_alloc;

    m_fir_size = obj.m_fir_size;
    m_org_fir_size = obj.m_org_fir_size;

    m_fir_alloc = new double[m_fir_size + 2]; // reserve some space for pointer align

    // align pointer!
    m_fir = (((size_t) m_fir_alloc & 0x0f) == 0) ? m_fir_alloc : (double *) (((size_t) m_fir_alloc & ~0x0f) + 0x10);

    memcpy(m_fir, obj.m_fir, m_fir_size * sizeof(double));

    m_x = obj.m_x;

    return *this;
}

// fir must be aligned! fir_size must be %8!
double FirFilter::fast_convolve(double *x)
{
    // convolution
    __m128d xy1, xy2, xy3, xy4;

    xy1 = _mm_setzero_pd();
    xy2 = _mm_setzero_pd();
    xy3 = _mm_setzero_pd();
    xy4 = _mm_setzero_pd();

    for (unsigned int i = 0; i < m_fir_size; i += 8) {
        xy1 = _mm_add_pd(xy1, _mm_mul_pd(_mm_loadu_pd(x + i), // NOLINT(portability-simd-intrinsics)
                                         _mm_load_pd(m_fir + i))); // NOLINT(portability-simd-intrinsics)
        xy2 = _mm_add_pd(xy2, _mm_mul_pd(_mm_loadu_pd(x + i + 2), // NOLINT(portability-simd-intrinsics)
                                         _mm_load_pd(m_fir + i + 2))); // NOLINT(portability-simd-intrinsics)
        xy3 = _mm_add_pd(xy3, _mm_mul_pd(_mm_loadu_pd(x + i + 4), // NOLINT(portability-simd-intrinsics)
                                         _mm_load_pd(m_fir + i + 4))); // NOLINT(portability-simd-intrinsics)
        xy4 = _mm_add_pd(xy4, _mm_mul_pd(_mm_loadu_pd(x + i + 6), // NOLINT(portability-simd-intrinsics)
                                         _mm_load_pd(m_fir + i + 6))); // NOLINT(portability-simd-intrinsics)
    }

    xy1 = _mm_add_pd(_mm_add_pd(xy1, xy2), _mm_add_pd(xy3, xy4)); // NOLINT(portability-simd-intrinsics)

    double xy_flt[2];

    _mm_storeu_pd(xy_flt, xy1);

    return xy_flt[0] + xy_flt[1];
}

// ResamplerNxMx
ResamplerNxMx::ResamplerNxMx(unsigned int nX, unsigned int mX, const double *fir, unsigned int fir_size) :
        m_x((fir_size % nX) == 0 ? fir_size / nX : fir_size / nX + 1)
{
    unsigned int *xfir_size;
    double *xfir;

    m_fir_size = fir_size;
    m_xN = nX;
    m_xM = mX;
    xfir_size = new unsigned int[nX];
    m_flt = new FirFilter[nX];

    for (unsigned int i = 0; i < nX; i++) {
        xfir_size[i] = (i < (fir_size % nX)) ? (fir_size / nX + 1) : (fir_size / nX);

        // use max size for alloc
        xfir = new double[xfir_size[0]];

        // fill and pad with zeros
        for (unsigned int j = 0; j < xfir_size[0]; j++) {
            xfir[j] = j < xfir_size[i] ? fir[i + j * nX] : 0;
        }

        // got filter
        m_flt[i] = FirFilter(xfir, xfir_size[i], true);

        delete[] xfir;
    }

    delete[] xfir_size;

    m_xN_counter = 0;
}

ResamplerNxMx::~ResamplerNxMx()
{
    delete[] m_flt;
}

void ResamplerNxMx::processSample(const double *x, unsigned int x_n, double *y, unsigned int *y_n)
{
    unsigned int offset, x_phase;

    assert((x_n * m_xN) % m_xM == 0); // x_n input samples to integer number of y_n output samples!!!

    offset = 0;

    for (unsigned int i = 0; i < x_n; i++) {
        // push 1 sample
        m_x.pushSample(x[i]);

        // actually we pushed xN samples (xN upsampled)
        m_xN_counter += m_xN;

        if (m_xN_counter >= m_xM) {
            // calculate phase to fill m_xM samples
            x_phase = m_xN_counter - m_xM;

            // apply phase shift (0 -> 0000x -> (N-1); 1 -> x0000 -> 0; 2 -> 0x000 -> 1)
            x_phase = (x_phase + (m_xN - 1)) % m_xN;

            y[offset++] = m_flt[x_phase].fast_convolve(m_x.getBuffer()) * (double) m_xN;

            // leave some zero virtual samples in buffer
            m_xN_counter -= m_xM;
        }
    }

    assert(offset == x_n * m_xN / m_xM);

    *y_n = offset;
}

// Dither
Dither::Dither(unsigned int n_bits)
{
    static int last_holdrand = 0;

    unsigned int max_value;

    assert(n_bits <= 31);

    max_value = static_cast<unsigned int>(2 << n_bits);
    m_rand_max = 1.0 / (double) max_value;

    m_holdrand = last_holdrand++;
}

Dither &Dither::operator=(const Dither &obj)
{
    m_rand_max = obj.m_rand_max;

    return *this;
}

// filter generation
void generateFilter(double *impulse, int taps, double sinc_freq)
{
    int int_sinc_freq;
    double x1, y1, x2, y2, y, sum_y, taps_per_pi, center_tap;

    taps_per_pi = sinc_freq / 2.0;
    center_tap = (double) (taps - 1) / 2.0;

    int_sinc_freq = (int) floor(sinc_freq);

    if ((double) int_sinc_freq != sinc_freq) {
        int_sinc_freq = 0;
    }

    sum_y = 0;

    for (int i = 0; i < taps; i++) {
        // sinc
        x1 = ((double) i - center_tap) / taps_per_pi * M_PI;
        y1 = (x1 == 0.0) ? 1.0 : (sin(x1) / x1);

        if (int_sinc_freq != 0 && ((taps - 1) % 2) == 0 && (int_sinc_freq % 2) == 0 &&
            ((i - ((taps - 1) / 2)) % (int_sinc_freq / 2)) == 0) {
            // insert true zero here!
            y1 = 0.0;
            //y1 = ((double)i == center_tap) ? 1.0 : 0.0;
        }

        if ((double) i == center_tap) {
            y1 = 1.0;
        }

        // windowing (BH7)
        x2 = (double) i / (double) (taps - 1);   // from [0.0 to 1.0]
        y2 = 0.2712203606 - 0.4334446123 * cos(2.0 * M_PI * x2) + 0.21800412 * cos(4.0 * M_PI * x2) -
             0.0657853433 * cos(6.0 * M_PI * x2) + 0.0107618673 * cos(8.0 * M_PI * x2) -
             0.0007700127 * cos(10.0 * M_PI * x2) + 0.00001368088 * cos(12.0 * M_PI * x2);
        y = y1 * y2;
        impulse[i] = y;
        sum_y += y;
    }

    // scale
    for (int i = 0; i < taps; i++) {
        impulse[i] /= sum_y;
    }
}
