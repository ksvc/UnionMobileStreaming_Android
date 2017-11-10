package com.union.media.streamer.encoder.x264;

import android.util.Log;

import java.nio.ByteBuffer;

/**
 * x264 encoder wrapper.
 */

public class X264EncoderWrapper {
    private static final String TAG = "X264EncoderWrapper";

    private long mInstance;
    private OnEncoded mOnEncoded;

    public interface OnEncoded {
        void onEncoded(ByteBuffer data, long dts, long pts, int flags);
    }

    public X264EncoderWrapper() {
        mInstance = _init();
    }

    public void setOnEncodedCallback(OnEncoded cb) {
        mOnEncoded = cb;
    }

    public int open(int bitrate, int pixFmt, int width, int height,
                    float fps, float iFrameInterval, int profile) {
        return _open(mInstance, bitrate, pixFmt, width, height, fps, iFrameInterval, profile);
    }

    public int adjustBitrate(int bitrate) {
        return _adjust_bitrate(mInstance, bitrate);
    }

    public int encode(ByteBuffer data, int width, int height, long pts, int flags) {
        return _encode(mInstance, data, width, height, pts, flags);
    }

    public void close() {
        _close(mInstance);
    }

    public void release() {
        _release(mInstance);
    }

    private void onEncoded(ByteBuffer data, long dts, long pts, int flags) {
        //Log.d(TAG, "onEncoded: " + data.limit() + " pts=" + pts + " dts=" + dts + " " + flags);
        if (mOnEncoded != null) {
            mOnEncoded.onEncoded(data, dts, pts, flags);
        }
    }

    private native long _init();
    private native int _open(long ptr, int bitrate, int pixFmt, int width, int height,
                             float fps, float iFrameInterval, int profile);
    private native int _adjust_bitrate(long ptr, int bitrate);
    private native int _encode(long ptr, ByteBuffer data, int width, int height,
                               long pts, int flags);
    private native void _close(long ptr);
    private native void _release(long ptr);

    static {
        try {
            System.loadLibrary("x264Encoder");
        } catch (UnsatisfiedLinkError error) {
            Log.e(TAG, "No libx264Encoder.so! Please check");
        }
    }
}
