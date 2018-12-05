/*

    MPEG-4 Audio RM Module
    Lossless coding of 1-bit oversampled audio - DST (Direct Stream Transfer)

    This software was originally developed by:

    * Aad Rijnberg
    Philips Digital Systems Laboratories Eindhoven
    <aad.rijnberg@philips.com>

    * Fons Bruekers
    Philips Research Laboratories Eindhoven
    <fons.bruekers@philips.com>

    * Eric Knapen
    Philips Digital Systems Laboratories Eindhoven
    <h.w.m.knapen@philips.com>

    And edited by:

    * Richard Theelen
    Philips Digital Systems Laboratories Eindhoven
    <r.h.m.theelen@philips.com>

    * Maxim V.Anisiutkin
    <maxim.anisiutkin@gmail.com>

    * Robert Tari
    <robert.tari@gmail.com>

    in the course of development of the MPEG-4 Audio standard ISO-14496-1, 2 and 3.
    This software module is an implementation of a part of one or more MPEG-4 Audio
    tools as specified by the MPEG-4 Audio standard. ISO/IEC gives users of the
    MPEG-4 Audio standards free licence to this software module or modifications
    thereof for use in hardware or software products claiming conformance to the
    MPEG-4 Audio standards. Those intending to use this software module in hardware
    or software products are advised that this use may infringe existing patents.
    The original developers of this software of this module and their company,
    the subsequent editors and their companies, and ISO/EIC have no liability for
    use of this software module or modifications thereof in an implementation.
    Copyright is not released for non MPEG-4 Audio conforming products. The
    original developer retains full right to use this code for his/her own purpose,
    assign or donate the code to a third party and to inhibit third party from
    using the code for non MPEG-4 Audio conforming products. This copyright notice
    must be included in all copies of derivative works.

    Copyright � 2004-2016.

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

#ifndef __DST_CONSTS_H__
#define __DST_CONSTS_H__

#define RESOL 8

//PREDICTION
#define SIZE_CODEDPREDORDER 7 // Number of bits in the stream for representing the "CodedPredOrder" in each frame
#define SIZE_PREDCOEF 9 // Number of bits in the stream for representing each filter coefficient in each frame

//RITHMETIC CODING
#define AC_BITS 8 // Number of bits and maximum level for coding the probability
#define AC_PROBS (1 << AC_BITS)
#define AC_HISBITS 6 // Number of entries in the histogram
#define AC_HISMAX (1 << AC_HISBITS)
#define AC_QSTEP (SIZE_PREDCOEF - AC_HISBITS) // Quantization step for histogram

//RICE CODING OF PREDICTION COEFFICIENTS AND PTABLES
#define NROFFRICEMETHODS 3 // Number of different Pred. Methods for filters used in combination with Rice coding
#define NROFPRICEMETHODS 3 // Number of different Pred. Methods for Ptables used in combination with Rice coding
#define MAXCPREDORDER 3 // max pred.order for prediction of filter coefs / Ptables entries
#define SIZE_RICEMETHOD 2 // nr of bits in stream for indicating method
#define SIZE_RICEM 3 // nr of bits in stream for indicating m

// SEGMENTATION
#define MAXNROF_FSEGS 4 // max nr of segments per channel for filters
#define MAXNROF_PSEGS 8 // max nr of segments per channel for Ptables
#define MIN_FSEG_LEN 1024 // min segment length in bits of filters
#define MIN_PSEG_LEN 32 // min segment length in bits of Ptables

// 64FS44 =>  4704, 128FS44 =>  9408, 256FS44 => 18816
#define MAX_DSDBYTES_INFRAME 18816
#define MAX_CHANNELS 6
#define MAX_DSDBITS_INFRAME (8 * MAX_DSDBYTES_INFRAME)

#define MAXNROF_SEGS 8 // max nr of segments per channel for filters or Ptables

#endif // __DST_CONSTS_H__
