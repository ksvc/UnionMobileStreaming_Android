//  UnionFLV.h
//  UnionPublisher
//
//  Created by shixuemei on 10/28/17.
//  Copyright © 2017 ksyun. All rights reserved.
//
#include <stdio.h>
#include <stdbool.h>
#include "UnionPublisherDef.h"
#include "UnionAVCommon.h"

/**
 定义FLV格式的封装
 */

#define UNIONFLV_SOUNDSIZE_OFFSET       1
#define UNIONFLV_SOUNDRATE_OFFSET       2
#define UNIONFLV_SOUNDFORMAT_OFFSET     4

#define UNIONFLV_FRAMETYPE_OFFSET       4

/**
 * SoundFormat
 */
enum {
    UNIONFLV_SOUNDFORMAT_PCM                    = 0,
    UNIONFLV_SOUNDFORMAT_ADPCM                  = 1 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_MP3                    = 2 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_PCM_LE                 = 3 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_NELLYMOSER_16KHZ_MONO  = 4 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_NELLYMOSER_8KHZ_MONO   = 5 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_NELLYMOSER             = 6 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_PCM_ALAW               = 7 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_PCM_MULAW              = 8 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_AAC                    = 10 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_SPEEX                  = 11 << UNIONFLV_SOUNDFORMAT_OFFSET,
    UNIONFLV_SOUNDFORMAT_MP3_8KHZ               = 14 << UNIONFLV_SOUNDFORMAT_OFFSET,
};

/**
 * SoundRate
 */
enum {
    UNIONFLV_SOUNDRATE_5512HZ   = 0,
    UNIONFLV_SOUNDRATE_11025HZ  = 1 << UNIONFLV_SOUNDRATE_OFFSET,
    UNIONFLV_SOUNDRATE_22050HZ  = 2 << UNIONFLV_SOUNDRATE_OFFSET,
    UNIONFLV_SOUNDRATE_44100HZ  = 3 << UNIONFLV_SOUNDRATE_OFFSET,
};

/**
 * SoundSize
 */
enum {
    UNIONFLV_SOUNDSIZE_8BIT   = 0,
    UNIONFLV_SOUNDSIZE_16BIT  = 1 << UNIONFLV_SOUNDSIZE_OFFSET,
};

/**
 * SoundType
 */
enum {
    UNIONFLV_SOUNDTYPE_MONO     = 0,
    UNIONFLV_SOUNDTYPE_STEREO   = 1,
};

/**
 * VideoFrameType
 */
enum {
    UNIONFLV_FRAMETYPE_KEY      = 1 << UNIONFLV_FRAMETYPE_OFFSET,
    UNIONFLV_FRAMETYPE_INTER    = 2 << UNIONFLV_FRAMETYPE_OFFSET,
};

/**
 * VideoCodecId
 */
enum {
    UNIONFLV_VIDEOCODECID_JPEG          = 1,
    UNIONFLV_VIDEOCODECID_H263          = 2,
    UNIONFLV_VIDEOCODECID_SCREENVIDEO   = 3,
    UNIONFLV_VIDEOCODECID_VP6           = 4,
    UNIONFLV_VIDEOCODECID_VP6_ALPHACHN  = 5,
    UNIONFLV_VIDEOCODECID_SCREENVIDEO2  = 6,
    UNIONFLV_VIDEOCODECID_AVC           = 7,
};

/**
 @abstract 获取flv audio tag的第一个字节
 
 @param audiofmt 音频格式
 
 @return flv audio tag的第一个字节
 */
uint8_t unionflv_get_audio_flags(UnionAudioEncCfg *audioEncCfg);

/**
 @abstract 获取flv aac AudioSpecificConfig
 
 @param audiofmt 音频格式
 
 @return AudioSpecificConfig
 */
uint16_t unionflv_get_aac_speccfg(UnionAudioEncCfg *audioEncCfg);

/**
 @abstract 获取flv video tag的第一个字节
 
 @param audiofmt 视频格式
 @param keyFrame 是否为关键帧
 
 @return flv video tag的第一个字节
 */
uint8_t unionflv_get_video_flags(UnionVideoEncCfg *videofmt, bool bKeyFrame);
