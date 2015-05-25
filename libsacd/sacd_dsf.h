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

#ifndef _SACD_DSF_H_INCLUDED
#define _SACD_DSF_H_INCLUDED

#include <stdint.h>
#include <vector>
#include "endianess.h"
#include "scarletbook.h"
#include "sacd_reader.h"
#include "sacd_dsd.h"

#pragma pack(1)

class FmtDSFChunk : public Chunk {
public:
    uint32_t format_version;
    uint32_t format_id;
    uint32_t channel_type;
    uint32_t channel_count;
    uint32_t samplerate;
    uint32_t bits_per_sample;
    uint64_t sample_count;
    uint32_t block_size;
    uint32_t reserved;
};

#pragma pack()

class sacd_dsf_t : public sacd_reader_t {
    sacd_media_t*       m_file;
    int                 m_version;
    int                 m_samplerate;
    int                 m_channel_count;
    int                 m_loudspeaker_config;
    uint64_t            m_file_size;
    std::vector<uint8_t>    m_block_data;
    int                 m_block_size;
    int                 m_block_offset;
    int                 m_block_data_end;
    uint64_t            m_sample_count;
    uint64_t            m_data_offset;
    uint64_t            m_data_size;
    uint64_t            m_read_offset;
    bool                m_is_lsb;
    uint64_t            m_id3_offset;
    std::vector<uint8_t>    m_id3_data;
    uint8_t             swap_bits[256];
public:
    sacd_dsf_t();
    virtual ~sacd_dsf_t();
    uint32_t get_track_count(area_id_e area_id = AREA_BOTH);
    int get_channels();
    int get_loudspeaker_config();
    int get_samplerate();
    int get_framerate();
    uint64_t get_size();
    uint64_t get_offset();
    float getProgress();
    double get_duration();
    double get_duration(uint32_t subsong);
    bool is_dst();
    bool open(sacd_media_t* p_file, uint32_t mode = 0);
    bool close();
    void set_area(area_id_e area_id);
    void set_emaster(bool emaster);
    bool set_track(uint32_t track_number, area_id_e area_id = AREA_BOTH, uint32_t offset = 0);
    bool read_frame(uint8_t* frame_data, int* frame_size, frame_type_e* frame_type);
    bool seek(double seconds);
    bool commit();
private:
    void write_id3tag(const void* data, uint32_t size);
};

#endif
