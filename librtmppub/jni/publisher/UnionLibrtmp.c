//
//  UnionLibrtmp.c
//  Unionlibrtmp
//
//  Created by shixuemei on 10/30/17.
//  Copyright © 2017 ksyun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include "rtmp.h"
#include "UnionLog.h"
#include "UnionFLV.h"
#include "UnionLibrtmp.h"
#include "UnionPublisherUtils.h"

#define RTMP_HEAD_SIZE          (sizeof(RTMPPacket) + RTMP_MAX_HEADER_SIZE)
#define RTMP_AACSPECINFO_LEN    2
#define RTMP_VIDEOHEADER_LEN    5
#define RTMP_VIDEODATA_Padding  20         //video frame多分配一点空间，防止nal startcode只有3位时分配空间不足
#define RTMP_METADATA_MAXLEN    1024

#define RTMP_AUDIO_CHANNELID    0x04
#define RTMP_VIDEO_CHANNELID    0x06

/**
 * UnionLibrtmp对象
 */
typedef struct UnionLibrtmp {
    RTMP                    *rtmpHandle;

    bool                    bInitAudio;
    bool                    bInitVideo;
    bool                    bSendMeta;
    
    int64_t                 videoBaseTime;
    int64_t                 audioBaseTime;
    
    int64_t                 lastVideoTime;
    int64_t                 lastAudioTime;
    
    UnionPublisherStatus    status;
    
    UnionVideoEncCfg        videoEncCfg;
    UnionAudioEncCfg        audioEncCfg;
    
    pthread_mutex_t         mutex;
    
    UnionDict                   userMetadata;
}UnionLibrtmp_t;

/**
 * @abstract 转换时间戳
 */
static uint32_t union_librtmp_get_relativeTime(UnionLibrtmp_t *librtmp, int64_t timestamp, UnionMediaType type)
{
    uint32_t ret = 0;
    
    if(NULL == librtmp)
        return 0;

    if(UNION_MEDIA_TYPE_AUDIO == type)
    {
        if(librtmp->audioBaseTime < 0)
        {
            librtmp->audioBaseTime = timestamp;
            librtmp->lastAudioTime = timestamp;
        }
        
        if(timestamp < librtmp->lastAudioTime)
        {
            UnionLogE("ERROR!!!! audio timestamp %lld is smaller than last one : %lld\n", timestamp, librtmp->lastAudioTime);
            timestamp = librtmp->lastAudioTime;
        }
        else
            librtmp->lastAudioTime = timestamp;
        
        ret = timestamp - librtmp->audioBaseTime;
    }
    else if(UNION_MEDIA_TYPE_VIDEO == type)
    {
        if(librtmp->videoBaseTime < 0)
        {
            librtmp->videoBaseTime = timestamp;
            librtmp->lastVideoTime = timestamp;
        }
        
        if(timestamp < librtmp->lastVideoTime)
        {
            UnionLogE("ERROR!!!! video timestamp %lld is smaller than last one : %lld\n", timestamp, librtmp->lastVideoTime);
            timestamp = librtmp->lastVideoTime;
        }
        else
            librtmp->lastVideoTime = timestamp;
        
        ret = timestamp - librtmp->videoBaseTime;
    }

    return ret;
}

/**
 * @abstract 创建RTMPacket包
 * @param size 要发送内容的长度，不包含rtmp头
 */
static RTMPPacket* union_librtmp_create_rtmppacket(int size)
{
    RTMPPacket *packet = NULL;
    
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + size);
    if(packet)
    {
        memset(packet, 0, RTMP_HEAD_SIZE);
        packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    }
    
    return packet;
}

/**
 * @abstract 销毁RTMPacket
 */
void union_librtmp_free_rtmppacket(RTMPPacket **packet)
{
    if(*packet)
    {
        free(*packet);
        *packet = NULL;
    }
    
    return ;
}

/**
 * @abstract 发送RMTP报文
 */
