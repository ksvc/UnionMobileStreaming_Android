/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper */

#ifndef _Included_com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper
#define _Included_com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper
 * Method:    _init
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1init
  (JNIEnv *, jobject);

/*
 * Class:     com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper
 * Method:    _open
 * Signature: (JIIIII)I
 */
JNIEXPORT jint JNICALL Java_com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1open
  (JNIEnv *, jobject, jlong, jint, jint, jint, jint, jint);

/*
 * Class:     com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper
 * Method:    _encode
 * Signature: (JLjava/nio/ByteBuffer;IJI)I
 */
JNIEXPORT jint JNICALL Java_com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1encode
  (JNIEnv *, jobject, jlong, jobject, jint, jlong, jint);

/*
 * Class:     com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper
 * Method:    _close
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1close
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper
 * Method:    _release
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_union_media_streamer_encoder_fdkaac_FdkAACEncoderWrapper__1release
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif