/*
    Copyright 2015 Robert Tari <robert.tari@gmail.com>
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

#include <string.h>
#include <assert.h>
#include "dsdpcm_converter_hq.h"

// dsdpcm_converter_hq
dsdpcm_converter_hq::dsdpcm_converter_hq(): m_dither24(24)
{
    unsigned int i, j;

    m_convert_called = false;
    m_decimation = 0;
    m_upsampling = 0;
    m_use_resampler = false;
    m_nChannels = 0;
    m_nDsdSamplerate = 0;
    m_nPcmSamplerate = 0;

    memset(m_dn, 0, sizeof(m_dn));
    memset(m_resampler, 0, sizeof(m_resampler));

    // fill bits_table
    for (i = 0; i < 16; i++)
    {
        for (j = 0; j < 4; j++)
            m_bits_table[i][j] = (i & (1 << (3 - j))) ? 1.0 : -1.0;
    }
}

dsdpcm_converter_hq::~dsdpcm_converter_hq()
{
    int i;

    for (i = 0; i < DSDPCM_MAX_CHANNELS; i++)
    {
        delete m_dn[i];
        delete m_resampler[i];
    }
}

int dsdpcm_converter_hq::init(int channels, int dsd_samplerate, int pcm_samplerate)
{
    double *impulse;
    int i, taps, sinc_freq, multiplier, divisor;
    conv_mode_t nConvMode;

    this->m_nChannels = channels;
    this->m_nDsdSamplerate = dsd_samplerate;
    this->m_nPcmSamplerate = pcm_samplerate;

    switch (dsd_samplerate)
    {
    case DSDxFs64:
        switch (pcm_samplerate)
        {
        case DSDxFs2:
            nConvMode = DSD64_88200;
            break;
        case 96000:
            nConvMode = DSD64_96000;
            break;
        case DSDxFs4:
            nConvMode = DSD64_176400;
            break;
        case 192000:
            nConvMode = DSD64_192000;
            break;
        default:
            return -2;
        }
        break;
    case DSDxFs128:
        switch (pcm_samplerate)
        {
        case DSDxFs2:
            nConvMode = DSD128_88200;
            break;
        case 96000:
            nConvMode = DSD128_96000;
            break;
        case DSDxFs4:
            nConvMode = DSD128_176400;
            break;
        case 192000:
            nConvMode = DSD128_192000;
            break;
        default:
            return -2;
        }
        break;
    case DSDxFs256:
        switch (pcm_samplerate)
        {
        case DSDxFs2:
            nConvMode = DSD256_88200;
            break;
        case 96000:
            nConvMode = DSD256_96000;
            break;
        case DSDxFs4:
            nConvMode = DSD256_176400;
            break;
        case 192000:
            nConvMode = DSD256_192000;
            break;
        default:
            return -2;
        }
        break;
    case DSDxFs512:
        switch (pcm_samplerate)
        {
        case DSDxFs2:
            nConvMode = DSD512_88200;
            break;
        case 96000:
            nConvMode = DSD512_96000;
            break;
        case DSDxFs4:
            nConvMode = DSD512_176400;
            break;
        case 192000:
            nConvMode = DSD512_192000;
            break;
        default:
            return -2;
        }
        break;
    default:
        return -1;
        break;
    }

    // prepare filter depending on conv mode
    for (i = 0; i < DSDPCM_MAX_CHANNELS; i++)
    {
        delete m_dn[i];
        delete m_resampler[i];
    }

    memset(m_dn, 0, sizeof(m_dn));
    memset(m_resampler, 0, sizeof(m_resampler));

    switch (nConvMode)
    {
        case DSD64_88200:
            multiplier = 1;
            divisor = 1;
            m_use_resampler = false;
            break;
        case DSD64_96000:
            multiplier = 1;
            divisor = 1;
            m_use_resampler = true;
            break;
        case DSD64_176400:
            multiplier = 1;
            divisor = 2;
            m_use_resampler = false;
            break;
        case DSD64_192000:
            multiplier = 1;
            divisor = 2;
            m_use_resampler = true;
            break;
        case DSD128_88200:
            multiplier = 2;
            divisor = 1;
            m_use_resampler = false;
            break;
        case DSD128_96000:
            multiplier = 2;
            divisor = 1;
            m_use_resampler = true;
            break;
        case DSD128_176400:
            multiplier = 2;
            divisor = 2;
            m_use_resampler = false;
            break;
        case DSD128_192000:
            multiplier = 2;
            divisor = 2;
            m_use_resampler = true;
            break;
        case DSD256_88200:
            multiplier = 4;
            divisor = 1;
            m_use_resampler = false;
            break;
        case DSD256_96000:
            multiplier = 4;
            divisor = 1;
            m_use_resampler = true;
            break;
        case DSD256_176400:
            multiplier = 4;
            divisor = 2;
            m_use_resampler = false;
            break;
        case DSD256_192000:
            multiplier = 4;
            divisor = 2;
            m_use_resampler = true;
            break;
        case DSD512_88200:
            multiplier = 8;
            divisor = 1;
            m_use_resampler = false;
            break;
        case DSD512_96000:
            multiplier = 8;
            divisor = 1;
            m_use_resampler = true;
            break;
        case DSD512_176400:
            multiplier = 8;
            divisor = 2;
            m_use_resampler = false;
            break;
        case DSD512_192000:
            multiplier = 8;
            divisor = 2;
            m_use_resampler = true;
            break;
        default:
            return -1;
    }

    if (!m_use_resampler)
    {
        // default decimation mode DSD64 -> 88.2 (32 times decimation)
        // actual decimation mode DSD64 * multiplier -> 88.2 * divisor
        m_upsampling = 1;
        m_decimation = 32 / divisor * multiplier;

        // generate filter
        taps = 574 * multiplier + 1; // NOTE: more taps have lack of depth
        sinc_freq = 70 * multiplier; // 44.1 * 64 / x  (NOTE: 80 sounds too dark and 64 sounds very harsh)
        impulse = new double[taps];

        generateFilter(impulse, taps, sinc_freq);

        for (i = 0; i < channels; i++)
            m_dn[i] = new DownsamplerNx(m_decimation, impulse, taps);

        delete[] impulse;

    }
    else
    {
        // default resampling mode DSD64 -> 96 (5/147 resampling)
        // actual resampling mode DSD64 * multiplier -> 96 * divisor (5 * divisor / 147 * multiplier)
        m_upsampling = 5 * divisor;
        m_decimation = 147 * multiplier;

        // generate filter
        taps = 2878 * divisor * multiplier + 1;
        sinc_freq = 70 * 5 * divisor * multiplier; // 44.1 * 64 * upsampling / x
        impulse = new double[taps];

        generateFilter(impulse, taps, sinc_freq);

        for (i = 0; i < channels; i++)
            m_resampler[i] = new ResamplerNxMx(m_upsampling, m_decimation, impulse, taps);

        delete[] impulse;
    }

    m_convert_called = false;

    return 0;
}

float dsdpcm_converter_hq::get_delay()
{
    if (!m_use_resampler)
        return (m_dn[0] != NULL) ? (float)(m_dn[0]->getFirSize() / 2) / (float)m_decimation : 0;
    else
        return (m_resampler[0] != NULL) ? (float)(m_resampler[0]->getFirSize() / 2) / (float)m_decimation : 0;
}

int dsdpcm_converter_hq::convert(uint8_t* dsd_data, float* pcm_data, int dsd_samples)
{
    if (!m_use_resampler)
        return convertDown(dsd_data, pcm_data, dsd_samples);
    else
        return convertResample(dsd_data, pcm_data, dsd_samples);
}

int dsdpcm_converter_hq::convertDown(uint8_t* dsd_data, float* pcm_data, int dsd_samples)
{
    int i, pcm_samples, ch, offset, dsd_offset, pcm_offset;
    double dsd_input[DSDPCM_MAX_CHANNELS][MAX_DECIMATION], x;
    uint8_t dsd8bits;

    assert((dsd_samples % m_decimation) == 0);
    assert((m_decimation % 8) == 0);

    pcm_samples = (dsd_samples * 8) / m_decimation / m_nChannels;

    dsd_offset = 0;
    pcm_offset = 0;

    // all PCM samples
    for (i = 0; i < pcm_samples; i++)
    {
        // fill decimation buffer for downsampling
        for (offset = 0; offset < m_decimation; offset += 8)
        {
            // (m_decimation % 8) == 0
            // all channels
            for (ch = 0; ch < m_nChannels; ch++)
            {
                dsd8bits = dsd_data[dsd_offset++];

                // fastfill doubles from bits
                memcpy(&dsd_input[ch][offset], m_bits_table[(dsd8bits & 0xf0) >> 4], 4 * sizeof(double));
                memcpy(&dsd_input[ch][offset + 4], m_bits_table[dsd8bits & 0x0f], 4 * sizeof(double));
            }
        }

        // now fill pcm samples in all channels!!!
        for (ch = 0; ch < m_nChannels; ch++)
        {
            x = m_dn[ch]->processSample(dsd_input[ch]);
            pcm_data[pcm_offset++] = (float)m_dither24.processSample(x);
        }
    }

    assert(dsd_offset == dsd_samples);
    assert(pcm_offset == pcm_samples * m_nChannels);

    m_convert_called = true;

    return pcm_offset;
}

int dsdpcm_converter_hq::convertResample(uint8_t* dsd_data, float* pcm_data, int dsd_samples)
{
    int i, pcm_samples, ch, offset, dsd_offset, pcm_offset, j;
    double dsd_input[DSDPCM_MAX_CHANNELS][MAX_RESAMPLING_IN + 8], x[DSDPCM_MAX_CHANNELS][MAX_RESAMPLING_OUT];
    uint8_t dsd8bits;
    unsigned int x_samples = 1;

    assert((dsd_samples % m_decimation) == 0);

    pcm_samples = (dsd_samples * 8) / m_decimation / m_nChannels * m_upsampling;
    dsd_offset = 0;
    pcm_offset = 0;
    offset = 0; // offset in dsd_input

    // all PCM samples
    for (i = 0; i < pcm_samples; i += x_samples)
    {
        // fill decimation buffer for downsampling
        for (; offset < m_decimation; offset += 8)
        {
            // has padding of 8 elements
            // all channels
            for (ch = 0; ch < m_nChannels; ch++)
            {
                dsd8bits = dsd_data[dsd_offset++];

                // fastfill doubles from bits
                memcpy(&dsd_input[ch][offset], m_bits_table[(dsd8bits & 0xf0) >> 4], 4 * sizeof(double));
                memcpy(&dsd_input[ch][offset + 4], m_bits_table[dsd8bits & 0x0f], 4 * sizeof(double));
            }
        }

        // now fill pcm samples in all channels!!!
        for (ch = 0; ch < m_nChannels; ch++)
        {
            m_resampler[ch]->processSample(dsd_input[ch], m_decimation, x[ch], &x_samples);

            // shift overfill in channel
            memcpy(&dsd_input[ch][0], &dsd_input[ch][m_decimation], (offset - m_decimation) * sizeof(double));
        }

        // shift offset
        offset -= m_decimation;

        // and output interleaving samples
        for (j = 0; j < (int)x_samples; j++)
        {
            for (ch = 0; ch < m_nChannels; ch++)
            {
                // interleave
                pcm_data[pcm_offset++] = (float)m_dither24.processSample(x[ch][j]);
            }
        }
    }

    assert(dsd_offset == dsd_samples);
    assert(pcm_offset == pcm_samples * m_nChannels);

    m_convert_called = true;

    return pcm_offset;
}

void dsdpcm_converter_hq::degibbs(float* pcm_data, int pcm_samples, int side)
{
    float delay = get_delay();
    int point = (int)ceil(delay);

    if (2 * point > pcm_samples)
    {
        return;
    }

    // fadein/fadeout to compensate lack of pre/post filter ringing
    for (int ch = 0; ch < m_nChannels; ch++)
    {
        switch (side)
        {
            case 0:

                for (int i = point; i < 2 * point; i++)
                {
                    pcm_data[m_nChannels * i + ch] *= (float)(i - point) / (float)point;
                }

                break;

            case 1:

                for (int i = 0; i < point; i++)
                {
                    pcm_data[m_nChannels * i + ch] *= (float)(point - i) / (float)point;
                }

                break;
        }
    }
}
