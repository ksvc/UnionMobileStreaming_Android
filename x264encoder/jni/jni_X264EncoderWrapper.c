#include <malloc.h>
#include <inttypes.h>
#include <string.h>
#include "jni_X264EncoderWrapper.h"
#include "x264Encoder.h"
#include <UnionLog.h>

JavaVM* g_current_java_vm_ = NULL;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_current_java_vm_ = vm;
    return JNI_VERSION_1_4;
}

typedef struct Context {
    jobject obj;
    jmethodID jmid_on_encoded;
} Context;

static inline X264Encoder* getInstance(jlong ptr) {
    return (X264Encoder*)(intptr_t) ptr;
}

static void freeContext(void* opaque) {
    Context* ctx = (Context*) opaque;
    if (ctx->obj) {
        JNIEnv *env = NULL;
        (*g_current_java_vm_)->GetEnv(g_current_java_vm_, (void**)&env, JNI_VERSION_1_4);
        (*env)->DeleteGlobalRef(env, ctx->obj);
    }
    free(ctx);
}

static void onEncoded(UnionAVPacket* out, void* opaque) {
#if 0
    UnionLogD("video: %d pts=%"PRId64" dts=%"PRId64,
              out->size, out->pts, out->dts);
#endif

    Context* ctx = (Context*) opaque;
    JNIEnv *env = NULL;
    (*g_current_java_vm_)->GetEnv(g_current_java_vm_, (void**)&env,JNI_VERSION_1_4);

    jobject jbuffer = (*env)->NewDirectByteBuffer(env, out->data, out->size);
    (*env)->CallVoidMethod(env, ctx->obj, ctx->jmid_on_encoded, jbuffer,
                           out->dts, out->pts, out->flags);
}

jlong Java_org_opencdnunion_media_streamer_encoder_x264_X264EncoderWrapper__1init
        (JNIEnv *env, jobject obj) {
    X264Encoder* thiz = x264EncInit();
    if (thiz) {
        Context* ctx = calloc(1, sizeof(Context));
        if (ctx == NULL) {
            return 0;
        }
        jclass clazz = (*env)->GetObjectClass(env, obj);
        ctx->obj = (*env)->NewGlobalRef(env, obj);
        ctx->jmid_on_encoded = (*env)->GetMethodID(env, clazz, "onEncoded",
                                                   "(Ljava/nio/ByteBuffer;JJI)V");
        x264EncSetCallback(thiz, onEncoded, ctx, freeContext);
    }
    return (jlong)(intptr_t) thiz;
}

jint Java_org_opencdnunion_media_streamer_encoder_x264_X264EncoderWrapper__1open
        (JNIEnv *env, jobject obj, jlong ptr, jint bitrate, jint pix_fmt, jint width, jint height,
         jfloat fps, jfloat iframe_interval, jint profile) {
    X264Encoder* thiz = getInstance(ptr);
    UnionVideoEncCfg config;
    memset(&config, 0, sizeof(config));
    config.codecId = UNION_CODEC_ID_H264;
    config.bitrate = bitrate;
    config.pixFmt = (UnionPixFmt) pix_fmt;
    config.width = width;
    config.height = height;
    config.frameRate = fps;
    config.iFrameInterval = iframe_interval;
    config.profile = (UnionCodecProfile) profile;
    return x264EncOpen(thiz, &config);
}

jint Java_org_opencdnunion_media_streamer_encoder_x264_X264EncoderWrapper__1adjust_1bitrate
        (JNIEnv *env, jobject obj, jlong ptr, jint bitrate) {
    X264Encoder* thiz = getInstance(ptr);
    return x264EncAdjustBitrate(thiz, bitrate);
}

jint Java_org_opencdnunion_media_streamer_encoder_x264_X264EncoderWrapper__1encode
        (JNIEnv *env, jobject obj, jlong ptr, jobject jbuffer,
         jint width, jint height, jlong pts, jint flags) {
    X264Encoder* thiz = getInstance(ptr);
    uint8_t* data = NULL;
    if (jbuffer) {
        data = (uint8_t*)(*env)->GetDirectBufferAddress(env, jbuffer);
    }

    UnionAVFrame frame;
    memset(&frame, 0, sizeof(frame));
    if (data) {
        frame.planeNum = 3;
        frame.plane[0] = data;
        frame.plane[1] = frame.plane[0] + width * height;
        frame.plane[2] = frame.plane[1] + width * height / 4;
        frame.stride[0] = width;
        frame.stride[1] = width / 2;
        frame.stride[2] = width / 2;
    }
    frame.pts = pts;
    frame.flags = flags;
    return x264EncEncode(thiz, &frame);
}

void Java_org_opencdnunion_media_streamer_encoder_x264_X264EncoderWrapper__1close
        (JNIEnv *env, jobject obj, jlong ptr) {
    X264Encoder* thiz = getInstance(ptr);
    x264EncClose(thiz);
}

void Java_org_opencdnunion_media_streamer_encoder_x264_X264EncoderWrapper__1release
        (JNIEnv *env, jobject obj, jlong ptr) {
    X264Encoder* thiz = getInstance(ptr);
    x264EncRelease(thiz);
}