int union_librtmp_send_packet(RTMP *h_rtmp, RTMPPacket *packet, int size, uint32_t timestamp, int type)
{
    int ret = -1;
    uint8_t channel_id = RTMP_AUDIO_CHANNELID;
    int headerType = RTMP_PACKET_SIZE_MEDIUM;

    if(h_rtmp && packet)
    {
        //LARGE类型的header必须用于块流的起始块或者流时间戳重置的时候
        if(((RTMP_PACKET_TYPE_AUDIO == type || RTMP_PACKET_TYPE_VIDEO == type) && !timestamp) ||
            RTMP_PACKET_TYPE_INFO == type)
            headerType = RTMP_PACKET_SIZE_LARGE;
        
        if(RTMP_PACKET_TYPE_VIDEO == type)
            channel_id = RTMP_VIDEO_CHANNELID;

        packet->m_packetType        = type;
        packet->m_nBodySize         = size;
        packet->m_nChannel          = channel_id;
        packet->m_nTimeStamp        = timestamp;
        packet->m_hasAbsTimestamp   = 0;
        packet->m_headerType        = headerType;
        packet->m_nInfoField2       = h_rtmp->m_stream_id;
        
        if(RTMP_SendPacket(h_rtmp, packet, TRUE))
            ret = 0;
    }
    
    return ret;
}

/**
 * @abstract 组装AVC Configuration并发送
 */
static void union_librtmp_package_avccfg(UnionLibrtmp_t *librtmp, uint8_t *buffer, uint8_t *sps, uint32_t sps_len, uint8_t *pps, uint32_t pps_len)
{
    int i = 0;
    
    if(buffer)
    {
        buffer[i++] = unionflv_get_video_flags(&librtmp->videoEncCfg, 1);
       
        //sequence header
        buffer[i++] = 0x00;

        //composition time offset
        buffer[i++] = 0x00;
        buffer[i++] = 0x00;
        buffer[i++] = 0x00;

        //AVCDecoderConfigurationRecord
        buffer[i++] = 0x01;       //AVCDecoderConfigruationRecord
        buffer[i++] = sps[1];
        buffer[i++] = sps[2];
        buffer[i++] = sps[3];
        buffer[i++] = 0xff;
        
        //sps
        buffer[i++] = 0xe1;
        buffer[i++] = (sps_len >> 8) & 0xff;
        buffer[i++] = sps_len & 0xff;
        memcpy(&buffer[i], sps, sps_len);
        i += sps_len;

        //pps
        buffer[i++] = 0x01;
        buffer[i++] = (pps_len >> 8) & 0xff;
        buffer[i++] = pps_len & 0xff;
        memcpy(&buffer[i], pps, pps_len);
        i += pps_len;
    }

    return ;
}

/**
 * @abstract 发送H264 sps pps
 */
static int union_librtmp_send_h264info(UnionLibrtmp_t *librtmp, UnionAVPacket *packet)
{
    uint8_t *sps = NULL, *pps = NULL;
    uint32_t sps_len = 0, pps_len = 0;
    uint8_t *nal_start = NULL, *nal_end = NULL;
    uint8_t *start = packet->data;
    uint8_t *end = packet->data + packet->size;
    int ret = -1;
   
    if(UNION_RB32(start) == 0x00000001 || UNION_RB24(start) == 0x000001)
    {
        nal_start = unionutils_avc_find_startcode(packet->data, end);
        for( ; ; )
        {
            while(nal_start < end && !*(nal_start++)) ;
            
            if (nal_start == end)
                break;
            
            nal_end = unionutils_avc_find_startcode(nal_start, end);
            if((nal_start[0] & 0x0f) == 7)
            {
                sps = nal_start;
                sps_len = nal_end - nal_start;
            }
            else if((nal_start[0] & 0x0f) == 8)
            {
                pps = nal_start;
                pps_len = nal_end - nal_start;
            }
            
            nal_start = nal_end;
        }
    }
    else
    {
        for( ; start + 4 < end ; )
        {
            uint32_t len = UNION_RB32(start);
            start += 4;
            
            if(len < 0 || start + len > end)
                break;
            
            if((start[0] & 0x0f) == 7)
            {
                sps = start;
                sps_len = len;
            }
            else if((start[0] & 0x0f) == 8)
            {
                pps = start;
                pps_len = len;
            }
            start += len;
        }
    }

    if(NULL == sps || NULL == pps || sps_len <= 0 || pps_len <= 0)
    {
        UnionLogE("invalid h264 video info!\n");
        return ret;
    }

    int size =  RTMP_VIDEOHEADER_LEN + 7 + sps_len + pps_len + 4;
    RTMPPacket *rtmpPacket = union_librtmp_create_rtmppacket(size);
    if(rtmpPacket)
    {
        union_librtmp_package_avccfg(librtmp, rtmpPacket->m_body, sps, sps_len, pps, pps_len);
        ret = union_librtmp_send_packet(librtmp->rtmpHandle, rtmpPacket, size, 0, RTMP_PACKET_TYPE_VIDEO);
        union_librtmp_free_rtmppacket(&rtmpPacket);
    }
    
    return ret;
}

