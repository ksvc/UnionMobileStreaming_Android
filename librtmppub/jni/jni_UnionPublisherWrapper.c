#include <stdlib.h>
#include "jni_UnionPublisherWrapper.h"
#include "publisher/UnionLibrtmp.h"
#include "UnionAVCommon.h"

typedef struct Context {
    jobject obj;
    jmethodID jmid_on_event;
} Context;

extern JavaVM* g_current_java_vm_;

jobject videoExtra;
jobject audioExtra;

static inline UnionLibrtmp_t* getInstance(jlong ptr)
{
    return (UnionLibrtmp_t*)(intptr_t) ptr;
}

static void freeContext(void* opaque) {
    Context* ctx = (Context*) opaque;
    if (ctx && ctx->obj) {
        JNIEnv *env = NULL;
        (*g_current_java_vm_)->GetEnv(g_current_java_vm_, (void**)&env, JNI_VERSION_1_4);
        (*env)->DeleteGlobalRef(env, ctx->obj);
    }
    free(ctx);
}

JNIEXPORT jlong JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1init
        (JNIEnv *env, jobject instance) {
    UnionLibrtmp_t* thiz = union_librtmp_open();
    return (jlong)(intptr_t) thiz;
}

JNIEXPORT void JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1setAudioEncCfg
        (JNIEnv *env, jobject instance, jlong ptr, jint codecId, jint profile, jint sampleFmt,
         jint sampleRate, jint channels, jint bitrate) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    UnionAudioEncCfg *aEncCfg = union_librtmp_get_audiocfg(thiz);
    if(aEncCfg) {
        aEncCfg->codecId = codecId;
        aEncCfg->profile = profile;
        aEncCfg->sampleFmt = sampleFmt;
        aEncCfg->sampleRate = sampleRate;
        aEncCfg->channels = channels;
        aEncCfg->bitrate = bitrate;
    }
}

JNIEXPORT void JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1setVideoEncCfg
        (JNIEnv *env, jobject instance, jlong ptr, jint codecId, jint profile, jint pixFmt,
         jint width, jint height, jint bitrate, jfloat frameRate, jfloat iFrameInterval) {

    // TODO
    UnionLibrtmp_t* thiz = getInstance(ptr);
    UnionVideoEncCfg *vEncCfg = union_librtmp_get_videocfg(thiz);
    if(vEncCfg) {
        vEncCfg->codecId = codecId;
        vEncCfg->profile = profile;
        vEncCfg->profile = pixFmt;
        vEncCfg->width = width;
        vEncCfg->height = height;
        vEncCfg->bitrate = bitrate;
        vEncCfg->frameRate = frameRate;
        vEncCfg->iFrameInterval = iFrameInterval;
    }
}

JNIEXPORT jint JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1send_1packet
        (JNIEnv *env, jobject instance, jlong ptr, jint type, jobject data,
         jint size, jlong dts, jlong pts, jint flags) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    uint8_t* buf = (uint8_t*)(*env)->GetDirectBufferAddress(env, data);
    UnionAVPacket packet;
    packet.type = type;
    packet.data = buf;
    packet.size = size;
    packet.dts = dts;
    packet.pts = pts;
    packet.flags = flags;
    return union_librtmp_send(thiz, &packet);
}

JNIEXPORT jint JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1start
        (JNIEnv *env, jobject instance, jlong ptr, jstring uri_) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    const char *uri = (*env)->GetStringUTFChars(env, uri_, 0);
    int ret = 0;
    ret = union_librtmp_start(thiz, uri, NULL);
    (*env)->ReleaseStringUTFChars(env, uri_, uri);
    return ret;
}

JNIEXPORT jint JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1stop
        (JNIEnv *env, jobject instance, jlong ptr) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    union_librtmp_stop(thiz);
}

JNIEXPORT void JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1release
        (JNIEnv *env, jobject instance, jlong ptr) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    union_librtmp_close(thiz);
}
