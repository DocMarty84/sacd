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

# include <xmmintrin.h>  // SSE2 inlines
# include <sys/stat.h>   // mkdir

# define _ASSERT(x)
# define MAX_PATH    260

#include <locale.h>     // setlocale
#include <stdarg.h>     // va_list
#include <stdio.h>      // snprintf
#include <string.h>     // memset
#include <stdio.h>      // sprintf
#include <stdlib.h>     // rand

#define _USE_MATH_DEFINES
#include <math.h>
#include "dsdpcm_converter.h"
#include "dsdpcm_converter_hq.h"

/*
 * dsdpcm_converter_hq
 */

dsdpcm_converter_hq::dsdpcm_converter_hq() : dsdpcm_conv_impl_t(), m_dither24(24)
{
    unsigned int i, j;

    m_convert_called = false;
    m_decimation = 0;
    m_upsampling = 0;
    m_use_resampler = false;

    memset(m_dn, 0, sizeof(m_dn));
    memset(m_resampler, 0, sizeof(m_resampler));

    // fill bits_table
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 4; j++)
            m_bits_table[i][j] = (i & (1 << (3 - j))) ? 1.0 : -1.0;
    }
}

dsdpcm_converter_hq::~dsdpcm_converter_hq()
{
    int i;

    for (i = 0; i < DSDPCM_MAX_CHANNELS; i++) {
        delete m_dn[i];
        delete m_resampler[i];
    }
}

int dsdpcm_converter_hq::init(int channels, int dsd_samplerate, int pcm_samplerate)
{
    double *impulse;
    int i, taps, sinc_freq;

    // common part

    this->channels = channels;
    this->dsd_samplerate = dsd_samplerate;
    this->pcm_samplerate = pcm_samplerate;

    switch (dsd_samplerate)
    {
    case DSDxFs64:
        break;
    case DSDxFs128:
        // NOT SUPPORTED!!!
        return -2;
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

    // DSD64_96000:
    m_upsampling = 5;
    m_decimation = 147;

    // generate filter
    taps = 2879;
    sinc_freq = 70 * 5;     // 44.1 * 64 * upsampling / x

    impulse = new double[taps];

    generateFilter(impulse, taps, sinc_freq);

    for (i = 0; i < channels; i++)
        m_resampler[i] = new ResamplerNxMx(m_upsampling, m_decimation, impulse, taps);

    delete[] impulse;

    m_use_resampler = true;

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

int
dsdpcm_converter_hq::convert(uint8_t* dsd_data, float* pcm_data, int dsd_samples)
{
    if (!m_use_resampler)
        return convertDown(dsd_data, pcm_data, dsd_samples);
    else
        return convertResample(dsd_data, pcm_data, dsd_samples);
}

int
dsdpcm_converter_hq::convertDown(uint8_t* dsd_data, float* pcm_data, int dsd_samples)
{
    int i, pcm_samples, ch, offset, dsd_offset, pcm_offset;
    double dsd_input[DSDPCM_MAX_CHANNELS][MAX_DECIMATION], x;
    uint8_t dsd8bits;

    _ASSERT((dsd_samples % m_decimation) == 0);
    _ASSERT((m_decimation % 8) == 0);

    pcm_samples = (dsd_samples * 8) / m_decimation / channels;

    dsd_offset = 0;
    pcm_offset = 0;

    // all PCM samples
    for (i = 0; i < pcm_samples; i++) {

        // fill decimation buffer for downsampling
        for (offset = 0; offset < m_decimation; offset += 8) {  // (m_decimation % 8) == 0

            // all channels
            for (ch = 0; ch < channels; ch++) {

                dsd8bits = dsd_data[dsd_offset++];

                // fastfill doubles from bits

                memcpy(&dsd_input[ch][offset], m_bits_table[(dsd8bits & 0xf0) >> 4], 4 * sizeof(double));
                memcpy(&dsd_input[ch][offset + 4], m_bits_table[dsd8bits & 0x0f], 4 * sizeof(double));

            }
        }

        // now fill pcm samples in all channels!!!
        for (ch = 0; ch < channels; ch++) {

            x = m_dn[ch]->processSample(dsd_input[ch]);

            pcm_data[pcm_offset++] = (float)m_dither24.processSample(x);

        }
    }

    _ASSERT(dsd_offset == dsd_samples);
    _ASSERT(pcm_offset == pcm_samples * channels);

    m_convert_called = true;

    return pcm_offset;
}

int
dsdpcm_converter_hq::convertResample(uint8_t* dsd_data, float* pcm_data, int dsd_samples)
{
    int i, pcm_samples, ch, offset, dsd_offset, pcm_offset, j;
    double dsd_input[DSDPCM_MAX_CHANNELS][MAX_RESAMPLING_IN + 8], x[DSDPCM_MAX_CHANNELS][MAX_RESAMPLING_OUT];
    uint8_t dsd8bits;
    unsigned int x_samples = 1;

    _ASSERT((dsd_samples % m_decimation) == 0);

    pcm_samples = (dsd_samples * 8) / m_decimation / channels * m_upsampling;

    dsd_offset = 0;
    pcm_offset = 0;

    offset = 0;     // offset in dsd_input

    // all PCM samples
    for (i = 0; i < pcm_samples; i += x_samples) {

        // fill decimation buffer for downsampling
        for (; offset < m_decimation; offset += 8) {    // has padding of 8 elements

            // all channels
            for (ch = 0; ch < channels; ch++) {

                dsd8bits = dsd_data[dsd_offset++];

                // fastfill doubles from bits

                memcpy(&dsd_input[ch][offset], m_bits_table[(dsd8bits & 0xf0) >> 4], 4 * sizeof(double));
                memcpy(&dsd_input[ch][offset + 4], m_bits_table[dsd8bits & 0x0f], 4 * sizeof(double));

            }
        }

        // now fill pcm samples in all channels!!!
        for (ch = 0; ch < channels; ch++) {
            m_resampler[ch]->processSample(dsd_input[ch], m_decimation, x[ch], &x_samples);

            // shift overfill in channel
            memcpy(&dsd_input[ch][0], &dsd_input[ch][m_decimation], (offset - m_decimation) * sizeof(double));
        }

        // shift offset
        offset -= m_decimation;

        // and output interleaving samples
        for (j = 0; j < (int)x_samples; j++) {
            for (ch = 0; ch < channels; ch++) {
                // interleave
                pcm_data[pcm_offset++] = (float)m_dither24.processSample(x[ch][j]);
            }
        }
    }

    _ASSERT(dsd_offset == dsd_samples);
    _ASSERT(pcm_offset == pcm_samples * channels);

    m_convert_called = true;

    return pcm_offset;
}
