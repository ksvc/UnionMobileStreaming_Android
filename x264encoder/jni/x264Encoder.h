#ifndef __X264_ENCODER_H__
#define __X264_ENCODER_H__

#include "UnionEncoderDef.h"

typedef struct X264Encoder X264Encoder;

/**
 * 创建x264编码器实例
 *
 * @return NULL或编码器实例
 */
X264Encoder* x264EncInit();

/**
 * 设置编码后的数据回调接口。
 *
 * @param thiz 编码器实例
 * @param cb 回调地址
 * @param opaque 透传结构体
 * @param opaqueFree 透传结构体的释放函数
 */
void x264EncSetCallback(X264Encoder* thiz, UnionAVEncCallback cb,
                        void* opaque, UnionAVEncOpaqueFree opaqueFree);
/**
 * 开启编码器
 *
 * @param thiz 编码器实例
 * @param cfg 视频编码参数
 * @return 成功返回0，否则返回错误码
 */
int x264EncOpen(X264Encoder* thiz, UnionVideoEncCfg* cfg);

/**
 * 编码过程中调整码率
 *
 * @param thiz      编码器实例指针
 * @param bitrate   码率调整目标值
 * @return          当前设置的码率
 */
int x264EncAdjustBitrate(X264Encoder* thiz, int bitrate);

/**
 * 编码一帧视频，如果有已编码的Packet，则会通过设置的回调输出。
 *
 * @param thiz  编码器实例指针
 * @param in    待编码的视频帧，为NULL时表示flush当前编码器
 * @return      成功返回0，失败时返回小于0的错误码
 */
int x264EncEncode(X264Encoder* thiz, const UnionAVFrame* in);

/**
 * 关闭编码器
 *
 * @param thiz 编码器实例
 */
void x264EncClose(X264Encoder* thiz);

/**
 * 释放当前编码器
 *
 * @param thiz 编码器实例
 */
void x264EncRelease(X264Encoder* thiz);

#endif // __X264_ENCODER_H__