/**
 * @abstract 发送视频帧头信息
 */
static int union_librtmp_send_videoinfo(UnionLibrtmp_t *librtmp, UnionAVPacket *packet)
{
    int ret = -1;
    
    if(UNION_CODEC_ID_H264 == librtmp->videoEncCfg.codecId)
        ret = union_librtmp_send_h264info(librtmp, packet);
    
    return ret;
}

/**
 * @abstract 发送h26X nal unit
 */
static int union_librtmp_send_videodata(UnionLibrtmp_t *librtmp, UnionAVPacket *packet)
{
    RTMPPacket * rtmpPacket = NULL;
    uint8_t *body = NULL, *buf = NULL;
    uint32_t cts = 0;
    int offset = 0;
    int ret = -1;
    
    if(UNION_CODEC_ID_H264 != librtmp->videoEncCfg.codecId)
        return -1;
    
    rtmpPacket = union_librtmp_create_rtmppacket(RTMP_HEAD_SIZE + RTMP_VIDEOHEADER_LEN + packet->size + RTMP_VIDEODATA_Padding);
    if(rtmpPacket)
    {
        buf = packet->data;
        body = (unsigned char *)rtmpPacket->m_body;
        
        //key frame
        body[0] = unionflv_get_video_flags(&librtmp->videoEncCfg, packet->flags & UNION_AV_FLAG_KEY_FRAME);
        
        body[1] = 0x01;         //nal unit
        
        cts = packet->pts - packet->dts;
        body[2] = (uint8_t)(cts >> 16);         //composition time offset
        body[3] = (uint8_t)(cts >> 8);
        body[4] = (uint8_t)cts;
        
        offset = 5;
        if(UNION_RB32(buf) == 0x00000001)
        {
            const uint8_t *p = buf;
            const uint8_t *end = p + packet->size;
            const uint8_t *nal_start, *nal_end;
            
            nal_start = unionutils_avc_find_startcode(p, end);
            for( ; ; )
            {
                while(nal_start < end && !*(nal_start++)) ;
                
                if (nal_start == end)
                    break;
                
                nal_end = unionutils_avc_find_startcode(nal_start, end);
                int nal_size = nal_end - nal_start;
                
                body[offset++] = (nal_size >> 24) & 0xff;
                body[offset++] = (nal_size >> 16) & 0xff;
                body[offset++] = (nal_size >>  8) & 0xff;
                body[offset++] = nal_size & 0xff;
                
                memcpy(body + offset, nal_start, nal_size);
                offset += nal_size;
                
                nal_start = nal_end;
            }
        }
        else
        {
            //copy data
            memcpy(&body[offset], buf, packet->size);
            offset += packet->size;
        }
        
        uint32_t timestamp = union_librtmp_get_relativeTime(librtmp, packet->dts, packet->type);
        ret = union_librtmp_send_packet(librtmp->rtmpHandle, rtmpPacket, offset, timestamp, RTMP_PACKET_TYPE_VIDEO);
        union_librtmp_free_rtmppacket(&rtmpPacket);
    }

    return ret;
}

