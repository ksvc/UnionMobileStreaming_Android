package org.opencdnunion.media.streamer.encoder.fdkaac;

import android.util.Log;

import org.opencdnunion.media.streamer.encoder.AudioEncoderBase;
import org.opencdnunion.media.streamer.framework.AVConst;
import org.opencdnunion.media.streamer.framework.AudioBufFrame;
import org.opencdnunion.media.streamer.framework.AudioEncodeConfig;

import java.nio.ByteBuffer;

/**
 * Audio encoder of AVCodec
 */
public class FdkAACEncoder extends AudioEncoderBase
        implements FdkAACEncoderWrapper.OnEncoded {
    private final static String TAG = "FdkAACEncoder";
    private final static boolean VERBOSE = false;

    private FdkAACEncoderWrapper mEncoder;

    @Override
    protected int doStart(AudioEncodeConfig config) {
        mEncoder = new FdkAACEncoderWrapper();
        mEncoder.setOnEncodedCallback(this);
        int audioProfile = config.profile;
        if (audioProfile == AVConst.PROFILE_AAC_HE_V2 && config.channels == 1) {
            audioProfile = AVConst.PROFILE_AAC_HE;
            ((AudioEncodeConfig)mEncodeConfig).profile = AVConst.PROFILE_AAC_HE;
            Log.w(TAG, "set aac_he_v2 for mono audio, fallback to aac_he");
        }
        return mEncoder.open(config.bitrate, config.sampleFmt, config.sampleRate,
                config.channels, audioProfile);
    }

    @Override
    protected void doStop() {
        mEncoder.close();
        mEncoder.release();
        mEncoder = null;
        onEncoded(null, 0, 0, AVConst.FLAG_END_OF_STREAM);
    }

    @Override
    protected void doFlush() {
        if (VERBOSE) {
            Log.d(TAG, "flush encoder");
        }
        mEncoder.encode(null, 0, 0);
    }

    @Override
    protected int doEncode(AudioBufFrame frame) {
        return mEncoder.encode(frame.buf, frame.pts, frame.flags);
    }

    /**
     * @hide
     */
    @Override
    public void onEncoded(ByteBuffer data, long dts, long pts, int flags) {
        sendEncodedPacket(data, dts, pts, flags);
    }
}
