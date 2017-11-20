package org.opencdnunion.streamer.publisher.librtmp;

import android.util.Log;

import org.opencdnunion.media.streamer.framework.AVConst;
import org.opencdnunion.media.streamer.framework.AVPacketBase;
import org.opencdnunion.media.streamer.framework.AudioEncodeConfig;
import org.opencdnunion.media.streamer.framework.AudioPacket;
import org.opencdnunion.media.streamer.framework.ImgPacket;
import org.opencdnunion.media.streamer.framework.VideoEncodeConfig;
import org.opencdnunion.media.streamer.publisher.Publisher;

/**
 * Rtmp publish module
 */

public class RtmpPublisher extends Publisher {
    private final static String TAG = "RtmpPublisher";

    private UnionPublisherWrapper mPublisherWrapper;

    public RtmpPublisher(String name) {
        super(name);
        mPublisherWrapper = new UnionPublisherWrapper();
    }

    @Override
    protected int doStart(String url) {
        return mPublisherWrapper.start(url);
    }

    @Override
    protected int doFrameAvailable(AVPacketBase packet) {
        int type = AVConst.UNION_MEDIA_TYPE_VIDEO;
        if (packet instanceof AudioPacket) {
            type = AVConst.UNION_MEDIA_TYPE_AUDIO;
            if ((packet.flags & AVConst.FLAG_CODEC_CONFIG) != 0) {
                AudioEncodeConfig cfg = ((AudioPacket) packet).cfg;
                mPublisherWrapper.setAudioEncCfg(cfg.codecId, cfg.profile,cfg.sampleFmt,
                        cfg.sampleRate, cfg.channels, cfg.bitrate / 1000);
            }
        } else if (packet instanceof ImgPacket) {
            if ((packet.flags & AVConst.FLAG_CODEC_CONFIG) != 0) {
                VideoEncodeConfig cfg = ((ImgPacket) packet).cfg;
                mPublisherWrapper.setVideoEncCfg(cfg.codecId, cfg.profile, cfg.pixFmt,
                        cfg.width, cfg.height, cfg.bitrate / 1000, cfg.frameRate, cfg.iFrameInterval);
            }
        }
        return mPublisherWrapper.sendPacket(type, packet.buf, packet.buf.limit(), packet.dts,
                packet.pts, packet.flags);
    }
    @Override
    protected int doStop() {
        return mPublisherWrapper.stop();
    }

    @Override
    protected void doFormatChanged(Object format) {

    }

    @Override
    protected void doRelease() {
        mPublisherWrapper.release();
    }

    /**
     * add user define string meta data info
     * @param key meta data key
     * @param value meta data key
     */
    public void addMetaOption(String key, String value) {
        mPublisherWrapper.addMetaOptionStr(key, value);
    }

    /**
     * add user define double meta data info
     * @param key meta data key
     * @param value meta data key
     */
    public void addMetaOption(String key, double value) {
        mPublisherWrapper.addMetaOptionNum(key, value);
    }
}
