#include <stdlib.h>
#include <publisher/UnionPublisherDef.h>
#include <UnionAVCommon.h>
#include "jni_UnionPublisherWrapper.h"
#include "publisher/UnionLibrtmp.h"
#include "UnionAVCommon.h"
#include "UnionLog.h"
#include "publisher/UnionPublisherDef.h"

static inline UnionLibrtmp_t* getInstance(jlong ptr)
{
    return (UnionLibrtmp_t*)(intptr_t) ptr;
}

JNIEXPORT jlong JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1init
        (JNIEnv *env, jobject instance) {
    UnionLibrtmp_t* thiz = union_librtmp_open();
    return (jlong)(intptr_t) thiz;
}

JNIEXPORT void JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1setAudioEncCfg
        (JNIEnv *env, jobject instance, jlong ptr, jint codecId, jint profile, jint sampleFmt,
         jint sampleRate, jint channels, jint bitrate) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    UnionAudioEncCfg aEncCfg = {0};
    aEncCfg.codecId = (UnionCodecID)codecId;
    aEncCfg.profile = (UnionCodecProfile)profile;
    aEncCfg.sampleFmt = (UnionSampleFmt)sampleFmt;
    aEncCfg.sampleRate = sampleRate;
    aEncCfg.channels = channels;
    aEncCfg.bitrate = bitrate;
    union_librtmp_set_audiocfg(thiz, &aEncCfg);
}

JNIEXPORT void JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1setVideoEncCfg
        (JNIEnv *env, jobject instance, jlong ptr, jint codecId, jint profile, jint pixFmt,
         jint width, jint height, jint bitrate, jfloat frameRate, jfloat iFrameInterval) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    UnionVideoEncCfg vEncCfg = {0};
    vEncCfg.codecId = (UnionCodecID)codecId;
    vEncCfg.profile = (UnionCodecProfile)profile;
    vEncCfg.pixFmt = (UnionPixFmt)pixFmt;
    vEncCfg.width = width;
    vEncCfg.height = height;
    vEncCfg.bitrate = bitrate;
    vEncCfg.frameRate = frameRate;
    vEncCfg.iFrameInterval = iFrameInterval;

    union_librtmp_set_videocfg(thiz, &vEncCfg);
}

JNIEXPORT jint JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1send_1packet
        (JNIEnv *env, jobject instance, jlong ptr, jint type, jobject data,
         jint size, jlong dts, jlong pts, jint flags) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    uint8_t* buf = (uint8_t*)(*env)->GetDirectBufferAddress(env, data);
    UnionAVPacket packet;
    packet.type = (UnionMediaType)type;
    packet.data = buf;
    packet.size = size;
    packet.dts = dts;
    packet.pts = pts;
    packet.flags = flags;
    return union_librtmp_send(thiz, &packet);
}

JNIEXPORT jint JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1start
        (JNIEnv *env, jobject instance, jlong ptr, jstring uri_) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    const char *uri = (*env)->GetStringUTFChars(env, uri_, 0);
    int ret = 0;
    ret = union_librtmp_start(thiz, uri, NULL);
    (*env)->ReleaseStringUTFChars(env, uri_, uri);
    return ret;
}

JNIEXPORT jint JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1stop
        (JNIEnv *env, jobject instance, jlong ptr) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    union_librtmp_stop(thiz);
}

JNIEXPORT void JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1release
        (JNIEnv *env, jobject instance, jlong ptr) {
    UnionLibrtmp_t* thiz = getInstance(ptr);
    union_librtmp_close(thiz);
}


JNIEXPORT void JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1set_1meta_1option_1number(
        JNIEnv *env, jobject instance, jlong ptr, jstring key_, jdouble value) {
    UnionLibrtmp_t* thiz = getInstance(ptr);

    const char *key = (*env)->GetStringUTFChars(env, key_, 0);

    union_librtmp_set_userMetadata(thiz, key, value, NULL);
    (*env)->ReleaseStringUTFChars(env, key_, key);
}

JNIEXPORT void JNICALL
Java_org_opencdnunion_streamer_publisher_librtmp_UnionPublisherWrapper__1set_1meta_1option_1string(
        JNIEnv *env, jobject instance, jlong ptr, jstring key_, jstring value_) {
    UnionLibrtmp_t* thiz = getInstance(ptr);

    const char *key = (*env)->GetStringUTFChars(env, key_, 0);
    char *value = (char*)(*env)->GetStringUTFChars(env, value_, 0);

    union_librtmp_set_userMetadata(thiz, key, 0, value);

    (*env)->ReleaseStringUTFChars(env, key_, key);
    (*env)->ReleaseStringUTFChars(env, value_, value);
}