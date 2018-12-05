/*
    Copyright 2018-2020 Zhao Wang <zhaow.km@gmail.com>

    This file is part of sacd2wav.

    sacd2wav is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sacd2wav is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with sacd2wav.  If not, see <http://www.gnu.org/licenses/gpl-3.0.txt>.
*/

#include <vector>
#include <string>
#include <thread>
#include <locale>
#include <getopt.h>
#include <sys/stat.h>
#include "libsacd/sacd_reader.h"
#include "version.h"
#include "libdstdec/dst_decoder_mt.h"
#include "converter_core.h"

struct TrackInfo
{
    int nTrack;
    area_id_e nArea;
};

int g_nCPUs = 2;
int g_nThreads = 2;
vector<TrackInfo> g_arrQueue;
pthread_mutex_t g_hMutex = PTHREAD_MUTEX_INITIALIZER;
string g_strOut;
int g_nSampleRate = 192000;
int g_nFinished = 0;


void packageInt(unsigned char *buf, int offset, int num, int bytes)
{
    buf[offset + 0] = (unsigned char) (num & 0xff);
    buf[offset + 1] = (unsigned char) ((num >> 8) & 0xff);

    if (bytes == 4) {
        buf[offset + 2] = (unsigned char) ((num >> 0x10) & 0xff);
        buf[offset + 3] = (unsigned char) ((num >> 0x18) & 0xff);
    }
}


void *fnProgress(void *threadargs)
{
    auto *arrSACD = (vector<ConverterCore *> *) threadargs;

    while (true) {
        float progress = 0;
        int tracks = (*arrSACD).at(0)->m_nTracks;

        for (int i = 0; i < g_nThreads; i++) {
            progress += (*arrSACD).at(static_cast<size_t>(static_cast<unsigned int>(i)))->m_fProgress;
        }

        progress = MAX((((tracks - MIN(g_nThreads, tracks) - g_arrQueue.size()) * 100.0f) + progress) / tracks, 0.0f);

        printf("\r%.2f%%", progress);
        fflush(stdout);

        if (g_nFinished == tracks) {
            break;
        }
        sleep(1);
    }
    return nullptr;
}

void *fnDecode(void *threadargs)
{
    auto *pSACD = (ConverterCore *) threadargs;

    while (!g_arrQueue.empty()) {
        pthread_mutex_lock(&g_hMutex);

        TrackInfo cTrackInfo = g_arrQueue.front();
        g_arrQueue.erase(g_arrQueue.begin());

        pthread_mutex_unlock(&g_hMutex);

        string strOutFile = g_strOut + pSACD->init(cTrackInfo.nTrack, g_nSampleRate, cTrackInfo.nArea);
        unsigned int nSize = 0x7fffffff;
        unsigned char arrHeader[44];
        unsigned char arrFormat[2] = {0x01, 0x00};

        memcpy(arrHeader, "RIFF", 4);
        packageInt(arrHeader, 4, nSize - 8, 4);
        memcpy(arrHeader + 8, "WAVE", 4);
        memcpy(arrHeader + 12, "fmt ", 4);
        packageInt(arrHeader, 16, 16, 4);
        memcpy(arrHeader + 20, arrFormat, 2);
        packageInt(arrHeader, 22, pSACD->m_nPcmOutChannels, 2);
        packageInt(arrHeader, 24, g_nSampleRate, 4);
        packageInt(arrHeader, 28, (g_nSampleRate * 24 * pSACD->m_nPcmOutChannels) / 8, 4);
        packageInt(arrHeader, 32, pSACD->m_nPcmOutChannels * 3, 2);
        packageInt(arrHeader, 34, 24, 2);
        memcpy(arrHeader + 36, "data", 4);
        packageInt(arrHeader, 40, nSize - 44, 4);
        FILE *pFile = fopen(strOutFile.data(), "wb");
        fwrite(arrHeader, 1, 44, pFile);

        bool bDone = false;

        while (!bDone || !pSACD->m_bTrackCompleted) {
            bDone = pSACD->decode(pFile);
        }

        nSize = static_cast<unsigned int>(ftell(pFile));
        packageInt(arrHeader, 4, nSize - 8, 4);
        packageInt(arrHeader, 40, nSize - 44, 4);
        fseek(pFile, 0, SEEK_SET);
        fwrite(arrHeader, 1, 44, pFile);
        fclose(pFile);

        printf("\r(%.2i/%.2i) \"%s\" written to disk\n", cTrackInfo.nTrack + 1, pSACD->m_nTracks, strOutFile.data());

        g_nFinished++;
    }

    return nullptr;
}

