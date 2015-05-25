/*
* HQ DSD->PCM converter 88.2/96 kHz
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

#ifndef _dsdpcm_converter_hq_h_
#define _dsdpcm_converter_hq_h_

#include "upsampler_p.h"

class dsdpcm_converter_hq : public dsdpcm_conv_impl_t
{
public:
    dsdpcm_converter_hq();
    virtual ~dsdpcm_converter_hq();

    int     init(int channels, int dsd_samplerate, int pcm_samplerate);
    int     convert(uint8_t* dsd_data, float* pcm_data, int dsd_samples);
    /*int     convert(uint8_t* dsd_data, double* pcm_data, int dsd_samples);*/

    void    set_gain(float dB_gain) {}      // not supported

    float   get_delay();
    bool    is_convert_called() { return m_convert_called; }

private:
    bool    m_convert_called;

    int     m_decimation;
    int     m_upsampling;
    bool    m_use_resampler;

    static const int    MAX_DECIMATION = 32;        // 64x -> 88.2 (44.1 not supported, 128x not supported)
    static const int    MAX_RESAMPLING_IN = 147;    // 64x -> 96  (147 -> 5 for 64x -> 96, 128x not supported)
    static const int    MAX_RESAMPLING_OUT = 5;     // 147 -> 5 for 64x -> 96

#if 1
    typedef DownsamplerNx_64    DownsamplerNx;
    typedef ResamplerNxMx_64    ResamplerNxMx;
#else
    typedef DownsamplerNx_32    DownsamplerNx;
    typedef ResamplerNxMx_32    ResamplerNxMx;
#endif

    DownsamplerNx   *m_dn[DSDPCM_MAX_CHANNELS];
    ResamplerNxMx   *m_resampler[DSDPCM_MAX_CHANNELS];

    Dither          m_dither24;

    double          m_bits_table[16][4];

    int     convertDown(uint8_t* dsd_data, float* pcm_data, int dsd_samples);
    int     convertResample(uint8_t* dsd_data, float* pcm_data, int dsd_samples);
};

#endif
