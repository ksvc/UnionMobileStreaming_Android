package com.union.streamer.publisher.librtmp;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import com.ksyun.media.streamer.framework.AVConst;
import com.ksyun.media.streamer.framework.AVPacketBase;
import com.ksyun.media.streamer.framework.AudioEncodeConfig;
import com.ksyun.media.streamer.framework.AudioPacket;
import com.ksyun.media.streamer.framework.ImgPacket;
import com.ksyun.media.streamer.framework.SinkPin;
import com.ksyun.media.streamer.framework.VideoEncodeConfig;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * Rtmp publish module
 */

public class UnionPublisher {
    private final static String TAG = "UnionPublisher";

    public static final int ERROR_UNKNOWN           = -3000;
    public static final int ERROR_START_FAILED      = -3001;
    public static final int ERROR_WRITE_FAILED      = -3002;
    public static final int ERROR_STOP_FAILED       = -3003;

    private static final int CMD_START = 1;
    private static final int CMD_STOP = 2;
    private static final int CMD_WRITE_FRAME = 3;
    private static final int CMD_RELEASE = 4;

    public static final int INFO_STARTED = 1;
    public static final int INFO_STOPPED = 2;

    public static final int STATE_IDLE = 0;
    public static final int STATE_STARTING = 1;
    public static final int STATE_PUBLISHING = 2;
    public static final int STATE_STOPPING = 3;

    private UnionPublisherWrapper mPublisherWrapper;
    private SinkPin<AVPacketBase> mAVSinkPin;

    private final Handler mMainHandler;
    protected PubListener mPubListener;
    protected HandlerThread mPublishThread;
    protected Handler mPublishHandler;
    protected AtomicInteger mState;

    private final Object mFrameSyncObject = new Object();
    private boolean mFrameSended;

    public UnionPublisher() {
        mAVSinkPin = new AVSinkPin();

        mState = new AtomicInteger(STATE_IDLE);
        mMainHandler = new Handler(Looper.getMainLooper());
        initPubThread("rtmppub");

        mPublisherWrapper = new UnionPublisherWrapper();
        mFrameSended = false;
    }

    public SinkPin<AVPacketBase> getAVSinkPin() {
        return mAVSinkPin;
    }

    public boolean start(String url) {
        if (mState.get() != STATE_IDLE && mState.get() != STATE_STOPPING) {
            Log.e(TAG, "startStream on invalid state");
            return false;
        }
        if (mPublishThread != null) {
            Message msg = mPublishHandler.obtainMessage(CMD_START, url);
            mPublishHandler.sendMessage(msg);
            return true;
        }
        return false;
    }

    public void stop() {
        if (mState.get() == STATE_IDLE || mState.get() == STATE_STOPPING) {
            return;
        }
        if (mPublishThread != null) {
            Message msg = mPublishHandler.obtainMessage(CMD_STOP);
            mPublishHandler.sendMessage(msg);
        }
    }

    public void release() {
        if (mPublishThread != null) {
            Message msg = mPublishHandler.obtainMessage(CMD_RELEASE, mPublishThread);
            mPublishHandler.sendMessage(msg);
            mPublishThread = null;
        }
    }

    private class AVSinkPin extends SinkPin<AVPacketBase> {
        @Override
        public void onFormatChanged(Object format) {

        }

        @Override
        public void onFrameAvailable(AVPacketBase packet) {
            if (mPublishThread != null) {
                Message msg = mPublishHandler.obtainMessage(CMD_WRITE_FRAME, packet);
                mPublishHandler.sendMessage(msg);
            }

            final int TIMEOUT_MS = 100;
            synchronized (mFrameSyncObject) {
                while (!mFrameSended) {
                    try {
                        mFrameSyncObject.wait(TIMEOUT_MS);
                        if (!mFrameSended) {
                            continue;
                        }
                    } catch (InterruptedException ie) {
                        throw new RuntimeException(ie);
                    }
                }
                mFrameSended = false;
            }
        }

