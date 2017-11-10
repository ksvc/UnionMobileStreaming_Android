/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_union_streamer_publisher_librtmp_UnionPublisherWrapper */

#define _Included_com_union_streamer_publisher_librtmp_UnionPublisherWrapper
#ifndef _Included_com_union_streamer_publisher_librtmp_UnionPublisherWrapper
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1init(JNIEnv *env,
                                                                       jobject instance);

JNIEXPORT void JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1setAudioEncCfg
        (JNIEnv *env, jobject instance, jlong ptr, jint codecId, jint profile, jint sampleFmt,
         jint sampleRate, jint channels, jint bitrate) ;

JNIEXPORT void JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1setVideoEncCfg
        (JNIEnv *env, jobject instance, jlong ptr, jint codecId, jint profile, jint pixFmt,
         jint width, jint height, jint bitrate, jfloat frameRate, jfloat iFrameInterval);

JNIEXPORT jint JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1send_1packet
        (JNIEnv *env, jobject instance, jlong ptr, jint type, jobject data,
         jint size, jlong dts, jlong pts, jint flags) ;

JNIEXPORT jint JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1start
        (JNIEnv *env, jobject instance, jlong ptr, jstring uri_);

JNIEXPORT jint JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1stop
        (JNIEnv *env, jobject instance, jlong ptr) ;

JNIEXPORT void JNICALL
Java_com_union_streamer_publisher_librtmp_UnionPublisherWrapper__1release
        (JNIEnv *env, jobject instance, jlong ptr) ;
#ifdef __cplusplus
}
#endif
#endif