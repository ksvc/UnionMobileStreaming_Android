#ifndef __UNION_AVCONST_H__
#define __UNION_AVCONST_H__

#include <stdint.h>

/**
 编码器ID(压缩格式)
 */
typedef enum {
    UNION_CODEC_ID_NONE = 0,
    // video codec
    UNION_CODEC_ID_H264 = 1,
    UNION_CODEC_ID_H265,
    // audio codec
    UNION_CODEC_ID_AAC = 0x100,
} UnionCodecID;

/**
 像素格式
 */
typedef enum {
    UNION_PIX_FMT_NONE = 0,
    UNION_PIX_FMT_NV12,
    UNION_PIX_FMT_NV21,
    UNION_PIX_FMT_YV12,
    UNION_PIX_FMT_I420,
} UnionPixFmt;

/**
 音频格式
 */
typedef enum {
        ///< unsigned 8 bits
    UNION_SAMPLE_FMT_U8,  
    ///< signed 16 bits
    UNION_SAMPLE_FMT_S16, 
    ///< signed 32 bits
    UNION_SAMPLE_FMT_S32, 
    ///< float
    UNION_SAMPLE_FMT_FLT, 
    ///< double
    UNION_SAMPLE_FMT_DBL, 

    ///< unsigned 8 bits, planar
    UNION_SAMPLE_FMT_U8P, 
    ///< signed 16 bits, planar
    UNION_SAMPLE_FMT_S16P,
    ///< signed 32 bits, planar
    UNION_SAMPLE_FMT_S32P,
    ///< float, planar
    UNION_SAMPLE_FMT_FLTP,
    ///< double, planar
    UNION_SAMPLE_FMT_DBLP,
} UnionSampleFmt;

// encoder profile
typedef enum {
    /// 未知
    UNION_CODEC_PROFILE_NONE = 0,
    UNION_CODEC_PROFILE_H264_BASELINE = 0x100,
    UNION_CODEC_PROFILE_H264_MAIN,
    UNION_CODEC_PROFILE_H264_HIGH,
    UNION_CODEC_PROFILE_H265_MAIN = 0x200,
    UNION_CODEC_PROFILE_H265_MAIN10,
    UNION_CODEC_PROFILE_AAC_LOW = 0x300,
    UNION_CODEC_PROFILE_AAC_HE,
    UNION_CODEC_PROFILE_AAC_HE_V2,
} UnionCodecProfile;

/**
 音视频帧的辅助信息
 */
enum {
    /// 当前packet中为关键帧
    UNION_AV_FLAG_KEY_FRAME = 1<<0,
    /// 当前packet中为sps,pps
    UNION_AV_FLAG_CODEC_CONFIG = 1<<1,
    /// 当前packet中最后一个包
    UNION_AV_FLAG_END_OF_STREAM = 1<<2,
    /// 当前packet中为P帧
    UNION_AV_FLAG_P_FRAME = 1<<3,
    /// 当前packet中为B帧
    UNION_AV_FLAG_B_FRAME = 1<<4,
};

/// 媒体类型
typedef enum  {
    /// unknown data
    UNION_MEDIA_TYPE_NONE = 0,
    /// video data
    UNION_MEDIA_TYPE_VIDEO = 1,
    /// audio data
    UNION_MEDIA_TYPE_AUDIO  = 2,
} UnionMediaType;

/**
 * 视频编码参数.
 */
typedef struct  {
    UnionCodecID codecId;           // CODEC_ID_XXX
    UnionCodecProfile profile;      // PROFILE_H264_XXX
    UnionPixFmt pixFmt;             // PIX_FMT_XXX
    int width;  /// 宽度
    int height; /// 高度
    int bitrate;   /// video bitrate in bps
    float frameRate; /// 帧率
    float iFrameInterval; /// 关键帧间隔
} UnionVideoEncCfg;

/**
 * 音频编码参数.
 */
typedef struct  {
    UnionCodecID codecId;    // CODEC_ID_XXX
    UnionCodecProfile profile;  // PROFILE_AAC_XXX
    UnionSampleFmt sampleFmt; // sample 格式
    int sampleRate; // 采样率
    int channels;   // 通道数
    int bitrate;    // 编码目标码率
} UnionAudioEncCfg;

/**
 * 音频编码参数.
 */
typedef union {
    UnionAudioEncCfg a; // 音频编码参数
    UnionVideoEncCfg v; // 视频编码参数
} UnionEncoderCfg;

/**
 * 编码器输入数据, 音视频共用
 */
typedef struct {
    /// plane个数
    int planeNum;
    /// 数据指针
    uint8_t * plane[4];
    /// 时间戳, 单位为ms
    int64_t pts;
    /// 辅助信息, 参见: UNION_AV_FLAG_XXX
    int flags;
    
    /// for图像,每行的宽度(可能大于width)
    int stride[4];
    /// for音频, sample数
    int nbSamples;
} UnionAVFrame;

typedef struct {
    /// 数据指针
    uint8_t * data;
    /// 数据的长度
    int size;
    /// 时间戳, 单位为ms
    int64_t pts;
    /// 解码时间戳, 单位为ms
    int64_t dts;
    /// 数据类别
    UnionMediaType type;
    /// 数据flags, 参见: UNION_AV_FLAG_XXX
    int flags;
} UnionAVPacket;

#endif  // __UNION_AVCONST_H__
