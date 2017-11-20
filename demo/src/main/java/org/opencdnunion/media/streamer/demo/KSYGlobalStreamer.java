package org.opencdnunion.media.streamer.demo;

import android.annotation.SuppressLint;

import org.opencdnunion.media.streamer.kit.UnionStreamer;

/**
 * Class to store global streaming instance.
 */

public class KSYGlobalStreamer {

    @SuppressLint("StaticFieldLeak")
    private static UnionStreamer sStreamer;

    // private constructor
    private KSYGlobalStreamer() {

    }

    public static void setInstance(UnionStreamer streamer) {
        sStreamer = streamer;
    }

    public static UnionStreamer getInstance() {
        return sStreamer;
    }
}
