//
//  UnionFLV.c
//  UnionPublisher
//
//  Created by shixuemei on 10/30/17.
//  Copyright © 2017 ksyun. All rights reserved.
//
#include <stdio.h>
#include <stdbool.h>
#include "UnionFLV.h"
#include "UnionPublisherUtils.h"

/**
 @abstract 获取flv audio tag的第一个字节
 */
uint8_t unionflv_get_audio_flags(UnionAudioEncCfg *audioEncCfg)
{
    uint8_t flag = 0;
    if(UNION_CODEC_ID_AAC == audioEncCfg->codecId)
    {
        flag |= UNIONFLV_SOUNDFORMAT_AAC;
        flag |= UNIONFLV_SOUNDRATE_44100HZ;
        flag |= UNIONFLV_SOUNDSIZE_16BIT;
        flag |= UNIONFLV_SOUNDTYPE_STEREO;
    }
    
    return flag;
}

/**
 @abstract 获取flv video tag的第一个字节
 */
uint8_t unionflv_get_video_flags(UnionVideoEncCfg *videoEncCfg, bool bKeyFrame)
{
    uint8_t flag = 0;
    if(UNION_CODEC_ID_H264 == videoEncCfg->codecId)
    {
        uint8_t keyframe = bKeyFrame ? UNIONFLV_FRAMETYPE_KEY : UNIONFLV_FRAMETYPE_INTER;
        flag = keyframe | UNIONFLV_VIDEOCODECID_AVC;
    }
    
    return flag;
}
