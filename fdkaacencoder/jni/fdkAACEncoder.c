#include "fdkAACEncoder.h"
#include <fdk-aac/aacenc_lib.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <UnionLog.h>

#undef LOG_TAG
#define LOG_TAG "FdkAACEncoder"

struct FdkAACEncoder {
    HANDLE_AACENCODER handler;
    AACENC_InfoStruct aacencInfo;
    UnionAudioEncCfg cfg;
    UnionAVEncCallback cb;
    void* opaque;
    UnionAVEncOpaqueFree opaqueFree;

    uint8_t* inBuf;
    int inBufSize;
    int inBufLimit;
    uint8_t* outBuf;
    int outBufSize;
    int64_t ptsDelay;
    int64_t nextPts;
    UnionAVPacket packet;
};

FdkAACEncoder* fdkAACEncInit() {
    FdkAACEncoder* thiz = calloc(1, sizeof(FdkAACEncoder));
    return thiz;
}

void fdkAACEncSetCallback(FdkAACEncoder* thiz, UnionAVEncCallback cb,
                        void* opaque, UnionAVEncOpaqueFree opaqueFree) {
    assert(thiz);
    thiz->cb = cb;
    thiz->opaque = opaque;
    thiz->opaqueFree = opaqueFree;
}

const UINT getAot(int profile) {
    switch (profile) {
        case UNION_CODEC_PROFILE_AAC_HE:
            return 5;
        case UNION_CODEC_PROFILE_AAC_HE_V2:
            return 29;
        case UNION_CODEC_PROFILE_AAC_LOW:
        default:
            return 2;
    }
}

