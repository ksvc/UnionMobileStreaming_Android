package org.opencdnunion.media.streamer.demo;

import android.util.Log;

import org.opencdnunion.media.streamer.filter.imgtex.RGBABufFilter;
import org.opencdnunion.media.streamer.util.gles.GLRender;

import java.nio.ByteBuffer;

/**
 * Demo filter to handle RBGA buffer in gpu pipe.
 */

public class RGBABufDemoFilter extends RGBABufFilter {
    private static final String TAG = "RGBABufDemoFilter";

    public RGBABufDemoFilter(GLRender glRender) {
        super(glRender);
    }

    @Override
    protected void onSizeChanged(int stride, int width, int height) {
        Log.d(TAG, "onSizeChanged " + stride + " " + width + "x" + height);
        // do nothing.
    }

    @Override
    protected ByteBuffer doFilter(ByteBuffer buffer, int stride, int width, int height) {
        Log.d(TAG, "doFilter " + buffer + " " + stride + " " + width + "x" + height);
        // handle buffer here
        return buffer;
    }
}
