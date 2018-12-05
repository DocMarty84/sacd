/*
    Copyright (c) 2015-2016 Robert Tari <robert.tari@gmail.com>
    Copyright (c) 2011-2015 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#ifndef __DSD_PCM_CONSTANTS_H__
#define __DSD_PCM_CONSTANTS_H__

#include <cstdint>


#define DSDxFs64 (44100 * 64)
#define DSDxFs128 (44100 * 128)
#define DSDxFs256 (44100 * 256)
#define DSDxFs512 (44100 * 512)
#define DSD_SILENCE_BYTE 0x69
#define DSDPCM_MAX_CHANNELS 6

#endif // __DSD_PCM_CONSTANTS_H__
