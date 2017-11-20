#include <malloc.h>
#include <inttypes.h>
#include <string.h>
#include "jni_FdkAACEncoderWrapper.h"
#include "fdkAACEncoder.h"
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

extern JavaVM* g_current_java_vm_;

static inline FdkAACEncoder* getInstance(jlong ptr) {
    return (FdkAACEncoder*)(intptr_t) ptr;
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
    UnionLogD("audio: %d pts=%"PRId64" dts=%"PRId64,
            out->size, out->pts, out->dts);
#endif

    Context* ctx = (Context*) opaque;
    JNIEnv *env = NULL;
    (*g_current_java_vm_)->GetEnv(g_current_java_vm_, (void**)&env,JNI_VERSION_1_4);

    jobject jbuffer = (*env)->NewDirectByteBuffer(env, out->data, out->size);
    (*env)->CallVoidMethod(env, ctx->obj, ctx->jmid_on_encoded, jbuffer,
                           out->dts, out->pts, out->flags);
}

jlong Java_org_opencdnunion_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1init
        (JNIEnv *env, jobject obj) {
    FdkAACEncoder* thiz = fdkAACEncInit();
    if (thiz) {
        Context* ctx = calloc(1, sizeof(Context));
        if (ctx == NULL) {
            return 0;
        }
        jclass clazz = (*env)->GetObjectClass(env, obj);
        ctx->obj = (*env)->NewGlobalRef(env, obj);
        ctx->jmid_on_encoded = (*env)->GetMethodID(env, clazz, "onEncoded",
                                                   "(Ljava/nio/ByteBuffer;JJI)V");
        fdkAACEncSetCallback(thiz, onEncoded, ctx, freeContext);
    }
    return (jlong)(intptr_t) thiz;
}

jint Java_org_opencdnunion_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1open
        (JNIEnv *env, jobject obj, jlong ptr, jint bitrate, jint sampleFmt, jint sampleRate,
         jint channels, jint profile) {
    FdkAACEncoder* thiz = getInstance(ptr);
    UnionAudioEncCfg cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.codecId = UNION_CODEC_ID_AAC;
    cfg.bitrate = bitrate;
    cfg.sampleFmt = (UnionSampleFmt) sampleFmt;
    cfg.sampleRate = sampleRate;
    cfg.channels = channels;
    cfg.profile = (UnionCodecProfile) profile;
    return fdkAACEncOpen(thiz, &cfg);
}

jint Java_org_opencdnunion_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1encode
        (JNIEnv *env, jobject obj, jlong ptr, jobject jbuffer, jint size, jlong pts, jint flags) {
    FdkAACEncoder* thiz = getInstance(ptr);
    uint8_t* data = NULL;
    if (jbuffer) {
        data = (uint8_t*)(*env)->GetDirectBufferAddress(env, jbuffer);
    }

    UnionAVFrame* pf = NULL;
    UnionAVFrame frame;
    if (data) {
        memset(&frame, 0, sizeof(frame));
        frame.planeNum = 1;
        frame.plane[0] = data;
        frame.nbSamples = size / 2;
        frame.pts = pts;
        frame.flags = flags;
        pf = &frame;
    }
    return fdkAACEncEncode(thiz, pf);
}

void Java_org_opencdnunion_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1close
        (JNIEnv *env, jobject obj, jlong ptr) {
    FdkAACEncoder* thiz = getInstance(ptr);
    fdkAACEncClose(thiz);
}

void Java_org_opencdnunion_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1release
        (JNIEnv *env, jobject obj, jlong ptr) {
    FdkAACEncoder* thiz = getInstance(ptr);
    fdkAACEncRelease(thiz);
}