/**
 * @abstract 发送视频帧
 */
static int union_librtmp_send_videoframe(UnionLibrtmp_t *librtmp, UnionAVPacket *packet)
{
    int ret = -1;
    
    if( NULL == packet || NULL == packet->data)
        return -1;
    
    if(packet->flags & UNION_AV_FLAG_CODEC_CONFIG)
    {
        ret = union_librtmp_send_videoinfo(librtmp, packet);
        if(ret >= 0)
            librtmp->bInitVideo = true;
    }
    else
    {
        if(!librtmp->bInitVideo)
            return -1;
        
        ret = union_librtmp_send_videodata(librtmp, packet);
    }

    return ret;
}

/**
 * @abstract 发送AudioSpecificInfo
 */
static int union_librtmp_send_audiospec(UnionLibrtmp_t *librtmp, UnionAVPacket *packet)
{
    char *body = NULL;
    int ret = -1;
    
    int size = RTMP_AACSPECINFO_LEN + packet->size;
    RTMPPacket *rtmpPacket = union_librtmp_create_rtmppacket(size);
    if(rtmpPacket)
    {
        if(UNION_CODEC_ID_AAC == librtmp->audioEncCfg.codecId )
        {
            body = rtmpPacket->m_body;
            body[0] = unionflv_get_audio_flags(&librtmp->audioEncCfg);
            body[1] = 0x00;
            memcpy(&body[2], packet->data, packet->size);
            
            ret = union_librtmp_send_packet(librtmp->rtmpHandle, rtmpPacket, size, 0, RTMP_PACKET_TYPE_AUDIO);
        }
        union_librtmp_free_rtmppacket(&rtmpPacket);
    }
    
    return ret;
}


/**
 * @abstract 发送音频帧
 */
static int union_librtmp_send_audioframe(UnionLibrtmp_t *librtmp, UnionAVPacket *packet)
{
    char *body = NULL;
    int size = 0;
    int ret = -1;
    
    if(NULL == packet || NULL == packet->data)
        return -1;
    
    if(packet->flags & UNION_AV_FLAG_CODEC_CONFIG)
    {
        ret = union_librtmp_send_audiospec(librtmp, packet);
        if(ret >= 0)
            librtmp->bInitAudio = true;
    }
    else
    {
        if(!librtmp->bInitAudio)
            return -1;
        
        size = RTMP_AACSPECINFO_LEN + packet->size;
        RTMPPacket *rtmpPacket = union_librtmp_create_rtmppacket(size);
        if(rtmpPacket)
        {
            if(UNION_CODEC_ID_AAC == librtmp->audioEncCfg.codecId )
            {
                body = rtmpPacket->m_body;
                body[0] = unionflv_get_audio_flags(&librtmp->audioEncCfg);
                body[1] = 0x01;
                memcpy(&body[2], packet->data, packet->size);
                
                uint32_t timestamp = union_librtmp_get_relativeTime(librtmp, packet->dts, packet->type);
                ret = union_librtmp_send_packet(librtmp->rtmpHandle, rtmpPacket, size, timestamp, RTMP_PACKET_TYPE_AUDIO);
            }
            union_librtmp_free_rtmppacket(&rtmpPacket);
        }
    }
    
    return ret;
}

/**
 * @abstract 释放user-define Metadata中的所有元素
 */
static void union_librtmp_free_dict(UnionDict *dict)
{
    if(NULL == dict || dict->number == 0)
        return ;
    
    for(int i = 0; i < dict->number; i++)
    {
        UnionDictElem *pElem = &dict->elems[i];
        if(pElem->name)
        {
            free(pElem->name);
            pElem->name = NULL;
        }
        
        if(UnionDataType_String == pElem->type)
        {
            free(pElem->val.string);
            pElem->val.string = NULL;
        }
    }
    
    free(dict->elems);
    dict->elems = NULL;
    dict->number = 0;
}

/**
 * @abstract 封装AFMObject
 */
