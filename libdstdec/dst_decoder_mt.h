/*
    Copyright 2015 Robert Tari <robert.tari@gmail.com>
    Copyright 2011-2014 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#ifndef ___DST_DECODER_H__
#define ___DST_DECODER_H__

#include <pthread.h>
#include "dst_decoder.h"

enum slot_state_t
{
    SLOT_EMPTY, SLOT_LOADED, SLOT_RUNNING, SLOT_READY, SLOT_READY_WITH_ERROR, SLOT_TERMINATING
};

class frame_slot_t
{
public:
    volatile int state;
    uint8_t *dsd_data;
    uint8_t *dst_data;
    size_t dst_size;
    int channel_count;
    int samplerate;
    pthread_t hThread{};
    pthread_cond_t hEventGet{};
    pthread_cond_t hEventPut{};
    pthread_mutex_t hMutex{};
    CDSTDecoder D;

    frame_slot_t()
    {
        state = SLOT_EMPTY;
        dsd_data = nullptr;
        dst_data = nullptr;
        dst_size = 0;
        channel_count = 0;
        samplerate = 0;
    }
};

class dst_decoder_t
{
public:
    int slot_nr;

    explicit dst_decoder_t(int threads);

    ~dst_decoder_t();

    int init(int channel_count, int samplerate, int framerate);

    int decode(uint8_t *dst_data, size_t dst_size, uint8_t **dsd_data, size_t *dsd_size);

private:
    frame_slot_t *frame_slots;
    int thread_count;
    int channel_count;
    int samplerate;
    int framerate;
    uint32_t frame_nr;
};

#endif  // ___DST_DECODER_H__
