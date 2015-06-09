/*
    Copyright 2015 Robert Tari <robert.tari@gmail.com>
    Copyright 2012 Vladislav Goncharov <vl-g@yandex.ru>
    Copyright 2012 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>

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

#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <stdio.h>
#include <locale>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "libsacd/sacd_reader.h"
#include "libsacd/sacd_disc.h"
#include "libsacd/sacd_dsdiff.h"
#include "libsacd/sacd_dsf.h"
#include "libsacd/version.h"
#include "libdsd2pcm/dsdpcm_converter.h"
#include "libdstdec/dst_decoder.h"

typedef struct
{
    const char * strIn;
    const char * strOut;
    bool bProgressLine;

} tThreadArgs;

typedef struct
{
    int nTrack;
    pthread_t hThread;
    float fProgress;
    tThreadArgs tArgs;

} tTrackArgs;

void packageInt(unsigned char * buf, int offset, int num, int bytes)
{
    buf[offset + 0] = (unsigned char)(num & 0xff);
    buf[offset + 1] = (unsigned char)((num >> 8) & 0xff);

    if (bytes == 4)
    {
        buf[offset + 2] = (unsigned char) ((num >> 0x10) & 0xff);
        buf[offset + 3] = (unsigned char) ((num >> 0x18) & 0xff);
    }
}

string toLower(const string& s)
{
    string result;

    locale loc;
    for (unsigned int i = 0; i < s.length(); ++i)
    {
        result += tolower(s.at(i), loc);
    }

    return result;
}

class input_sacd_t
{
    media_type_t media_type;
    sacd_media_t* sacd_media;
    sacd_reader_t* sacd_reader;
    area_id_e area_id;
    vector<uint8_t> dsd_buf;
    int dsd_buf_size;
    vector<uint8_t> dst_buf;
    int dst_buf_size;
    vector<float> pcm_buf;
    DstDec dst_decoder;
    dsdpcm_converter_t dsdpcm_convert;
    int dsd_samplerate;
    int framerate;
    bool m_bDstDecInit;

public:

    bool track_completed;
    int pcm_out_channels;
    unsigned int pcm_out_channel_map;
    float m_fProgress;

    input_sacd_t()
    {
        sacd_media = NULL;
        sacd_reader = NULL;
        sacd_reader = NULL;
        m_bDstDecInit = false;
        m_fProgress = 0;
    }

    virtual ~input_sacd_t()
    {
        if (sacd_reader)
        {
            delete sacd_reader;
        }

        if (sacd_media)
        {
            delete sacd_media;
        }

        if (m_bDstDecInit)
        {
            Close(&dst_decoder);
        }
    }

    int open(string p_path)
    {
        int nTracks = 0;

        string ext = toLower(p_path.substr(p_path.length()-3, 3));
        media_type = UNK_TYPE;

        if (ext == "iso")
        {
            media_type = ISO_TYPE;
        }
        else if (ext == "dat")
        {
            media_type = ISO_TYPE;
        }
        else if (ext == "dff")
        {
            media_type = DSDIFF_TYPE;
        }
        else if (ext == "dsf")
        {
            media_type = DSF_TYPE;
        }

        if (media_type == UNK_TYPE)
        {
            printf("PANIC: exception_io_unsupported_format\n");
            throw 100;
        }

        sacd_media = new sacd_media_file_t();

        if (!sacd_media)
        {
            printf("PANIC: exception_overflow\n");
            throw 100;
        }

        switch (media_type)
        {
            case ISO_TYPE:
                sacd_reader = new sacd_disc_t;
                if (!sacd_reader)
                {
                    printf("PANIC: exception_overflow\n");
                    throw 100;
                }
                break;
            case DSDIFF_TYPE:
                sacd_reader = new sacd_dsdiff_t;
                if (!sacd_reader)
                {
                    printf("PANIC: exception_overflow\n");
                    throw 100;
                }
                break;
            case DSF_TYPE:
                sacd_reader = new sacd_dsf_t;
                if (!sacd_reader)
                {
                    printf("PANIC: exception_overflow\n");
                    throw 100;
                }
                break;
            default:
                printf("PANIC: exception_io_data\n");
                throw 100;
                break;
        }

        if (!sacd_media->open(p_path.c_str()))
        {
            printf("PANIC: exception_io_data\n");
            throw 100;
        }

        if ((nTracks = sacd_reader->open(sacd_media, 0)) == 0)
        {
            printf("PANIC: exception_io_data\n");
            throw 100;
        }

        return nTracks;
    }

    string decode_initialize(uint32_t nSubsong)
    {
        sacd_reader->set_emaster(0);
        string strFileName = "";

        if (sacd_reader->get_track_count(AREA_MULCH) != 0)
        {
            strFileName = sacd_reader->set_track(nSubsong, AREA_MULCH, 0);
        }
        else
        {
            strFileName = sacd_reader->set_track(nSubsong, AREA_TWOCH, 0);
        }

        dsd_samplerate = sacd_reader->get_samplerate();
        framerate = sacd_reader->get_framerate();
        pcm_out_channels = sacd_reader->get_channels();

        switch (pcm_out_channels)
        {
            case 2:
                pcm_out_channel_map = 1<<0 | 1<<1;
                break;
            case 5:
                pcm_out_channel_map = 1<<0 | 1<<1 | 1<<2 | 1<<4 | 1<<5;
                break;
            case 6:
                pcm_out_channel_map = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5;
                break;
            default:
                pcm_out_channel_map = 0;
                break;
        }

        dst_buf_size = dsd_buf_size = dsd_samplerate / 8 / framerate * pcm_out_channels;
        dsd_buf.resize(dsd_buf_size);
        dst_buf.resize(dst_buf_size);
        pcm_buf.resize(pcm_out_channels * 96000 / 75);
        dsdpcm_convert.init(pcm_out_channels, dsd_samplerate, 96000);
        dsdpcm_convert.set_gain(0);
        track_completed = false;

        return strFileName;
    }

    int decode(FILE * pFile)
    {
        if (track_completed)
        {
            return false;
        }

        int dsd_size = 0;
        int dst_size = 0;
        int pcm_samples;
        int nFrame = 0;

        while (1)
        {
            dst_size = dst_buf_size;
            frame_type_e frame_type;

            if (sacd_reader->read_frame(dst_buf.data(), &dst_size, &frame_type))
            {
                if (dst_size > 0)
                {
                    if (frame_type == FRAME_INVALID)
                    {
                        dst_size = dst_buf_size;
                        memset(dst_buf.data(), 0xAA, dst_size);
                    }

                    if (frame_type == FRAME_DST)
                    {
                        if (!m_bDstDecInit)
                        {
                            if (Init(&dst_decoder, pcm_out_channels, dst_buf_size) != 0)
                            {
                                return false;
                            }

                            m_bDstDecInit = true;
                        }

                        Decode(&dst_decoder, dst_buf.data(), dsd_buf.data(), nFrame, &dst_size);
                        dsd_size = dst_buf_size;

                        nFrame++;
                    }
                    else
                    {
                        dsd_buf = dst_buf;
                        dsd_size = dst_size;
                    }

                    if (dsd_size > 0)
                    {
                        if (!dsdpcm_convert.is_convert_called())
                        {
                            pcm_samples = dsdpcm_convert.convert(dsd_buf.data(), pcm_buf.data(), dsd_size);
                            dsdpcm_convert.degibbs(pcm_buf.data(), pcm_samples, 0);
                        }
                        else
                        {
                            dsdpcm_convert.convert(dsd_buf.data(), pcm_buf.data(), dsd_size);
                        }

                        writeData(pFile, pcm_buf);

                        return true;
                    }
                }
            }
            else
            {
                break;
            }
        }

        dsd_buf.clear();
        dst_buf.clear();
        dst_size = 0;

        if (m_bDstDecInit)
        {
            Decode(&dst_decoder, dst_buf.data(), dsd_buf.data(), nFrame, &dst_size);
        }

        if (dsd_size > 0)
        {
            dsdpcm_convert.convert(dsd_buf.data(), pcm_buf.data(), dsd_size);
            writeData(pFile, pcm_buf);
            return true;
        }

        dsd_size = dsd_buf_size;
        memset(dsd_buf.data(), 0xAA, dsd_size);
        pcm_samples = dsdpcm_convert.convert(dsd_buf.data(), pcm_buf.data(), dsd_size);
        dsdpcm_convert.degibbs(pcm_buf.data(), pcm_samples, 1);
        writeData(pFile, pcm_buf);
        track_completed = true;

        return false;
    }

    void writeData(FILE * pFile, vector<float> audio_data)
    {
        int nDst = audio_data.size() * 3;
        unsigned char * pDst = new unsigned char[nDst];
        float * pSrc = audio_data.data();

        for(int i = 0; i != nDst; i += 3)
        {
            int nVal = lrintf ((*pSrc++) * 8388607.0) ;

            pDst[i] = nVal;
            pDst[i+1] = nVal >> 8;
            pDst[i+2] = nVal >> 16;
        }

        fwrite(pDst, sizeof(unsigned char), nDst, pFile);
        delete[] pDst;

        m_fProgress = sacd_reader->getProgress();
    }
};

void * fnThread (void* threadargs)
{
    tTrackArgs * ta = (tTrackArgs *)threadargs;
    input_sacd_t * pSacd = new input_sacd_t();
    int nTracks = pSacd->open(ta->tArgs.strIn);
    string strOutFile = ta->tArgs.strOut + pSacd->decode_initialize(ta->nTrack);

    bool rd = true;
    unsigned int nSize = 0x7fffffff;
    unsigned char arrHeader[68];
    unsigned char arrFormat[2] = {0xFE, 0xFF};
    unsigned char arrSubtype[16] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};

    memcpy (arrHeader, "RIFF", 4);
    packageInt (arrHeader, 4, nSize - 8, 4);
    memcpy (arrHeader + 8, "WAVE", 4);
    memcpy (arrHeader + 12, "fmt ", 4);
    packageInt (arrHeader, 16, 40, 4);
    memcpy (arrHeader + 20, arrFormat, 2);
    packageInt (arrHeader, 22, pSacd->pcm_out_channels, 2);
    packageInt (arrHeader, 24, 96000, 4);
    packageInt (arrHeader, 28, pSacd->pcm_out_channels * 288000, 4);
    packageInt (arrHeader, 32, pSacd->pcm_out_channels * 3, 2);
    packageInt (arrHeader, 34, 24, 2);
    packageInt (arrHeader, 36, 22, 2);
    packageInt (arrHeader, 38, 24, 2);
    packageInt (arrHeader, 40, pSacd->pcm_out_channel_map, 4);
    memcpy (arrHeader + 44, arrSubtype, 16);
    memcpy (arrHeader + 60, "data", 4);
    packageInt (arrHeader, 64, nSize - 68, 4);
    FILE * pFile = fopen(strOutFile.data(), "wb");
    fwrite(arrHeader, 1, 68, pFile);

    while (pSacd->track_completed == false || rd == true)
    {
        rd = pSacd->decode(pFile);
        ta->fProgress = pSacd->m_fProgress;
    }

    nSize = ftell (pFile);
    packageInt (arrHeader, 4, nSize - 8, 4);
    packageInt (arrHeader, 64, nSize - 68, 4);
    fseek (pFile, 0, SEEK_SET);
    fwrite (arrHeader, 1, 68, pFile);
    fclose(pFile);

    ta->fProgress = 100;

    if (ta->tArgs.bProgressLine)
    {
        printf("FILE\t%s\t%.2i\t%.2i\n", strOutFile.data(), ta->nTrack + 1, nTracks);
    }

    return 0;
}

void * fnProgress (void* threadargs)
{
    vector<tTrackArgs> * arrTA = (vector<tTrackArgs> *)threadargs;

    while(1)
    {
        float fProgress = 0.0;

        for (size_t i = 0; i < arrTA->size(); i++)
        {
            fProgress += arrTA->at(i).fProgress;
        }

        fProgress = fProgress / arrTA->size();

        if (arrTA->at(0).tArgs.bProgressLine)
        {
            printf("PROGRESS\t%.2f\n", fProgress);
        }
        else
        {
            printf("\r%.2f%%", fProgress);
        }

        fflush(stdout);

        if(fProgress == 100)
            break;

        sleep(1);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    string strIn = "";
    string strOut = "";
    char strPath[PATH_MAX];
    int nOpt;
    bool bProgressLine = false;
    bool bPrintHelp = false;

    const char strHelpText[] =
    "\n"
    "Usage: sacd -i infile [-o outdir] [options]\n\n"
    "  -i, --infile         : Specify the input file (*.iso, *.dsf, *.dff)\n"
    "  -o, --outdir         : The folder to write the WAVE files to. If you omit\n"
    "                         this, the files will be placed in the input file's\n"
    "                         directory\n"
    "  -p, --progress       : Display progress to new lines. Use this if you intend\n"
    "                         to parse the output through a script. This option only\n"
    "                         lists either one progress percentage per line, or one\n"
    "                         status/error message.\n"
    "  -h, --help           : Show this help message\n\n";

    const struct option tOptionsTable[] =
    {
        {"infile", required_argument, NULL, 'i' },
        {"outdir", required_argument, NULL, 'o' },
        {"progress", no_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    while ((nOpt = getopt_long(argc, argv, "i:o:ph", tOptionsTable, NULL)) >= 0)
    {
        switch (nOpt)
        {
            case 'i':
                strIn = optarg;
                break;
            case 'o':
                strOut = optarg;
                break;
            case 'p':
                bProgressLine = true;
                break;
            default:
                bPrintHelp = true;
                break;
        }
    }

    if (bPrintHelp || argc == 1 || strIn.empty())
    {
        printf(bProgressLine ? "PANIC: Invalid command-line syntax\n" : strHelpText);
        return 0;
    }

    struct stat tStat;

    if (stat(strIn.c_str(), &tStat) == -1 || !S_ISREG(tStat.st_mode))
    {
        printf("PANIC: Input file does not exist\n");
        return 0;
    }

    strIn = realpath(strIn.data(), strPath);

    if (strOut.empty())
    {
        strOut = strIn.substr(0, strIn.find_last_of("/") + 1);
    }

    if (strOut.empty() || stat(strOut.c_str(), &tStat) == -1 || !S_ISDIR(tStat.st_mode))
    {
        printf("PANIC: Output directory does not exist\n");
        return 0;
    }

    strOut = realpath(strOut.data(), strPath);

    if (strOut.compare(strOut.size() - 1, 1, "/") != 0)
    {
        strOut += "/";
    }

    input_sacd_t * pSacd = new input_sacd_t();
    int nTracks = pSacd->open(strIn);
    delete pSacd;

    tThreadArgs tArgs;
    tArgs.strIn = strIn.c_str();
    tArgs.strOut = strOut.c_str();
    tArgs.bProgressLine = bProgressLine;
    vector<tTrackArgs> arrTracks(nTracks);
    pthread_t hProgressThread;

    time_t now = time(0);

    if (!bProgressLine)
    {
        printf("\n\nsacd - Command-line SACD decoder version %s\n\n", APPVERSION);
    }

    for (int i = 0; i < nTracks; i++)
    {
        arrTracks[i].nTrack = i;
        arrTracks[i].fProgress = 0.0;
        arrTracks[i].tArgs = tArgs;

        pthread_create(&arrTracks[i].hThread, NULL, fnThread, &arrTracks[i]);
        pthread_detach(arrTracks[i].hThread);
    }

    pthread_create(&hProgressThread, NULL, fnProgress, &arrTracks);
    pthread_join(hProgressThread, NULL);

    int nSeconds = time(0) - now;

    if (bProgressLine)
    {
        printf("FINISHED\t%d\n", nSeconds);
    }
    else
    {
        printf("\nFinished in %d seconds.\n\n", nSeconds);
    }

    return 0;
}

//1040/72
