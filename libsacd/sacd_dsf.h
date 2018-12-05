/*
    Copyright 2015-2016 Robert Tari <robert.tari@gmail.com>
    Copyright 2011-2016 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#ifndef __SACD_DSF_H__
#define __SACD_DSF_H__

#include <cstdint>
#include <vector>
#include "endianess.h"
#include "scarletbook.h"
#include "sacd_reader.h"
#include "sacd_dsd.h"

#pragma pack(1)

class FmtDSFChunk : public Chunk
{
public:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    uint32_t format_id;
    uint32_t channel_type;
    uint32_t channel_count;
    uint32_t samplerate;
    uint32_t bits_per_sample;
    uint64_t sample_count;
    uint32_t block_size;
#pragma clang diagnostic pop
};

#pragma pack()

class sacd_dsf_t : public sacd_reader_t
{
public:
    sacd_dsf_t();

    ~sacd_dsf_t() override;

    uint32_t get_track_count(area_id_e area_id) override;

    int get_channels() override;

    int get_samplerate() override;

    int get_framerate() override;

    float getProgress() override;

    int open(sacd_media_t *p_file) override;

    bool close() override;

    string set_track(int track_number, area_id_e area_id, uint32_t offset) override;

    bool read_frame(uint8_t *frame_data, size_t *frame_size, frame_type_e *frame_type) override;

private:
    sacd_media_t *m_file;
    int m_samplerate;
    int m_channel_count;
    uint64_t m_file_size;
    vector<uint8_t> m_block_data;
    int m_block_size;
    int m_block_offset;
    int m_block_data_end;
    uint64_t m_sample_count;
    uint64_t m_data_offset;
    uint64_t m_data_size;
    uint64_t m_data_end_offset;
    uint64_t m_read_offset;
    bool m_is_lsb;
    uint64_t m_id3_offset;
    uint8_t swap_bits[256];
};

#endif  // __SACD_DSF_H__
