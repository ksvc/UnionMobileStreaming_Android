package com.union.media.streamer.encoder.x264;

import android.util.Log;

import com.ksyun.media.streamer.encoder.VideoBufEncoderBase;
import com.ksyun.media.streamer.framework.AVConst;
import com.ksyun.media.streamer.framework.ImgBufFrame;
import com.ksyun.media.streamer.framework.VideoEncodeConfig;

import java.nio.ByteBuffer;

/**
 * Video encoder of AVCodec
 */
public class X264Encoder extends VideoBufEncoderBase
        implements X264EncoderWrapper.OnEncoded {
    private final static String TAG = "AVCodecVideoEncoder";
    private final static boolean VERBOSE = false;

    private X264EncoderWrapper mEncoder;

    @Override
    protected int doStart(VideoEncodeConfig config) {
        mEncoder = new X264EncoderWrapper();
        mEncoder.setOnEncodedCallback(this);
        return mEncoder.open(config.bitrate, config.pixFmt, config.width, config.height,
                config.frameRate, config.iFrameInterval, config.profile);
    }

    @Override
    protected void doStop() {
        mEncoder.close();
        mEncoder.release();
        mEncoder = null;
        onEncoded(null, 0, 0, AVConst.FLAG_END_OF_STREAM);
    }

    @Override
    protected void doAdjustBitrate(int bitrate) {
        mEncoder.adjustBitrate(bitrate);
    }

    @Override
    protected void doFlush() {
        if (VERBOSE) {
            Log.d(TAG, "flush encoder");
        }
        mEncoder.encode(null, 0, 0, 0, 0);
    }

    @Override
    protected int doEncode(ImgBufFrame frame) {
        return mEncoder.encode(frame.buf, frame.format.width, frame.format.height,
                frame.pts, frame.flags);
    }

    @Override
    public void onEncoded(ByteBuffer data, long dts, long pts, int flags) {
        sendEncodedPacket(data, dts, pts, flags);
    }
}
