//
// Created by Zhao Wang on 2018-12-05.
//

#include "libdstdec/dst_decoder_mt.h"
#include "libdsd2pcm/dsd_pcm_converter.h"
#include "libsacd/sacd_dsf.h"
#include "libsacd/sacd_dsdiff.h"
#include "libsacd/sacd_disc.h"
#include <locale>
#include <cmath>
#include "converter_core.h"

string toLower(const string &s)
{
    string result;
    locale loc;

    for (char i : s) {
        result += tolower(i, loc);
    }

    return result;
}

void ConverterCore::dsd2pcm(uint8_t *dsd_data, int dsd_samples, float *pcm_data)
{
    m_pDsdPcmConverter->convert(dsd_data, dsd_samples, pcm_data);
}

void ConverterCore::writeData(FILE *pFile, int nOffset, int nSamples)
{
    int nFramesIn = nSamples * m_nPcmOutChannels;
    int nBytesOut = nFramesIn * 3;
    auto pSrc = (char *) (m_arrPcmBuf.data() + nOffset * m_nPcmOutChannels);
    auto pDst = new char[nBytesOut];
    float fSample;
    int32_t nVal;
    int nOut = 0;

    for (int nFrame = 0; nFrame < nFramesIn; nFrame++, pSrc += 4) {
        fSample = *(float *) (pSrc);
        fSample = MIN(fSample, 1.0f);
        fSample = MAX(fSample, -1.0f);
        fSample *= 8388608.0;

        nVal = static_cast<int32_t>(lrintf(fSample));
        nVal = MIN(nVal, 8388607);
        nVal = MAX(nVal, -8388608);

        pDst[nOut++] = static_cast<char>(nVal);
        pDst[nOut++] = static_cast<char>(nVal >> 8);
        pDst[nOut++] = static_cast<char>(nVal >> 16);
    }

    fwrite(pDst, 1, static_cast<size_t>(nBytesOut), pFile);
    delete[] pDst;

    m_fProgress = m_pSacdReader->getProgress();
}

ConverterCore::ConverterCore()
{
    m_pSacdMedia = nullptr;
    m_pSacdReader = nullptr;
    m_pDsdPcmConverter = nullptr;
    m_pDstDecoder = nullptr;
    m_fProgress = 0;
    m_nTracks = 0;
    m_nPcmOutSamples = 0;
    m_nPcmOutDelta = 0;
}

ConverterCore::~ConverterCore()
{
    delete m_pSacdReader;
    delete m_pSacdMedia;
    delete m_pDsdPcmConverter;
    delete m_pDstDecoder;
}

int ConverterCore::open(const string &p_path)
{
    string ext = toLower(p_path.substr(p_path.length() - 3, 3));
    media_type_t tMediaType = UNK_TYPE;

    if (ext == "iso") {
        tMediaType = ISO_TYPE;
    } else if (ext == "dat") {
        tMediaType = ISO_TYPE;
    } else if (ext == "dff") {
        tMediaType = DSDIFF_TYPE;
    } else if (ext == "dsf") {
        tMediaType = DSF_TYPE;
    }

    m_pSacdMedia = new sacd_media_t();

    if (!m_pSacdMedia) {
        printf("Error: exception_overflow\n");
        return 0;
    }

    switch (tMediaType) {
    case ISO_TYPE:
        m_pSacdReader = new sacd_disc_t;
        if (!m_pSacdReader) {
            printf("Error: exception_overflow\n");
            return 0;
        }
        break;
    case DSDIFF_TYPE:
        m_pSacdReader = new sacd_dsdiff_t;
        if (!m_pSacdReader) {
            printf("Error: exception_overflow\n");
            return 0;
        }
        break;
    case DSF_TYPE:
        m_pSacdReader = new sacd_dsf_t;
        if (!m_pSacdReader) {
            printf("Error: exception_overflow\n");
            return 0;
        }
        break;
    default:
        printf("Error: exception_io_unsupported_format\n");
        return 0;
    }

    if (!m_pSacdMedia->open(p_path.c_str())) {
        printf("Error: exception_io_data\n");
        return 0;
    }

    if ((m_nTracks = m_pSacdReader->open(m_pSacdMedia)) == 0) {
        printf("Error: failed to parse SACD media\n");
        return 0;
    }

    return m_nTracks;
}

