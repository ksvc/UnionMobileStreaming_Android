#ifndef __UNIONENCODERDEF_H__
#define __UNIONENCODERDEF_H__

#include <UnionAVCommon.h>

enum {
    UNION_ENC_ERR_NONE = 0,
    UNION_ENC_ERR_UNKNOWN = -1,
    UNION_ENC_ERR_UNSUPPORTED = -2,
};

typedef void (*UnionAVEncOpaqueFree)(void* opaque);
typedef void (*UnionAVEncCallback)(UnionAVPacket* out, void* opaque);

#endif //__UNIONENCODERDEF_H__
