#ifndef __UNIONLOG_H__
#define __UNIONLOG_H__

#define LOG_TAG "UnionStreamer"

#if ANDROID
#include <android/log.h>
#define UnionLogV(fmt, args...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt, ##args)
#define UnionLogD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define UnionLogI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define UnionLogW(fmt, args...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ##args)
#define UnionLogE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)
#else
#define UnionLogV printf
#define UnionLogD printf
#define UnionLogI printf
#define UnionLogW printf
#define UnionLogE printf
#endif

#endif //__UNIONLOG_H__