int fdkAACEncOpen(FdkAACEncoder* thiz, UnionAudioEncCfg* cfg) {
    assert(thiz && cfg);
    memcpy(&thiz->cfg, cfg, sizeof(thiz->cfg));

    int ret = UNION_ENC_ERR_UNKNOWN;
    if (cfg->sampleFmt != UNION_SAMPLE_FMT_S16) {
        UnionLogE("only UNION_SAMPLE_FMT_S16 supported!");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }

    if (aacEncOpen(&thiz->handler, 0, (const UINT) cfg->channels) != AACENC_OK) {
        UnionLogE("Unable to open encoder");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    if (aacEncoder_SetParam(thiz->handler, AACENC_AOT, getAot(cfg->profile)) != AACENC_OK) {
        UnionLogE("Unable to set the AOT");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    if (aacEncoder_SetParam(thiz->handler, AACENC_SAMPLERATE,
                            (const UINT) cfg->sampleRate) != AACENC_OK) {
        UnionLogE("Unable to set the AOT");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    if (aacEncoder_SetParam(thiz->handler, AACENC_CHANNELMODE,
                            (const UINT) cfg->channels) != AACENC_OK) {
        UnionLogE("Unable to set the channel mode");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    if (aacEncoder_SetParam(thiz->handler, AACENC_BITRATE,
                            (const UINT) cfg->bitrate) != AACENC_OK) {
        UnionLogE("Unable to set the bitrate");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    if (aacEncoder_SetParam(thiz->handler, AACENC_TRANSMUX, 0) != AACENC_OK) {
        UnionLogE("Unable to set the RAW transmux");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    if (aacEncoder_SetParam(thiz->handler, AACENC_SIGNALING_MODE,
                            (cfg->profile == UNION_CODEC_PROFILE_AAC_HE_V2) ? 0 : 2) != AACENC_OK) {
        UnionLogE("Unable to set the signaling mode");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    if (aacEncoder_SetParam(thiz->handler, AACENC_AFTERBURNER, 1) != AACENC_OK) {
        UnionLogE("Unable to set the afterburner mode");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    if (aacEncEncode(thiz->handler, NULL, NULL, NULL, NULL) != AACENC_OK) {
        UnionLogE("Unable to initialize the encoder");
        ret = UNION_ENC_ERR_UNKNOWN;
        goto Error;
    }
    if (aacEncInfo(thiz->handler, &thiz->aacencInfo) != AACENC_OK) {
        UnionLogE("Unable to get the encoder info");
        ret = UNION_ENC_ERR_UNKNOWN;
        goto Error;
    }

    // calculate pts delay
    thiz->ptsDelay = thiz->aacencInfo.encoderDelay * 1000 / cfg->sampleRate;

    // alloc input buffer
    thiz->inBufSize = cfg->channels * 2 * thiz->aacencInfo.frameLength;
    thiz->inBuf = (uint8_t*) malloc((size_t) thiz->inBufSize);
    thiz->inBufLimit = 0;
    if (thiz->inBuf == NULL) {
        thiz->inBufSize = 0;
        goto Error;
    }

    // alloc output buffer
    thiz->outBufSize = thiz->aacencInfo.maxOutBufBytes;
    UnionLogD("outBufSize=%d, delay=%d", thiz->outBufSize, thiz->aacencInfo.encoderDelay);
    thiz->outBuf = (uint8_t*) malloc((size_t) thiz->outBufSize);
    if (thiz->outBuf == NULL) {
        thiz->outBufSize = 0;
        goto Error;
    }

    UnionAVPacket* pkt = &thiz->packet;
    memset(pkt, 0, sizeof(thiz->packet));
    pkt->type = UNION_MEDIA_TYPE_AUDIO;
    pkt->data = thiz->aacencInfo.confBuf;
    pkt->size = thiz->aacencInfo.confSize;
    pkt->flags |= UNION_AV_FLAG_CODEC_CONFIG;
    if (thiz->cb) {
        thiz->cb(pkt, thiz->opaque);
    }

    thiz->nextPts = INT64_MIN;
    return 0;

  Error:
    fdkAACEncClose(thiz);
    return ret;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
static int encode(FdkAACEncoder* thiz, int bytesPerSample, bool flush) {
    AACENC_BufDesc inBuf = { 0 }, outBuf = { 0 };
    AACENC_InArgs inArgs = { 0 };
    AACENC_OutArgs outArgs = { 0 };
    int inIdentifier = IN_AUDIO_DATA;
    int inSize = thiz->inBufSize;
    int inElemSize = 2;
    int outIdentifier = OUT_BITSTREAM_DATA;
    int outSize = thiz->outBufSize;
    int outElemSize = 1;
    void *inPtr = thiz->inBuf;
    void *outPtr = thiz->outBuf;

    if (flush) {
        inArgs.numInSamples = -1;
    } else {
        inArgs.numInSamples = inSize / bytesPerSample;
        inBuf.numBufs = 1;
        inBuf.bufs = &inPtr;
        inBuf.bufferIdentifiers = &inIdentifier;
        inBuf.bufSizes = &inSize;
        inBuf.bufElSizes = &inElemSize;
    }
    outBuf.numBufs = 1;
    outBuf.bufs = &outPtr;
    outBuf.bufferIdentifiers = &outIdentifier;
    outBuf.bufSizes = &outSize;
    outBuf.bufElSizes = &outElemSize;

    int ret = aacEncEncode(thiz->handler, &inBuf, &outBuf, &inArgs, &outArgs);
    if (ret != AACENC_OK) {
        if (ret == AACENC_ENCODE_EOF) {
            UnionLogD("aac encode eof");
        } else {
            UnionLogE("Encoding failed!");
            return UNION_ENC_ERR_UNKNOWN;
        }
    }

    UnionAVPacket* pkt = &thiz->packet;
    memset(pkt, 0, sizeof(thiz->packet));
    pkt->type = UNION_MEDIA_TYPE_AUDIO;
    pkt->data = thiz->outBuf;
    pkt->size = outArgs.numOutBytes;
    pkt->pts = thiz->nextPts - thiz->ptsDelay;
    pkt->dts = pkt->pts;
    if (thiz->cb) {
        thiz->cb(pkt, thiz->opaque);
    }
    return 0;
}
#pragma clang diagnostic pop

int fdkAACEncEncode(FdkAACEncoder* thiz, const UnionAVFrame* in) {
    assert(thiz && thiz->handler);

    int ret = 0;
    int bytesPerSample = 2;
    int size = 0;
    uint8_t* data = NULL;
    if (in) {
        size = in->nbSamples * bytesPerSample;
        data = in->plane[0];
        thiz->nextPts = in->pts;
    }
    int offset = 0;
    do {
        if (data != NULL) {
            int remain = size - offset;
            int cap = thiz->inBufSize - thiz->inBufLimit;
            int len = (remain > cap) ? cap : remain;
            memcpy(thiz->inBuf + thiz->inBufLimit, data + offset, (size_t) len);
            offset += len;
            thiz->inBufLimit += len;
            if (thiz->inBufLimit == thiz->inBufSize) {
                if ((ret = encode(thiz, bytesPerSample, false)) < 0)  {
                    return ret;
                }
                thiz->inBufLimit = 0;
                int64_t off_sample = ((int64_t) offset) / bytesPerSample / thiz->cfg.channels;
                thiz->nextPts = in->pts + off_sample * 1000 / thiz->cfg.sampleRate;
            } else {
                continue;
            }
        } else {
            if ((ret = encode(thiz, bytesPerSample, false)) < 0)  {
                return ret;
            }
        }
    } while (offset < size);

    return 0;
}

void fdkAACEncClose(FdkAACEncoder* thiz) {
    if (thiz == NULL) {
        return;
    }
    if (thiz->handler) {
        aacEncClose(&thiz->handler);
    }
    if (thiz->inBuf != NULL) {
        free(thiz->inBuf);
        thiz->inBuf = NULL;
        thiz->inBufSize = 0;
    }
    if (thiz->outBuf != NULL) {
        free(thiz->outBuf);
        thiz->outBuf = NULL;
        thiz->outBufSize = 0;
    }
}

void fdkAACEncRelease(FdkAACEncoder* thiz) {
    if (thiz == NULL) {
        return;
    }
    fdkAACEncClose(thiz);
    if (thiz->opaque && thiz->opaqueFree) {
        thiz->opaqueFree(thiz->opaque);
    }
    free(thiz);
}