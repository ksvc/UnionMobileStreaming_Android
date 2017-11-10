//  UnionLibrtmp.h
//  UnionLibrtmp
//
//  Created by shixuemei on 10/28/17.
//  Copyright © 2017 ksyun. All rights reserved.
//
#ifndef _UNIONLIBRTMP_H_
#define _UNIONLIBRTMP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "UnionAVCommon.h"
#include "UnionPublisherDef.h"
    
typedef struct UnionLibrtmp UnionLibrtmp_t;

/**
 @abstract 创建推流对象
 
 @param mode 推流方式
     
 @return  成功返回UnionLibrtmp实例，失败返回nil
 */
UnionLibrtmp_t *union_librtmp_open();

/**
 @abstract 启动推流
     
 @param librtmp 推流对象
 @param url 目标地址
     
 @return  成功返回0，失败返回负数
 */
int union_librtmp_start(UnionLibrtmp_t *librtmp, const char *url, void *param);

/**
 @abstract 发送数据包(本地或者网络) 
     
 @param librtmp 推流对象
 @param pkt 数据包
     
 @return  成功返回0，失败返回负数
 */
int union_librtmp_send(UnionLibrtmp_t *librtmp, UnionAVPacket* packet);

/**
 @abstract 停止推流
     
 @param librtmp 推流对象
 */
void union_librtmp_stop(UnionLibrtmp_t *librtmp);

/**
 @abstract 销毁推流对象 
     
 @param librtmp 推流对象
 */
void union_librtmp_close(UnionLibrtmp_t *librtmp);

/**
 @abstract 获取推流器当前状态 
     
 @param librtmp 推流对象
 
 @return 返回推流器当前状态 
 */
UnionPublisherStatus union_librtmp_get_status(UnionLibrtmp_t *librtmp);
    
/**
 @abstract 获取当前流的视频信息
     
 @param librtmp 推流对象
     
 @return 返回推流器当前状态
*/
UnionVideoEncCfg* union_librtmp_get_videocfg(UnionLibrtmp_t *librtmp);
    
    
/**
 @abstract 获取当前流的音频信息
     
 @param librtmp 推流对象
     
 @return 当前流的音频格式
 */
UnionAudioEncCfg* union_librtmp_get_audiocfg(UnionLibrtmp_t *librtmp);

#ifdef __cplusplus
}
#endif

#endif //_UNIONLIBRTMP_H_
