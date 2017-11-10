#ifndef __FDKAACENCODER_H__
#define __FDKAACENCODER_H__

#include "UnionEncoderDef.h"

typedef struct FdkAACEncoder FdkAACEncoder;

/**
 * 初始化fdk-aac编码器
 *
 * @return NULL或编码器实例
 */
FdkAACEncoder* fdkAACEncInit();

/**
 * 设置编码后的数据回调接口。
 *
 * @param thiz 编码器实例
 * @param cb 回调地址
 * @param opaque 透传结构体
 * @param opaqueFree 透传结构体的释放函数
 */
void fdkAACEncSetCallback(FdkAACEncoder* thiz, UnionAVEncCallback cb,
                        void* opaque, UnionAVEncOpaqueFree opaqueFree);

/**
 * 开启编码器
 *
 * @param thiz 编码器实例
 * @param cfg 音频编码参数
 * @return 成功返回0，否则返回错误码
 */
int fdkAACEncOpen(FdkAACEncoder* thiz, UnionAudioEncCfg* cfg);

/**
 * 传送PCM音频数据给编码器
 *
 * @param thiz 编码器实例
 * @param in 音频PCM数据，为NULL时表示flush当前编码器
 * @return 成功返回0，否则返回错误码
 */
int fdkAACEncEncode(FdkAACEncoder* thiz, const UnionAVFrame* in);

/**
 * 关闭编码器
 *
 * @param thiz 编码器实例
 */
void fdkAACEncClose(FdkAACEncoder* thiz);

/**
 * 释放当前编码器
 *
 * @param thiz 编码器实例
 */
void fdkAACEncRelease(FdkAACEncoder* thiz);

#endif //__FDKAACENCODER_H__
