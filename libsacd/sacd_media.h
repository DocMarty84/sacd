/*
    Copyright 2015-2016 Robert Tari <robert.tari@gmail.com>
    Copyright 2011-2012 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#ifndef __SACD_MEDIA_H__
#define __SACD_MEDIA_H__

#include <cstdint>
#include <cstring>
#include <string>
#include <cstdio>

#ifndef MARK_TIME
#define MARK_TIME(m) ((double)(m).hours * 60 * 60 + (double)(m).minutes * 60 + (double)(m).seconds + ((double)(m).samples + (double)(m).offset) / (double)m_samplerate)
#endif  // MARK_TIME

#ifndef MIN
#define MIN(a, b) (((a)<(b))?(a):(b))
#endif  // MIN

#ifndef MAX
#define MAX(a, b) (((a)>(b))?(a):(b))
#endif  // MAX

using namespace std;

class sacd_media_t
{
public:
    sacd_media_t();

    virtual ~sacd_media_t();

    virtual bool open(const char *path);

    virtual bool close();

    virtual bool seek(int64_t position, int mode);

    bool seek(int64_t position);

    virtual int64_t get_position();

    virtual size_t read(void *data, size_t size);

    virtual int64_t skip(int64_t bytes);

    virtual string getFileName();

private:
    FILE *media_file;
    string m_strFilePath;
};

#endif  // __SACD_MEDIA_H__
