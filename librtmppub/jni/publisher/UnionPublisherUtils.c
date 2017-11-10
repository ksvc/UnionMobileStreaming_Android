//
//  UnionPublisherUtils.c
//  UnionPublisher
//
//  Created by shixuemei on 10/30/17.
//  Copyright © 2017 ksyun. All rights reserved.

#include "UnionPublisherUtils.h"

/**
 @abstract 采样率索引
 */
const int unionstreamer_audio_samplerates[16] = {
    96000, 88200, 64000, 48000, 44100, 32000,
   24000, 22050, 16000, 12000, 11025, 8000, 7350
};

/**
 @abstract 获取采样率索引
 */
uint8_t unionutils_get_samplerateindex(int samplerate)
{
    int samplerate_index = 0;
    for(samplerate_index = 0; samplerate_index < (sizeof(unionstreamer_audio_samplerates) / sizeof(int)); samplerate_index++)
    {
        if(unionstreamer_audio_samplerates[samplerate_index] == samplerate)
            break;
    }
    return samplerate_index;
}

static const uint8_t *union_avc_find_startcode_internal(const uint8_t *p, const uint8_t *end)
{
    const uint8_t *a = p + 4 - ((intptr_t)p & 3);
    
    for (end -= 3; p < a && p < end; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }
    
    for (end -= 3; p < end; p += 4) {
        uint32_t x = *(const uint32_t*)p;
        if ((x - 0x01010101) & (~x) & 0x80808080) { // generic
            if (p[1] == 0) {
                if (p[0] == 0 && p[2] == 1)
                    return p;
                if (p[2] == 0 && p[3] == 1)
                    return p+1;
            }
            if (p[3] == 0) {
                if (p[2] == 0 && p[4] == 1)
                    return p+2;
                if (p[4] == 0 && p[5] == 1)
                    return p+3;
            }
        }
    }
    
    for (end += 3; p < end; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }
    
    return end + 3;
}

/**
 @abstract 查找nal start code
 
 @param p 起始地址
 @param end 结束地址
 
 @return nal unit的起始地址(带start code)
 */
const uint8_t *unionutils_avc_find_startcode(const uint8_t *p, const uint8_t *end)
{
    const uint8_t *out= union_avc_find_startcode_internal(p, end);
    if(p<out && out<end && !out[-1]) out--;
    return out;
}
