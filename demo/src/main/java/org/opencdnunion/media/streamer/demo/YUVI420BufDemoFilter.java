package org.opencdnunion.media.streamer.demo;

import android.util.Log;

import org.opencdnunion.media.streamer.filter.imgtex.ImgTexBufFilter;
import org.opencdnunion.media.streamer.framework.AVConst;
import org.opencdnunion.media.streamer.util.gles.GLRender;

import java.nio.ByteBuffer;

/**
 * Demo filter to handle YUV420 buffer in gpu pipe.
 */

public class YUVI420BufDemoFilter extends ImgTexBufFilter {
    private static final String TAG = "YUVI420BufDemoFilter";

    public YUVI420BufDemoFilter(GLRender glRender) {
        super(glRender, AVConst.PIX_FMT_I420);
    }

    @Override
    protected void onSizeChanged(int[] stride, int width, int height) {
        Log.d(TAG, "onSizeChanged " + stride[0] + " " + width + "x" + height);
        // do nothing.
    }

    @Override
    protected ByteBuffer doFilter(ByteBuffer buffer, int[] stride, int width, int height) {
        Log.d(TAG, "doFilter " + buffer + " " + stride[0] + " " + width + "x" + height);
        // handle buffer here
        return buffer;
    }
}