static void union_librtmp_amf_encode(AMFObject *amfObj, uint8_t *name, AMFDataType type, double number, bool bTrue, uint8_t *string)
{
    AMFObjectProperty amfObjProp;
    amfObjProp.p_name.av_val = name;
    amfObjProp.p_name.av_len = strlen(name);

    if(AMF_NUMBER == type)
        amfObjProp.p_vu.p_number = number;
    else if(AMF_BOOLEAN == type)
        amfObjProp.p_vu.p_number = bTrue;
    else if(AMF_STRING == type)
    {
        if(!string)
            return ;
        
        amfObjProp.p_vu.p_aval.av_val = string;
        amfObjProp.p_vu.p_aval.av_len = strlen(string);
    }
    else
        return ;
    
    amfObjProp.p_type = type;
    AMF_AddProp(amfObj, &amfObjProp);
    return ;
}

/**
 * @abstract 组装metada
 */
static int union_librtmp_compose_metadata(UnionLibrtmp_t *librtmp, uint8_t *buffer, int size)
{
    UnionVideoEncCfg *videoEncCfg = NULL;
    UnionAudioEncCfg *audioEncCfg = NULL;
    uint8_t amfString[32], amfArray[1024];
    uint8_t *end = NULL;
    int offset = 0, len = 0;
    AMFObject obj;

    if(NULL == buffer || size <= 0)
        return -1;
    
    memset(&obj, 0, sizeof(AMFObject));
    
    //@setDataFrame
    AVal setDataFrame = AVC("@setDataFrame");
    end = AMF_EncodeString(&amfString, &amfString + sizeof(amfString), &setDataFrame);
    len = end - amfString;
    
    if(len <= 0 || offset + len > size)
        return -1;
    
    memcpy(buffer + offset, amfString, len);
    offset += len;
    
    //onMetadata
    AVal onMetadata = AVC("onMetadata");
    end = AMF_EncodeString(&amfString, &amfString + sizeof(amfString), &onMetadata);
    len = end - amfString;
    
    if(len <= 0 || offset + len > size)
        return -1;
    memcpy(buffer + offset, amfString, len);
    offset += len;

    //video
    videoEncCfg = &librtmp->videoEncCfg;
    union_librtmp_amf_encode(&obj, "duartion", AMF_NUMBER, 0, 0, NULL);
    union_librtmp_amf_encode(&obj, "width", AMF_NUMBER, videoEncCfg->width, 0, NULL);
    union_librtmp_amf_encode(&obj, "height", AMF_NUMBER, videoEncCfg->height, 0, NULL);
    union_librtmp_amf_encode(&obj, "framerate", AMF_NUMBER, videoEncCfg->frameRate, 0, NULL);
    union_librtmp_amf_encode(&obj, "videodatarate", AMF_NUMBER, videoEncCfg->bitrate, 0, NULL);
    union_librtmp_amf_encode(&obj, "interval", AMF_NUMBER, videoEncCfg->iFrameInterval, 0, NULL);
    if(UNION_CODEC_ID_H264 == videoEncCfg->codecId)
        union_librtmp_amf_encode(&obj, "videocodecid", AMF_NUMBER, UNIONFLV_VIDEOCODECID_AVC, 0, NULL);
    
    //audio
    audioEncCfg = &librtmp->audioEncCfg;
    union_librtmp_amf_encode(&obj, "audiodatarate", AMF_NUMBER, audioEncCfg->bitrate, 0, NULL);
    union_librtmp_amf_encode(&obj, "audiosamplerate", AMF_NUMBER, audioEncCfg->sampleRate, 0, NULL);
    union_librtmp_amf_encode(&obj, "stereo", AMF_BOOLEAN, 0, audioEncCfg->channels == 2 ? 1 : 0, NULL);
    int samplesize = audioEncCfg->sampleFmt == UNION_SAMPLE_FMT_U8 ? 8 :16;
    union_librtmp_amf_encode(&obj, "audiosamplesize", AMF_NUMBER, samplesize, 0, NULL);
    if(UNION_CODEC_ID_AAC == audioEncCfg->codecId)
            union_librtmp_amf_encode(&obj, "audiocodecid", AMF_NUMBER, UNIONFLV_SOUNDFORMAT_AAC >> UNIONFLV_SOUNDFORMAT_OFFSET, 0, NULL);
    
    //user define
    UnionDictElem *dictElem = NULL;
    for(int i = 0; i < librtmp->userMetadata.number; i++)
    {
        dictElem = &(librtmp->userMetadata.elems[i]);
        if(UnionDataType_Number ==  dictElem->type)
            union_librtmp_amf_encode(&obj, dictElem->name, AMF_NUMBER, dictElem->val.number,  0, NULL);
        else if(UnionDataType_String == dictElem->type)
            union_librtmp_amf_encode(&obj, dictElem->name, AMF_STRING, 0,  0, dictElem->val.string);
    }
    
    end = AMF_EncodeEcmaArray(&obj, &amfArray, &amfArray + sizeof(amfArray));
    len = end - amfArray;
    
    if(len <= 0 || offset  + len > size)
    {
        AMF_Reset(&obj);
        return -1;
    }
    
    memcpy(buffer + offset, amfArray, len);
    offset += len;
    
    AMF_Reset(&obj);
    return offset;
}

