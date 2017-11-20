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
 @abstract 设置当前流的视频格式
 
 @param publisher 推流对象
 */
void union_librtmp_set_videocfg(UnionLibrtmp_t *publisher, UnionVideoEncCfg *vEncCfg);
    
/**
 @abstract 设置当前流的音频格式
 
 @param publisher 推流对象
 */
void union_librtmp_set_audiocfg(UnionLibrtmp_t *publisher, UnionAudioEncCfg *aEncCfg);
    
/**
 @abstract 设置用户自定义的metadata
 
 @param publisher 推流对象
 @param char 关键字
 @param number 数值
 @param string 字符串
 */
void union_librtmp_set_userMetadata(UnionLibrtmp_t *librtmp, char *key, double number, char *string);

#ifdef __cplusplus
}
#endif

#endif //_UNIONLIBRTMP_H_
