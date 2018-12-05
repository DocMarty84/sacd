//
// Created by Zhao Wang on 2018-12-05.
//

#ifndef SACD_SACD_CORE_H
#define SACD_SACD_CORE_H

#include "libdsd2pcm/dsd_pcm_converter.h"

extern int g_nCPUs;

class SACD
{
private:

    sacd_media_t* m_pSacdMedia;
    dst_decoder_t* m_pDstDecoder;
    vector<uint8_t> m_arrDstBuf;
    vector<uint8_t> m_arrDsdBuf;
    vector<float> m_arrPcmBuf;
    int m_nDsdBufSize;
    int m_nDstBufSize;
    DSDPCMConverter* m_pDsdPcmConverter480;
    int m_nDsdSamplerate;
    int m_nFramerate;
    int m_nPcmOutSamples;
    int m_nPcmOutDelta;

    void dsd2pcm(uint8_t* dsd_data, int dsd_samples, float* pcm_data);

    void writeData(FILE * pFile, int nOffset, int nSamples);

public:

    int m_nTracks;
    float m_fProgress;
    int m_nPcmOutChannels;
    sacd_reader_t* m_pSacdReader;
    bool m_bTrackCompleted;

    SACD();
    ~SACD();

    int open(const string &p_path);

    string init(uint32_t nSubsong, int g_nSampleRate, area_id_e nArea);

    void fixPcmStream(bool bIsEnd, float* pPcmData, int nPcmSamples);

    bool decode(FILE* pFile);
};

#endif //SACD_SACD_CORE_H
