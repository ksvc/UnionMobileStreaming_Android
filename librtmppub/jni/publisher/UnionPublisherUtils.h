//
//  UnionPublisherUtils.h
//  UnionPublisher
//
//  Created by shixuemei on 10/30/17.
//  Copyright © 2017 ksyun. All rights reserved.
//
#include <stdio.h>

#define UNION_RB16(x)                        \
 ((((const uint8_t*)(x))[0] << 8) |          \
   ((const uint8_t*)(x))[1])

#define UNION_RB24(x)                        \
 ((((const uint8_t*)(x))[0] << 16) |         \
  (((const uint8_t*)(x))[1] <<  8) |         \
    ((const uint8_t*)(x))[2])

#define UNION_RB32(x)                             \
 (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
            (((const uint8_t*)(x))[1] << 16) |    \
            (((const uint8_t*)(x))[2] <<  8) |    \
             ((const uint8_t*)(x))[3])

/**
 @abstract 查找nal start code
 
 @param p 起始地址
 @param end 结束地址
 
 @return nal unit的起始地址(带start code)
 */
const uint8_t *unionutils_avc_find_startcode(const uint8_t *p, const uint8_t *end);
