/*
    Copyright 2015 Robert Tari <robert.tari@gmail.com>
    Copyright 2011-2013 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include "dst_decoder.h"
#include "dst_decoder_mt.h"

void* DSTDecoderThread(void* threadarg)
{
    frame_slot_t* frame_slot = (frame_slot_t*)threadarg;

    while (1)
    {
        pthread_mutex_lock(&frame_slot->hMutex);

        while (frame_slot->state != SLOT_LOADED && frame_slot->state != SLOT_TERMINATING)
        {
            pthread_cond_wait(&frame_slot->hEventPut, &frame_slot->hMutex);
        }

        if (frame_slot->state == SLOT_TERMINATING)
        {
            pthread_mutex_unlock(&frame_slot->hMutex);
            return 0;
        }

        frame_slot->state = SLOT_RUNNING;
        pthread_mutex_unlock(&frame_slot->hMutex);

        Decode(&frame_slot->D, frame_slot->dst_data, frame_slot->dsd_data, frame_slot->frame_nr, &frame_slot->dst_size);

        pthread_mutex_lock(&frame_slot->hMutex);
        frame_slot->state = SLOT_READY;
        pthread_cond_signal(&frame_slot->hEventGet);
        pthread_mutex_unlock(&frame_slot->hMutex);
    }

    return 0;
}

int dst_decoder_create_mt(dst_decoder_t** dst_decoder, int thread_count)
{
    *dst_decoder = (dst_decoder_t*)calloc(1, sizeof(dst_decoder_t));

    if (*dst_decoder == NULL)
    {
        printf("PANIC: Could not create DST decoder object");
        return -1;
    }

    (*dst_decoder)->frame_slots = (frame_slot_t*)calloc(thread_count, sizeof(frame_slot_t));

    if ((*dst_decoder)->frame_slots == NULL)
    {
        *dst_decoder = NULL;
        printf("PANIC: Could not create DST decoder slot array");
        return -2;
    }

    (*dst_decoder)->thread_count  = thread_count;
    (*dst_decoder)->channel_count = 0;
    (*dst_decoder)->samplerate = 0;
    (*dst_decoder)->slot_nr = 0;

    return 0;
}

int dst_decoder_destroy_mt(dst_decoder_t* dst_decoder)
{
    int i;

    try
    {
        for (i = 0; i < dst_decoder->thread_count; i++)
        {
            uint8_t* dsd_data = 0;
            size_t dsd_size = 0;
            dst_decoder_decode_mt(dst_decoder, NULL, 0, &dsd_data, &dsd_size);
        }

        for (i = 0; i < dst_decoder->thread_count; i++)
        {
            frame_slot_t* frame_slot = &dst_decoder->frame_slots[i];

            if (frame_slot->initialized)
            {
                pthread_mutex_lock(&frame_slot->hMutex);
                frame_slot->state = SLOT_TERMINATING;
                pthread_cond_signal(&frame_slot->hEventPut);
                pthread_mutex_unlock(&frame_slot->hMutex);
                pthread_join(frame_slot->hThread, NULL);
                pthread_cond_destroy(&frame_slot->hEventGet);
                pthread_cond_destroy(&frame_slot->hEventPut);
                pthread_mutex_destroy(&frame_slot->hMutex);

                if (Close(&frame_slot->D) != 0)
                {
                    printf("PANIC: Could not close DST decoder slot");
                }

                frame_slot->initialized = 0;
            }
        }

        free(dst_decoder->frame_slots);
        free(dst_decoder);
    }
    catch (...)
    {
        printf("PANIC: Exception caught while destroying DST decoder");
    }

    return 0;
}

int dst_decoder_init_mt(dst_decoder_t* dst_decoder, int channel_count, int samplerate, int framerate)
{
    int i;

    for (i = 0; i < dst_decoder->thread_count; i++)
    {
        frame_slot_t* frame_slot = &dst_decoder->frame_slots[i];

        if (Init(&frame_slot->D, channel_count, (samplerate / 44100) / (framerate / 75)) == 0)
        {
            frame_slot->channel_count = channel_count;
            frame_slot->samplerate = samplerate;
            frame_slot->framerate = framerate;
            frame_slot->dsd_size = (size_t)(samplerate / 8 / framerate * channel_count);
            pthread_mutex_init(&frame_slot->hMutex, NULL);
            pthread_cond_init(&frame_slot->hEventGet, NULL);
            pthread_cond_init(&frame_slot->hEventPut, NULL);
        }
        else
        {
            printf("PANIC: Could not initialize decoder slot");
            return -1;
        }

        frame_slot->initialized = 1;
        pthread_create(&frame_slot->hThread, NULL, DSTDecoderThread, frame_slot);
    }

    dst_decoder->channel_count = channel_count;
    dst_decoder->samplerate = samplerate;
    dst_decoder->framerate = framerate;
    dst_decoder->frame_nr = 0;

    return 0;
}

int dst_decoder_decode_mt(dst_decoder_t* dst_decoder, uint8_t* dst_data, size_t dst_size, uint8_t** dsd_data, size_t* dsd_size)
{
    // Get current slot
    frame_slot_t* frame_slot = &dst_decoder->frame_slots[dst_decoder->slot_nr];

    // Allocate encoded frame into the slot
    frame_slot->dsd_data = *dsd_data;
    frame_slot->dst_data = dst_data;
    frame_slot->dst_size = dst_size;
    frame_slot->frame_nr = dst_decoder->frame_nr;

    // Release worker (decoding) thread on the loaded slot
    if (dst_size > 0)
    {
        pthread_mutex_lock(&frame_slot->hMutex);
        frame_slot->state = SLOT_LOADED;
        pthread_cond_signal(&frame_slot->hEventPut);
        pthread_mutex_unlock(&frame_slot->hMutex);
    }
    else
    {
        frame_slot->state = SLOT_EMPTY;
    }

    // Advance to the next slot
    dst_decoder->slot_nr = (dst_decoder->slot_nr + 1) % dst_decoder->thread_count;
    frame_slot = &dst_decoder->frame_slots[dst_decoder->slot_nr];

    // Dump decoded frame
    if (frame_slot->state != SLOT_EMPTY)
    {
        pthread_mutex_lock(&frame_slot->hMutex);

        while (frame_slot->state != SLOT_READY)
        {
            pthread_cond_wait(&frame_slot->hEventGet, &frame_slot->hMutex);
        }

        pthread_mutex_unlock(&frame_slot->hMutex);
    }

    switch (frame_slot->state)
    {
        case SLOT_READY:
            *dsd_data = frame_slot->dsd_data;
            *dsd_size = (size_t)(dst_decoder->samplerate / 8 / dst_decoder->framerate * dst_decoder->channel_count);
            break;
        default:
            *dsd_data = NULL;
            *dsd_size = 0;
            break;
    }

    dst_decoder->frame_nr++;

    return 0;
}
