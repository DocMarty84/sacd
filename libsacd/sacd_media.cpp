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

#include <unistd.h>
#include <sys/stat.h>
#include "scarletbook.h"
#include "sacd_media.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

sacd_media_file_t::sacd_media_file_t()
{
}

sacd_media_file_t::~sacd_media_file_t()
{
}

bool sacd_media_file_t::open(const char* path)
{
    try
    {
        media_file = fopen(path, "r");
        return true;
    }
    catch (...)
    {
    }

    return false;
}

bool sacd_media_file_t::close()
{
    return true;
}

bool sacd_media_file_t::seek(int64_t position, int mode)
{
    fseek(media_file, position, mode);
    return true;
}

int64_t sacd_media_file_t::get_position()
{
    return ftell(media_file);
}

int64_t sacd_media_file_t::get_size()
{
    struct stat stat_buf;
    fstat(fileno(media_file), &stat_buf);
    return stat_buf.st_size;
}

size_t sacd_media_file_t::read(void* data, size_t size)
{
    return fread(data, 1, size, media_file);
}

size_t sacd_media_file_t::write(const void* data, size_t size)
{
    return fwrite(data, 1, size, media_file);
}

int64_t sacd_media_file_t::skip(int64_t bytes)
{
    return fseek(media_file, bytes, SEEK_CUR);
}

int sacd_media_file_t::truncate(int64_t position)
{
    return ftruncate(fileno(media_file), position);
}