string ConverterCore::init(int nTrack, int g_nSampleRate, area_id_e nArea)
{
    if (m_pDsdPcmConverter) {
        delete m_pDsdPcmConverter;
        m_pDsdPcmConverter = nullptr;
    }

    if (m_pDstDecoder) {
        delete m_pDstDecoder;
        m_pDstDecoder = nullptr;
    }

    string strFileName = m_pSacdReader->set_track(nTrack, nArea, 0);
    m_nDsdSamplerate = m_pSacdReader->get_samplerate();
    m_nFramerate = m_pSacdReader->get_framerate();
    m_nPcmOutSamples = g_nSampleRate / m_nFramerate;
    m_nPcmOutChannels = m_pSacdReader->get_channels();
    m_nDstBufSize = m_nDsdBufSize = m_nDsdSamplerate / 8 / m_nFramerate * m_nPcmOutChannels;
    m_arrDsdBuf.resize(static_cast<size_t>(static_cast<unsigned int>(m_nDsdBufSize * g_nCPUs)));
    m_arrDstBuf.resize(static_cast<size_t>(static_cast<unsigned int>(m_nDstBufSize * g_nCPUs)));
    m_arrPcmBuf.resize(static_cast<size_t>(static_cast<unsigned int>(m_nPcmOutChannels * m_nPcmOutSamples)));
    m_pDsdPcmConverter = new DSDPCMConverter();
    m_pDsdPcmConverter->init(m_nPcmOutChannels, m_nDsdSamplerate, g_nSampleRate);

    float fPcmOutDelay = m_pDsdPcmConverter->get_delay();

    m_nPcmOutDelta = (int) (fPcmOutDelay - 0.5f);

    if (m_nPcmOutDelta > m_nPcmOutSamples - 1) {
        m_nPcmOutDelta = m_nPcmOutSamples - 1;
    }

    m_bTrackCompleted = false;

    return strFileName;
}

void ConverterCore::fixPcmStream(bool bIsEnd, float *pPcmData, int nPcmSamples)
{
    if (!bIsEnd) {
        if (nPcmSamples > 1) {
            for (int ch = 0; ch < m_nPcmOutChannels; ch++) {
                pPcmData[0 * m_nPcmOutChannels + ch] = pPcmData[1 * m_nPcmOutChannels + ch];
            }
        }
    } else {
        if (nPcmSamples > 1) {
            for (int ch = 0; ch < m_nPcmOutChannels; ch++) {
                pPcmData[(nPcmSamples - 1) * m_nPcmOutChannels + ch] = pPcmData[(nPcmSamples - 2) * m_nPcmOutChannels +
                                                                                ch];
            }
        }
    }
}

bool ConverterCore::decode(FILE *pFile)
{
    if (m_bTrackCompleted) {
        return true;
    }

    uint8_t *pDsdData;
    uint8_t *pDstData;
    size_t nDsdSize = 0;
    size_t nDstSize = 0;
    int nThread = 0;

    while (true) {
        nThread = m_pDstDecoder ? m_pDstDecoder->slot_nr : 0;
        pDsdData = m_arrDsdBuf.data() + m_nDsdBufSize * nThread;
        pDstData = m_arrDstBuf.data() + m_nDstBufSize * nThread;
        nDstSize = static_cast<size_t>(m_nDstBufSize);
        frame_type_e nFrameType;

        if (m_pSacdReader->read_frame(pDstData, &nDstSize, &nFrameType)) {
            if (nDstSize > 0) {
                if (nFrameType == FRAME_INVALID) {
                    nDstSize = static_cast<size_t>(m_nDstBufSize);
                    memset(pDstData, DSD_SILENCE_BYTE, nDstSize);
                }

                if (nFrameType == FRAME_DST) {
                    if (!m_pDstDecoder) {
                        m_pDstDecoder = new dst_decoder_t(g_nCPUs);

                        if (!m_pDstDecoder ||
                            m_pDstDecoder->init(m_nPcmOutChannels, m_nDsdSamplerate, m_nFramerate) != 0) {
                            return true;
                        }
                    }

                    m_pDstDecoder->decode(pDstData, nDstSize, &pDsdData, &nDsdSize);
                } else {
                    pDsdData = pDstData;
                    nDsdSize = nDstSize;
                }

                if (nDsdSize > 0) {
                    int nRemoveSamples = 0;

                    if ((m_pDsdPcmConverter && !m_pDsdPcmConverter->is_convert_called())) {
                        nRemoveSamples = m_nPcmOutDelta;
                    }

                    dsd2pcm(pDsdData, static_cast<int>(nDsdSize), m_arrPcmBuf.data());

                    if (nRemoveSamples > 0) {
                        fixPcmStream(false, m_arrPcmBuf.data() + m_nPcmOutChannels * nRemoveSamples,
                                     m_nPcmOutSamples - nRemoveSamples);
                    }

                    writeData(pFile, nRemoveSamples, m_nPcmOutSamples - nRemoveSamples);

                    return false;
                }
            }
        } else {
            break;
        }
    }

    pDsdData = nullptr;
    pDstData = nullptr;
    nDstSize = 0;

    if (m_pDstDecoder) {
        m_pDstDecoder->decode(pDstData, nDstSize, &pDsdData, &nDsdSize);
    }

    if (nDsdSize > 0) {
        dsd2pcm(pDsdData, static_cast<int>(nDsdSize), m_arrPcmBuf.data());
        writeData(pFile, 0, m_nPcmOutSamples);

        return false;
    }

    if (m_nPcmOutDelta > 0) {
        dsd2pcm(nullptr, 0, m_arrPcmBuf.data());
        fixPcmStream(true, m_arrPcmBuf.data(), m_nPcmOutDelta);
        writeData(pFile, 0, m_nPcmOutDelta);
    }

    m_bTrackCompleted = true;

    return true;
}
