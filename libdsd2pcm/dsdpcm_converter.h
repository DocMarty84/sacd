/*
* Copyright (c) 2011-2012 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>
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

#ifndef _DSDPCM_CONVERTER_H_INCLUDED
#define _DSDPCM_CONVERTER_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

#define DSDxFs1   (44100 * 1)
#define DSDxFs2   (44100 * 2)
#define DSDxFs4   (44100 * 4)
#define DSDxFs8   (44100 * 8)
#define DSDxFs64  (44100 * 64)
#define DSDxFs128 (44100 * 128)

#define CTABLES(fir_length) ((fir_length + 7) / 8)

#define DSDPCM_MAX_CHANNELS 6
#define DSDPCM_MAX_FRAMELEN (DSDxFs128 / 75 / 8)
#define DSDPCM_MAX_SAMPLES  (DSDPCM_MAX_FRAMELEN * DSDPCM_MAX_CHANNELS)
#define DSDPCM_GAIN_0       8388608

#define DSDFIR1_8_LENGTH  80
#define DSDFIR1_16_LENGTH 160
#define DSDFIR1_64_LENGTH 641
#define PCMFIR2_2_LENGTH  27
#define PCMFIR3_2_LENGTH  151
#define PCMFIR_OFFSET     0x7fffffff
#define PCMFIR_SCALE      31

typedef uint8_t dsd_sample_t[DSDPCM_MAX_CHANNELS];

class dsdpcm_conv_impl_t {
public:
    int         channels;
    int         dsd_samplerate;
    int         pcm_samplerate;
    float       gain0;
public:
    dsdpcm_conv_impl_t()
    {
        channels = 0;
        dsd_samplerate = 0;
        pcm_samplerate = 0;
    }
    virtual int init(int channels, int dsd_samplerate, int pcm_samplerate) = 0;
    virtual float get_delay() = 0;
    virtual bool is_convert_called() = 0;
    virtual int convert(uint8_t* dsd_data, float* pcm_data, int dsd_samples) = 0;
    virtual void set_gain(float dB_gain) = 0;
};

class dsdpcm_converter_t {
    dsdpcm_conv_impl_t* converter;
    float* step0_data;
    float* step1_data;
    bool   can_degibbs;
public:
    dsdpcm_converter_t();
    ~dsdpcm_converter_t();
    int init(int channels, int dsd_samplerate, int pcm_samplerate, bool dont_reinit = false);
    float get_delay();
    bool is_convert_called();
    int convert(uint8_t* dsd_data, int32_t* pcm_data, int dsd_samples);
    int convert(uint8_t* dsd_data, float* pcm_data, int dsd_samples);
    void set_gain(float dB_gain);
    void degibbs(float* pcm_data, int pcm_samples, int side);
private:
    float get_gibbs_scale(float* pcm_data, float delay, int channel, int side);
};

extern const int32_t DSDFIR1_8_COEFS[DSDFIR1_8_LENGTH];
extern const int32_t DSDFIR1_16_COEFS[DSDFIR1_16_LENGTH];
extern const int32_t DSDFIR1_64_COEFS[DSDFIR1_64_LENGTH];
extern const int32_t PCMFIR2_2_COEFS[PCMFIR2_2_LENGTH];
extern const int32_t PCMFIR3_2_COEFS[PCMFIR3_2_LENGTH];

#endif
