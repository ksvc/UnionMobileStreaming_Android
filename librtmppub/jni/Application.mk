APP_ABI 	 := armeabi-v7a arm64-v8a x86
APP_STL 	 := gnustl_static

#APP_CFLAGS  += -g -gdwarf-2
APP_CFLAGS   += -O3
APP_OPTIM 	 := release

STREAMER_STATIC = 0

APP_PLATFORM := android-9