/**
 * @abstract 发送metada
 */
static void union_librtmp_send_metadata(UnionLibrtmp_t *librtmp)
{
    RTMPPacket *rtmpPacket = NULL;
    uint8_t *body = NULL;
    int32_t len = 0;
    int size = RTMP_METADATA_MAXLEN;
 
    rtmpPacket = union_librtmp_create_rtmppacket(RTMP_METADATA_MAXLEN);
    if(rtmpPacket)
    {
        body = (unsigned char *)rtmpPacket->m_body;
        len = union_librtmp_compose_metadata(librtmp, body, RTMP_METADATA_MAXLEN);
        if(len > 0)
            union_librtmp_send_packet(librtmp->rtmpHandle, rtmpPacket, len, 0, RTMP_PACKET_TYPE_INFO);
        union_librtmp_free_rtmppacket(&rtmpPacket);
    }
    return ;
}

/**
 * @abstract 设置默认视频格式
 */
static void union_librtmp_set_default_videocfg(UnionLibrtmp_t *librtmp)
{
    if(NULL == librtmp)
        return ;
    
    UnionVideoEncCfg *videoEncCfg = &librtmp->videoEncCfg;
    memset(videoEncCfg, 0, sizeof(UnionVideoEncCfg));
    videoEncCfg->codecId = UNION_CODEC_ID_H264;
    
    return ;
}

/**
 * @abstract 设置默认音频格式
 */
static void union_librtmp_set_default_audiocfg(UnionLibrtmp_t *librtmp)
{
    if(NULL == librtmp)
        return ;
    
    UnionAudioEncCfg *audioEncCfg = &librtmp->audioEncCfg;
    memset(audioEncCfg, 0, sizeof(UnionAudioEncCfg));
    audioEncCfg->codecId       = UNION_CODEC_ID_AAC;
    audioEncCfg->profile       = UNION_CODEC_PROFILE_AAC_LOW;
    audioEncCfg->sampleFmt     = UNION_SAMPLE_FMT_S16;
    audioEncCfg->sampleRate    = 44100;
    audioEncCfg->channels      = 1;
    audioEncCfg->bitrate       = 0;
    
    return ;
}

/**
 @abstract 获取推流器当前状态
 
 @param publisher 推流对象
 
 @return 返回推流器当前状态
 */
UnionPublisherStatus union_librtmp_get_status(UnionLibrtmp_t *publisher)
{
    if(NULL == publisher)
        return -1;
    
    return publisher->status;
}

/**
 @abstract 设置当前流的视频格式
 
 @param publisher 推流对象
 */
