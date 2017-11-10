LOCAL_PATH := $(call my-dir)

PREBUILT_PATH := $(LOCAL_PATH)/../../prebuilt
FDK_AAC_PATH := $(PREBUILT_PATH)/fdk-aac
FDK_AAC_LIB := ../$(FDK_AAC_PATH)/$(TARGET_ARCH_ABI)/lib

include $(CLEAR_VARS)
LOCAL_MODULE := libfdk-aac
LOCAL_SRC_FILES := $(FDK_AAC_LIB)/libfdk-aac.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := fdkAACEncoder

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(PREBUILT_PATH)/include
LOCAL_C_INCLUDES += $(FDK_AAC_PATH)/$(TARGET_ARCH_ABI)/include

LOCAL_SRC_FILES := fdkAACEncoder.c \
				   jni_FdkAACEncoderWrapper.c

LOCAL_STATIC_LIBRARIES := libfdk-aac
LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)