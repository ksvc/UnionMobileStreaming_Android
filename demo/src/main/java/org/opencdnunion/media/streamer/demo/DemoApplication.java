package org.opencdnunion.media.streamer.demo;

import android.app.Application;

import com.facebook.drawee.backends.pipeline.Fresco;

import org.opencdnunion.media.streamer.util.device.DeviceInfoTools;

/**
 * demo for deviceTools
 * 建议在app加载时对DeviceInfoTools进行初始化，以便最快拿到设备信息
 */

public class DemoApplication extends Application {
    @Override
    public void onCreate() {

        super.onCreate();
        // 初始化本地存储，若本地无信息或者信息已经过期，会向服务器发起请求
        DeviceInfoTools.getInstance().init(this);
        // 初始化fresco库，用来支持动态水印功能
        Fresco.initialize(this);
    }
}