void union_librtmp_set_videocfg(UnionLibrtmp_t *librtmp, UnionVideoEncCfg *vEncCfg)
{
    if(NULL == librtmp || NULL == vEncCfg)
        return ;
    
    pthread_mutex_lock(&librtmp->mutex);
    memcpy(&(librtmp->videoEncCfg), vEncCfg, sizeof(UnionVideoEncCfg));
    librtmp->bSendMeta = false;
    pthread_mutex_unlock(&librtmp->mutex);
    
    return ;
}

/**
 @abstract 设置当前流的音频格式
 
 @param publisher 推流对象
 */
void union_librtmp_set_audiocfg(UnionLibrtmp_t *librtmp, UnionAudioEncCfg *aEncCfg)
{
    if(NULL == librtmp || NULL == aEncCfg)
        return ;
    
    pthread_mutex_lock(&librtmp->mutex);
    memcpy(&(librtmp->audioEncCfg), aEncCfg, sizeof(UnionAudioEncCfg));
    librtmp->bSendMeta = false;
    pthread_mutex_unlock(&librtmp->mutex);
    
    return;
}

/**
 @abstract 设置用户自定义的metadata
 
 @param publisher 推流对象
 @param char 关键字
 @param number 数值
 @param string 字符串
 */
void union_librtmp_set_userMetadata(UnionLibrtmp_t *librtmp, char *key, double number, char *string)
{
    if(NULL == librtmp || NULL == key)
        return ;
    
    UnionDictElem *pElem = NULL;
    librtmp->userMetadata.elems  = (UnionDictElem *)realloc(librtmp->userMetadata.elems, (librtmp->userMetadata.number  + 1)* sizeof(UnionDictElem));
    if(librtmp->userMetadata.elems)
    {
        pElem = &librtmp->userMetadata.elems[librtmp->userMetadata.number];
        memset(pElem, 0, sizeof(UnionDictElem));
        if(string)
        {
            pElem->val.string = malloc(strlen(string) + 1);
            if(pElem->val.string)
            {
                memset(pElem->val.string, 0, strlen(string) + 1);
                strcpy(pElem->val.string, string);
            }
            pElem->type = UnionDataType_String;
        }
        else
        {
            pElem->type = UnionDataType_Number;
            pElem->val.number = number;
        }
        
        pElem->name = malloc(strlen(key) + 1);
        if(pElem->name)
        {
            memset(pElem->name, 0, strlen(key) + 1);
            strcpy(pElem->name, key);
            librtmp->userMetadata.number++;
        }
        else
        {
            if(pElem->val.string)
                free(pElem->val.string);
        }
    }
    
    return ;
}

/**
 * @abstract 开始推流
 */
int union_librtmp_start(UnionLibrtmp_t *librtmp, const char *url, void *param)
{
    RTMP *rtmpHandle = NULL;
    int errorCode = UnionPublisher_Error_Unknown;
    int value = 1;
    int fd = 0;
    
    if(NULL == librtmp || NULL == librtmp->rtmpHandle)
        goto FAIL;
    
    if(NULL == url || strncmp(url, "rtmp://", strlen("rtmp://")))
    {
        errorCode = UnionPublisher_Error_Invalid_Address;
        goto FAIL;
    }
    
    rtmpHandle = librtmp->rtmpHandle;

    if(UnionPublisher_Status_Started != librtmp->status)
    {
        RTMP_Init(rtmpHandle);
        
        //设置url
        if(!RTMP_SetupURL(rtmpHandle, url))
        {
            errorCode = UnionPublisher_Error_Invalid_Address;
            goto FAIL;
        }

        RTMP_EnableWrite(rtmpHandle);
        
        //连接服务器
        if(!RTMP_Connect(rtmpHandle, NULL))
        {
            errorCode = UnionPublisher_Error_ConnectServer_Failed;
            goto FAIL;
        }
        
        fd = RTMP_Socket(rtmpHandle);
        if(fd > 0)
#ifdef ANDROID
            setsockopt(fd, SOL_SOCKET, MSG_NOSIGNAL, &value, sizeof(value));
#else
            setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));
