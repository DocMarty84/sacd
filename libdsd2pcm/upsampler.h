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

// this is the stub file included twice from upsampler_p.h

#ifndef FIR_DOUBLE_PRECISION
# error FIR_DOUBLE_PRECISION must be defined to 0 or to 1
#endif

// can't use templates due to memory align and SSE use
#if FIR_DOUBLE_PRECISION
# define SAMPLE_T   double
#else
# define SAMPLE_T   float
#endif

// history buffer for FIR
class FirHistory {
public:
    FirHistory(unsigned int fir_size);
    ~FirHistory();

    FirHistory&  operator=(const FirHistory &obj);

    void    pushSample(double x);
    void    reset(bool reset_to_1 = false);

    SAMPLE_T    *getBuffer() { return &m_x[m_head]; }

    unsigned int    getSize() const { return m_fir_size; }

private:
    SAMPLE_T    *m_x;       // [m_fir_size * 2]

    unsigned int    m_fir_size;
    unsigned int    m_head;     // write to m_x[--head]
};

// FIR filter
class FirFilter {
public:
    FirFilter(const double *fir, unsigned int fir_size, bool no_history = false);
    FirFilter();
    ~FirFilter();

    FirFilter&  operator=(const FirFilter &obj);

    double  processSample(double x);
    void    reset(bool reset_to_1 = false);

    void    pushSample(double x);

    double  fast_convolve(SAMPLE_T *x);

    const SAMPLE_T  *getFir() const { return m_fir; }
    unsigned int    getFirSize() const { return m_org_fir_size; }

private:
    SAMPLE_T    *m_fir;     // [m_fir_size], aligned
    SAMPLE_T    *m_fir_alloc;

    FirHistory      m_x;
    unsigned int    m_fir_size;         // aligned
    unsigned int    m_org_fir_size;
};

/*
 * Nx downsampler
 */

class DownsamplerNx {
public:
    DownsamplerNx(unsigned int nX, const double *fir, unsigned int fir_len);

    double  processSample(const double *x);
    void    reset(bool reset_to_1 = false);

    unsigned int    getFirSize() const { return m_flt.getFirSize(); }

private:
    FirFilter       m_flt;
    unsigned int    m_xN;
};

/*
 * Nx/Mx resampler
 */

class ResamplerNxMx {
public:
    ResamplerNxMx(unsigned int nX, unsigned int mX, const double *fir, unsigned int fir_size);
    ~ResamplerNxMx();

    void    processSample(const double *x, unsigned int x_n, double *y, unsigned int *y_n);
    void    reset(bool reset_to_1 = false);

    unsigned int    getFirSize() const { return m_fir_size; }

private:
    unsigned int    m_xN;       // up^
    unsigned int    m_xM;       // down_

    unsigned int    m_fir_size;

    FirFilter      *m_flt;      // [m_xN]

    FirHistory      m_x;

    unsigned int    m_xN_counter;   // how many virtually upsampled samples we have in FirHistory?
};

#undef SAMPLE_T