        @Override
        public void onDisconnect(boolean recursive){
            super.onDisconnect(recursive);
            if (recursive) {
                release();
            }
        }
    }

    public interface PubListener {
        void onInfo(int type, long msg);

        void onError(int err, long msg);
    }

    public void setPubListener(PubListener listener) {
        this.mPubListener = listener;
    }

    private void postInfo(final int type, final long msg) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mPubListener != null) {
                    mPubListener.onInfo(type, msg);
                }
            }
        });
    }

    private void postError(final int err, final long msg) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mPubListener != null) {
                    mPubListener.onError(err, msg);
                }
            }
        });
    }

    private int doStart(String url) {
        return mPublisherWrapper.start(url);
    }

    private int doWriteFrame(AVPacketBase packet) {
        int type = AVConst.UNION_MEDIA_TYPE_VIDEO;
        if (packet instanceof AudioPacket) {
            type = AVConst.UNION_MEDIA_TYPE_AUDIO;
            if ((packet.flags & AVConst.FLAG_CODEC_CONFIG) != 0) {
                AudioEncodeConfig cfg = ((AudioPacket) packet).cfg;
                mPublisherWrapper.setAudioEncCfg(cfg.codecId, cfg.profile,cfg.sampleFmt,
                        cfg.sampleRate, cfg.channels, cfg.bitrate);
            }
        } else if (packet instanceof ImgPacket) {
            if ((packet.flags & AVConst.FLAG_CODEC_CONFIG) != 0) {
                VideoEncodeConfig cfg = ((ImgPacket) packet).cfg;
                mPublisherWrapper.setVideoEncCfg(cfg.codecId, cfg.profile, cfg.pixFmt,
                        cfg.width, cfg.height, cfg.bitrate, cfg.frameRate, cfg.iFrameInterval);
            }
        }
        return mPublisherWrapper.sendPacket(type, packet.buf, packet.buf.limit(), packet.dts,
                packet.pts, packet.flags);
    }
    private int doStop() {
        return mPublisherWrapper.stop();
    }

    private void doRelease() {
        mPublisherWrapper.release();
    }

    private void initPubThread(String name) {
        mPublishThread = new HandlerThread(name + "thread");
        mPublishThread.start();
        mPublishHandler = new Handler(mPublishThread.getLooper()) {
            @Override
            @SuppressWarnings("unchecked")
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case CMD_START: {
                        if (mState.get() == STATE_IDLE) {
                            mState.set(STATE_STARTING);

                            int err = doStart((String) msg.obj);
                            mState.set(err == 0 ? STATE_PUBLISHING : STATE_IDLE);
                            if (err == 0) {
                                postInfo(INFO_STARTED, 0);
                            } else {
                                postError(ERROR_START_FAILED, err);
                            }
                        }
                        break;
                    }
                    case CMD_WRITE_FRAME:
                        if (mState.get() == STATE_PUBLISHING) {
                            int err = 0;
                            synchronized (mFrameSyncObject) {
                                if (mFrameSended) {
                                    Log.e(TAG, "mFrameSended already set, frame could be dropped");
                                    return;
                                }

                                mFrameSended = true;
                                err = doWriteFrame((AVPacketBase) msg.obj);
                                mFrameSyncObject.notifyAll();
                            }
                            if (err < 0) {
                                postError(ERROR_WRITE_FAILED, err);
                            }
                        } else {
                            mFrameSended = true;
                            Log.e(TAG, "Please start publisher before encoder, " +
                                    "or memory leak may be occured!");
                        }
                        break;
                    case CMD_STOP: {
                        if (mState.get() == STATE_PUBLISHING) {
                            mState.set(STATE_STOPPING);

                            doStop();
                            mState.set(STATE_IDLE);
                            postInfo(INFO_STOPPED, 0);
                        }
                        break;
                    }
                    case CMD_RELEASE: {
                        doRelease();
                        ((HandlerThread) msg.obj).quit();
                        break;
                    }
                }
            }
        };
    }
}