#endif

        //连接流
        if(!RTMP_ConnectStream(rtmpHandle, 0))
        {
            errorCode = UnionPublisher_Error_ConnectStream_Failed;
            goto FAIL;
        }
        
        librtmp->audioBaseTime = -1;
        librtmp->videoBaseTime = -1;

        librtmp->lastAudioTime = -1;
        librtmp->lastVideoTime = -1;
        
        librtmp->bInitAudio = false;
        librtmp->bInitVideo = false;
        librtmp->bSendMeta = false;
        
        librtmp->status = UnionPublisher_Status_Started;
    }
    
    return 0;

FAIL:
    if(rtmpHandle)
        RTMP_Close(rtmpHandle);

    librtmp->status = UnionPublisher_Status_Error;
    return errorCode;
}

/**
 * @abstract 发送数据包
 */
int union_librtmp_send(UnionLibrtmp_t *librtmp, UnionAVPacket* packet)
{
   int ret = -1;
    
    if(NULL == librtmp || NULL == packet || UnionPublisher_Status_Started != librtmp->status)
        goto FAIL;

    pthread_mutex_lock(&librtmp->mutex);
    
    if(!librtmp->bSendMeta)
    {
        union_librtmp_send_metadata(librtmp);
        librtmp->bSendMeta = true;
    }
    
    if(packet->type == UNION_MEDIA_TYPE_VIDEO)
        ret = union_librtmp_send_videoframe(librtmp, packet);
    else if(packet->type == UNION_MEDIA_TYPE_AUDIO)
        ret = union_librtmp_send_audioframe(librtmp, packet);
    pthread_mutex_unlock(&librtmp->mutex);
    
    if(ret >= 0)
        return ret;

FAIL:
    if(librtmp->rtmpHandle)
        RTMP_Close(librtmp->rtmpHandle);
    
    librtmp->status = UnionPublisher_Status_Error;
    return UnionPublisher_Error_Send_Failed;
}

/**
 * @abstract 停止推流
 */
void union_librtmp_stop(UnionLibrtmp_t *librtmp)
{
    if(NULL == librtmp)
        return ;

    if(UnionPublisher_Status_Idle == librtmp->status ||
       UnionPublisher_Status_Stopped == librtmp->status)
        return ;

    if(librtmp->rtmpHandle)
        RTMP_Close(librtmp->rtmpHandle);
   
    librtmp->status = UnionPublisher_Status_Stopped;
    return ;
}

/**
 * @abstract 销毁推流器 
 */
void union_librtmp_close(UnionLibrtmp_t *librtmp)
{
    if(NULL == librtmp)
        return ;
    
    union_librtmp_stop(librtmp);
    
    if(librtmp->rtmpHandle)
    {
        RTMP_Free(librtmp->rtmpHandle);
        librtmp->rtmpHandle = NULL;
    }
    
    union_librtmp_free_dict(&librtmp->userMetadata);
    
    pthread_mutex_destroy(&librtmp->mutex);
    free(librtmp);
    librtmp = NULL;

    return ;
}

/**
 * @abstract 创建librtmp推流器 
 */
UnionLibrtmp_t *union_librtmp_open()
{
    UnionLibrtmp_t      *librtmp = NULL;
    RTMP                *rtmpHandle = NULL;
    
    librtmp = malloc(sizeof(UnionLibrtmp_t));
    if(NULL == librtmp)
        goto FAIL;
    
    memset(librtmp, 0, sizeof(UnionLibrtmp_t));
    
    union_librtmp_set_default_videocfg(librtmp);
    union_librtmp_set_default_audiocfg(librtmp);
    
    rtmpHandle = RTMP_Alloc();
    if(NULL == rtmpHandle)
        goto FAIL;
    
    librtmp->rtmpHandle = rtmpHandle;
    pthread_mutex_init(&librtmp->mutex, NULL);
    
    librtmp->status   = UnionPublisher_Status_Idle;
    return librtmp;
    
FAIL:
    if(librtmp)
    {
        free(librtmp);
        librtmp = NULL;
    }
    
    UnionLogE("librtmp librtmp created failed!\n");
    return NULL;
}
