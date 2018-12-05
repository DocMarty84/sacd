/*
    Copyright 2015-2016 Robert Tari <robert.tari@gmail.com>
    Copyright 2012 Vladislav Goncharov <vl-g@yandex.ru> (HQ DSD->PCM converter 88.2/96 kHz)

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

#ifndef __DSD_PCM_CONVERTER_H__
#define __DSD_PCM_CONVERTER_H__

#include <stdlib.h>
#include <stdint.h>

#include "dsd_pcm_constants.h"
#include "upsampler.h"

class DSDPCMConverter
{
public:
    DSDPCMConverter();

    ~DSDPCMConverter();

    int init(int channels, int dsd_samplerate, int pcm_samplerate);

    int convert(uint8_t *dsd_data, int dsd_samples, float *pcm_data);

    float get_delay();

    bool is_convert_called();

private:
    unsigned int m_decimation;
    unsigned int m_upsampling;
    int m_nChannels;
    bool conv_called;
    static const int MAX_RESAMPLING_IN = 147 * 2; // 64x -> 96  (147 -> 5 for 64x -> 96, 128x not supported)
    static const int MAX_RESAMPLING_OUT = 5 * 2; // 147 -> 5 for 64x -> 96
    ResamplerNxMx *m_resampler[DSDPCM_MAX_CHANNELS];
    Dither m_dither24;
    double m_bits_table[16][4];
    uint8_t swap_bits[256];

    int convertResample(uint8_t *dsd_data, int dsd_samples, float *pcm_data);
};

#endif // __DSD_PCM_CONVERTER_H__
