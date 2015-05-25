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

#ifndef _dither_h_
#define _dither_h_

// inline dithering implementation
class Dither {
public:
    Dither(unsigned int n_bits);
    Dither& operator=(const Dither &obj);

    double  processSample(double x) { return (x + m_rand_max * (double)(fast_rand() - (RAND_MAX / 2)) / (double)(RAND_MAX / 2)); }

protected:
    double  m_rand_max;
    int     m_holdrand;
    
    int     fast_rand() { return (((m_holdrand = m_holdrand * 214013L + 2531011L) >> 16) & 0x7fff); }   // libc rand() is too slow
};


#endif
