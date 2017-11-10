#include "x264Encoder.h"
#include <x264.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <UnionLog.h>

#undef LOG_TAG
#define LOG_TAG "X264Encoder"

struct X264Encoder {
    x264_t* x264;
    x264_param_t* x264Param;
    x264_picture_t x264PicIn;
    x264_picture_t x264PicOut;
    UnionVideoEncCfg cfg;
    UnionAVEncCallback cb;
    void* opaque;
    UnionAVEncOpaqueFree opaqueFree;

    // reused packet
    UnionAVPacket packet;
};

X264Encoder* x264EncInit() {
    X264Encoder* thiz = (X264Encoder*)calloc(1, sizeof(X264Encoder));
    return thiz;
}

void x264EncSetCallback(X264Encoder* thiz, UnionAVEncCallback cb,
                        void* opaque, UnionAVEncOpaqueFree opaqueFree) {
    assert(thiz);
    thiz->cb = cb;
    thiz->opaque = opaque;
    thiz->opaqueFree = opaqueFree;
}

static const char* getProfileName(int profile) {
    switch (profile) {
        case UNION_CODEC_PROFILE_H264_BASELINE:
            return "baseline";
        case UNION_CODEC_PROFILE_H264_MAIN:
            return "main";
        case UNION_CODEC_PROFILE_H264_HIGH:
        default:
            return "high";
    }
}

static int getCsp(int pixFmt) {
    switch (pixFmt) {
        case UNION_PIX_FMT_I420:
            return X264_CSP_I420;
        case UNION_PIX_FMT_YV12:
            return X264_CSP_YV12;
        case UNION_PIX_FMT_NV12:
            return X264_CSP_NV12;
        case UNION_PIX_FMT_NV21:
            return X264_CSP_NV21;
        default:
            return X264_CSP_NONE;
    }
}

int x264EncOpen(X264Encoder* thiz, UnionVideoEncCfg* cfg) {
    assert(thiz && cfg);
    if (getCsp(cfg->pixFmt) == X264_CSP_NONE) {
        return UNION_ENC_ERR_UNSUPPORTED;
    }

    int ret = 0;
    memcpy(&thiz->cfg, cfg, sizeof(thiz->cfg));
    thiz->x264Param = malloc(sizeof(x264_param_t));
    assert(thiz->x264Param);
    x264_param_t* x264Param = thiz->x264Param;
    x264_param_default_preset(x264Param, "superfast", NULL);

    x264Param->i_width = cfg->width;
    x264Param->i_height = cfg->height;
    x264Param->rc.i_bitrate = cfg->bitrate / 1000;
    x264Param->rc.i_vbv_max_bitrate = cfg->bitrate / 1000;
    x264Param->rc.i_vbv_buffer_size = cfg->bitrate / 1000;
    x264Param->i_fps_den = 1;
    x264Param->i_fps_num = (uint32_t) cfg->frameRate;
    x264Param->i_timebase_den = 1000;
    x264Param->i_timebase_num = 1;
    x264Param->b_repeat_headers = 0;
    //x264Param->b_annexb = 0;

    float iFrameInterval = cfg->iFrameInterval;
    iFrameInterval = iFrameInterval ? iFrameInterval : 3.0f;
    iFrameInterval = (iFrameInterval < 1.0f) ? 1.0f : iFrameInterval;
    x264Param->i_keyint_max = (int) (iFrameInterval * cfg->frameRate);

    if (x264_param_apply_profile(x264Param, getProfileName(cfg->profile)) < 0) {
        UnionLogE("apply profile failed!");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }

    thiz->x264 = x264_encoder_open(x264Param);
    if (thiz->x264 == NULL) {
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }

    x264_nal_t* pNal;
    int nNal;
    int size = x264_encoder_headers(thiz->x264, &pNal, &nNal);
    if (size <= 0) {
        UnionLogE("get headers failed!");
        ret = UNION_ENC_ERR_UNSUPPORTED;
        goto Error;
    }
    UnionAVPacket* pkt = &thiz->packet;
    memset(pkt, 0, sizeof(thiz->packet));
    pkt->type = UNION_MEDIA_TYPE_VIDEO;
    pkt->data = pNal[0].p_payload;
    pkt->size = size;
    pkt->flags |= UNION_AV_FLAG_CODEC_CONFIG;
    if (thiz->cb) {
        thiz->cb(pkt, thiz->opaque);
    }
    return 0;

  Error:
    x264EncClose(thiz);
    return ret;
}

// adjust bitrate dynamically, return previous bitrate.
int x264EncAdjustBitrate(X264Encoder* thiz, int bitrate) {
    assert(thiz && thiz->x264 && thiz->x264Param);
    int preBitrate = thiz->x264Param->rc.i_bitrate * 1000;
    thiz->x264Param->rc.i_bitrate = bitrate / 1000;
    thiz->x264Param->rc.i_vbv_max_bitrate = bitrate / 1000;
    thiz->x264Param->rc.i_vbv_buffer_size = bitrate / 1000;
    x264_encoder_reconfig(thiz->x264, thiz->x264Param);
    return preBitrate;
}

int x264EncEncode(X264Encoder* thiz, const UnionAVFrame* in) {
    assert(thiz && thiz->x264);
    x264_picture_t* picIn = NULL;
    x264_picture_t* picOut = &thiz->x264PicOut;
    x264_picture_init(picOut);

    if (in) {
        picIn = &thiz->x264PicIn;
        x264_picture_init(picIn);
        picIn->img.i_csp = getCsp(thiz->cfg.pixFmt);
        picIn->img.i_plane = in->planeNum;
        picIn->i_pts = in->pts;
        for(int i = 0; i < in->planeNum; i++) {
            picIn->img.plane[i] = in->plane[i];
            picIn->img.i_stride[i] = in->stride[i];
        }
        if (in->flags & UNION_AV_FLAG_KEY_FRAME) {
            picIn->i_type = X264_TYPE_IDR;
        }
    }

    x264_nal_t* pNal;
    int nNal;
    do {
        int size = x264_encoder_encode(thiz->x264, &pNal, &nNal, picIn, picOut);
        if (size > 0) {
            UnionAVPacket *pkt = &thiz->packet;
            memset(pkt, 0, sizeof(thiz->packet));
            pkt->type = UNION_MEDIA_TYPE_VIDEO;
            pkt->data = pNal[0].p_payload;
            pkt->size = size;
            pkt->dts = picOut->i_dts;
            pkt->pts = picOut->i_pts;
            if (picOut->b_keyframe) {
                pkt->flags |= UNION_AV_FLAG_KEY_FRAME;
            }
            if (thiz->cb) {
                thiz->cb(pkt, thiz->opaque);
            }
        }
    } while (!picIn && x264_encoder_delayed_frames(thiz->x264));
    return 0;
}

void x264EncClose(X264Encoder* thiz) {
    if (thiz == NULL) {
        return;
    }
    if (thiz->x264) {
        x264_encoder_close(thiz->x264);
        thiz->x264 = NULL;
    }
    free(thiz->x264Param);
    thiz->x264Param = NULL;
}

void x264EncRelease(X264Encoder* thiz) {
    if (thiz == NULL) {
        return;
    }
    x264EncClose(thiz);
    if (thiz->opaque && thiz->opaqueFree) {
        thiz->opaqueFree(thiz->opaque);
    }
    free(thiz);
}
