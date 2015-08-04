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

#ifndef _DST_DECODER_H_INCLUDED
#define _DST_DECODER_H_INCLUDED

#include <pthread.h>
#include "types.h"

enum slot_state_t {SLOT_EMPTY, SLOT_LOADED, SLOT_RUNNING, SLOT_READY, SLOT_TERMINATING};

typedef struct _frame_slot_t
{
    volatile int state;
    int initialized;
    int frame_nr;
    uint8_t* dsd_data;
    int dsd_size;
    uint8_t* dst_data;
    int dst_size;
    int channel_count;
    int samplerate;
    int framerate;
    pthread_t hThread;
    pthread_cond_t hEventGet;
    pthread_cond_t hEventPut;
    pthread_mutex_t hMutex;
    DstDec D;
} frame_slot_t;

typedef struct _dst_decoder_t
{
    frame_slot_t* frame_slots;
    int slot_nr;
    int thread_count;
    int channel_count;
    int samplerate;
    int framerate;
    uint32_t frame_nr;
} dst_decoder_t;

int dst_decoder_create_mt(dst_decoder_t** dst_decoder, int thread_count);
int dst_decoder_destroy_mt(dst_decoder_t* dst_decoder);
int dst_decoder_init_mt(dst_decoder_t* dst_decoder, int channel_count, int samplerate, int framerate);
int dst_decoder_decode_mt(dst_decoder_t* dst_decoder, uint8_t* dst_data, size_t dst_size, uint8_t** dsd_data, size_t* dsd_size);

#endif
