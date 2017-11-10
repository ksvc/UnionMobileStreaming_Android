LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

PREBUILT_PATH := $(LOCAL_PATH)/../../prebuilt
LIB_RTMP_PATH := $(PREBUILT_PATH)/librtmp/$(TARGET_ARCH_ABI)
LIB_RTMP_LIB := ../$(LIB_RTMP_PATH)/lib

include $(CLEAR_VARS)
LOCAL_MODULE := rtmp
LOCAL_SRC_FILES := $(LIB_RTMP_LIB)/librtmp.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rtmppub
LOCAL_CONLYFLAGS := -std=c99

LOCAL_SRC_FILES := publisher/UnionFLV.c \
					publisher/UnionLibrtmp.c \
					publisher/UnionPublisherUtils.c \
					jni_UnionPublisherWrapper.c


LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(PREBUILT_PATH)/include
LOCAL_C_INCLUDES += $(LIB_RTMP_PATH)/include
LOCAL_C_INCLUDES += $(LIB_RTMP_PATH)

LOCAL_STATIC_LIBRARIES := rtmp
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog -lz

include $(BUILD_SHARED_LIBRARY)