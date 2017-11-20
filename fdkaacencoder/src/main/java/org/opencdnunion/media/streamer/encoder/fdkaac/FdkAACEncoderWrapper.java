package org.opencdnunion.media.streamer.encoder.fdkaac;

import android.util.Log;

import java.nio.ByteBuffer;

/**
 * x264 encoder wrapper.
 */

public class FdkAACEncoderWrapper {
    private static final String TAG = "FdkAACEncoderWrapper";

    private long mInstance;
    private OnEncoded mOnEncoded;

    public interface OnEncoded {
        void onEncoded(ByteBuffer data, long dts, long pts, int flags);
    }

    public FdkAACEncoderWrapper() {
        mInstance = _init();
    }

    public void setOnEncodedCallback(OnEncoded cb) {
        mOnEncoded = cb;
    }

    public int open(int bitrate, int sampleFmt, int sampleRate, int channels, int profile) {
        return _open(mInstance, bitrate, sampleFmt, sampleRate, channels, profile);
    }

    public int encode(ByteBuffer data, long pts, int flags) {
        return _encode(mInstance, data, (data == null) ? 0 : data.limit(), pts, flags);
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
    private native int _open(long ptr, int bitrate, int sampleFmt, int sampleRate,
                             int channels, int profile);
    private native int _encode(long ptr, ByteBuffer data, int size, long pts, int flags);
    private native void _close(long ptr);
    private native void _release(long ptr);

    static {
        try {
            System.loadLibrary("fdkAACEncoder");
        } catch (UnsatisfiedLinkError error) {
            Log.e(TAG, "No libfdkAACEncoder.so! Please check");
        }
    }
}