int main(int argc, char *argv[])
{
    g_nCPUs = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));

    string strIn;
    char strPath[PATH_MAX];
    int nOpt;
    bool bPrintHelp = false;

    const char strHelpText[] =
            "sacd2wav - a utility to convert SACD (Super Audio CD) file to HiRes wave\n"
            "           files ready for TASCAM DR-100 Mark 2/3\n"
            "           (version " APPVERSION ")\n"
            "usage: sacd2wav -i infile [-o outdir] [options]\n"
            "  -i, --infile         : Specify the input file (*.iso, *.dsf, *.dff)\n"
            "  -o, --outdir         : The folder to write the WAVE files to. If you omit\n"
            "                         this, the files will be placed in the input file's\n"
            "                         directory\n"
            "  -r, --rate           : The output sample rate.\n"
            "                         Valid rates are: 96000 or 192000.\n"
            "                         If you omit this, 192KHz will be used.\n"
            "  -h, --help           : Show this help message\n\n";

    const struct option tOptionsTable[] =
            {
                    {"infile", required_argument, nullptr, 'i'},
                    {"outdir", required_argument, nullptr, 'o'},
                    {"rate",   required_argument, nullptr, 'r'},
                    {"help",   no_argument,       nullptr, 'h'},
                    {nullptr, 0,                  nullptr, 0}
            };

    while ((nOpt = getopt_long(argc, argv, "i:o:r:h", tOptionsTable, nullptr)) >= 0) {
        switch (nOpt) {
        case 'i':
            strIn = optarg;
            break;
        case 'o':
            g_strOut = optarg;
            break;
        case 'r': {
            string s = optarg;

            if (s == "96000" || s == "192000") {
                g_nSampleRate = stoi(optarg);
            } else {
                printf("Error: invalid sample rate, only 96000 and 192000 is supported\n");
                return 4;
            }
            break;
        }
        default:
            bPrintHelp = true;
            break;
        }
    }

    if (bPrintHelp || argc == 1 || strIn.empty()) {
        printf("Error: illegal option\n");
        printf(strHelpText);
        return 1;
    }

    struct stat tStat{};

    if (stat(strIn.c_str(), &tStat) == -1 || !S_ISREG(tStat.st_mode)) {
        printf("Error: input file does not exist\n");
        return 2;
    }

    strIn = realpath(strIn.data(), strPath);

    if (g_strOut.empty()) {
        g_strOut = strIn.substr(0, strIn.find_last_of('/') + 1);
    }

    if (g_strOut.empty() || stat(g_strOut.c_str(), &tStat) == -1 || !S_ISDIR(tStat.st_mode)) {
        printf("Error: output directory does not exist\n");
        return 3;
    }

    g_strOut = realpath(g_strOut.data(), strPath);

    if (g_strOut.compare(g_strOut.size() - 1, 1, "/") != 0) {
        g_strOut += "/";
    }

    auto *pSacd = new ConverterCore();

    if (!pSacd->open(strIn)) {
        exit(1);
    }

    int nTwoch = pSacd->m_pSacdReader->get_track_count(AREA_TWOCH);
    int nMulch = pSacd->m_pSacdReader->get_track_count(AREA_MULCH);

    if (nTwoch > 0) {
        for (int i = 0; i < nTwoch; i++) {
            TrackInfo cTrackInfo = {i, AREA_TWOCH};
            g_arrQueue.push_back(cTrackInfo);
        }
    } else {
        for (int i = 0; i < nMulch; i++) {
            TrackInfo cTrackInfo = {i, AREA_MULCH};
            g_arrQueue.push_back(cTrackInfo);
        }
    }

    delete pSacd;

    g_nThreads = MIN(g_nCPUs, (int) g_arrQueue.size());

    time_t nNow = time(nullptr);
    pthread_t hThreadProgress;
    vector<ConverterCore *> arrSACD(static_cast<size_t>(g_nThreads));
    vector<pthread_t> arrThreads(static_cast<size_t>(g_nThreads));

    for (int i = 0; i < g_nThreads; i++) {
        arrSACD[i] = new ConverterCore();
        arrSACD[i]->open(strIn);
        pthread_create(&arrThreads[i], nullptr, fnDecode, arrSACD[i]);
        pthread_detach(arrThreads[i]);
    }

    pthread_create(&hThreadProgress, nullptr, fnProgress, &arrSACD);
    pthread_join(hThreadProgress, nullptr);
    pthread_mutex_destroy(&g_hMutex);

    for (int i = 0; i < g_nThreads; i++) {
        delete arrSACD[i];
    }

    int nSeconds = (int) (time(nullptr)) - (int) nNow;

    printf("\nFinished in %d seconds.\n\n", nSeconds);

    return 0;
}
