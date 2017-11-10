package com.union.streamer.publisher.librtmp;

import android.util.Log;

import java.nio.ByteBuffer;

/**
 * Rtmp jni wrapper
 */

public class UnionPublisherWrapper {
    private final static String TAG = "UnionPublisherWrapper";

    private long mInstance = 0;

    public UnionPublisherWrapper() {
        mInstance = _init();
    }

    public int setAudioEncCfg(int codecId, int profile, int sampleFmt, int sampleRate,
                               int channels, int bitrate) {
        if (mInstance == 0) {
            return -1;
        }
        _setAudioEncCfg(mInstance, codecId, profile, sampleFmt, sampleRate, channels, bitrate);
        return 0;
    }

    public int setVideoEncCfg(int codecId, int profile, int pixFmt, int width, int height,
                               int bitrate, float frameRate, float iFrameIntervale) {
        if (mInstance == 0) {
            return -1;
        }
        _setVideoEncCfg(mInstance, codecId, profile, pixFmt, width, height, bitrate, frameRate,
                iFrameIntervale);
        return 0;
    }

    public int start(String url) {
        if (mInstance == 0) {
            return -1;
        }
        return _start(mInstance, url);
    }

    public int sendPacket(int type, ByteBuffer data, int size, long dts, long pts, int flags) {
        if (mInstance == 0) {
            return -1;
        }
        return _send_packet(mInstance, type, data, size, dts, pts, flags);
    }

    public int stop() {
        if (mInstance == 0) {
            return -1;
        }
        return _stop(mInstance);
    }

    public void release() {
        if (mInstance != 0) {
            _release(mInstance);
        }
    }

    private native long _init();
    private native void _setAudioEncCfg(long ptr, int codecId, int profile, int sampleFmt, int sampleRate,
                                       int channels, int bitrate);
    private native void _setVideoEncCfg(long ptr, int codecId, int profile, int pixFmt, int width,
                                        int height, int bitrate, float frameRate, float iFrameInterval);
    private native int _start(long ptr, String uri);
    private native int _send_packet(long ptr, int type, ByteBuffer data, int size, long dts,
                                    long pts, int flags);
    private native int _stop(long ptr);
    private native void _release(long ptr);

    static {
        try {
            System.loadLibrary("rtmppub");
        } catch (UnsatisfiedLinkError error) {
            Log.e(TAG, "No publisher.so! Please check");
        }
    }
}
