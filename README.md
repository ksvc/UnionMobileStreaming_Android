# Union Mobile Streaming SDK(Android)

全模块化的移动直播推流方案的出现，是为了应对移动直播业务需求的任意拓宽。

* [About](#1-about)
* [License](#2-license)
* [Getting Started](#3-getting-started)
* [Feedback](#4-feedback)

## 1. about
[UnionMobileStreaming][ums]是全模块化的移动直播推流方案，完成了移动平台音视频数据的采集、处理、编码和推流工作。
基于模块化设计，主要思路如下：
* 提供全通道全部raw数据；
* 支持任意节点的第三方数据处理；
* 支持任意编码codec的加载和使用；
* 支持任意推流protocol的加载和使用；
---
基于模块化设计，模块功能分为：

* 采集
    * 内建基于AudioRecord的音频数据采集
    * 内建基于系统组件的视频数据采集
    * 提供所有采集的音视频raw数据
    * 支持任意第三方音视频数据输入（连麦、录屏等）

* 处理
    * 基于SrcPin接口的filter接入（覆盖美颜、AR特效等）

* 混合
    * 支持最大八个通道的音频输入混合
    * 支持最大八个图层的视频输入混合

* 编码
    * 内建基于MediaCodec的音频硬编码
    * 内建基于MediaCodec的视频硬编码
    * 支持基于发送速率的编码码控
    * 支持第三方视频软编码实现（例如[x264][x264]、[ksc265][ksc265]）
    * 支持第三方音频软编码实现（例如[fdk-aac][fdk-aac]）
  
* 推流
    * 支持第三方发送实现（例如[librtmp][librtmp]）
    * 支持第三方UDP发送
    * 支持任意发送协议

## 2. license
[UnionMobileStreaming][ums] is a SDK with closed source.

## 3. getting started
please read [wiki][wiki].

## 4. feedback
Please post your discussion on github [issues][issues].
 
[ums]:https://github.com/ksvc/UnionMobileStreaming_Android
[ksc265]:https://github.com/ksvc/ks265codec
[x264]:https://www.videolan.org/developers/x264.html
[fdk-aac]:http://wiki.hydrogenaud.io/index.php?title=Fraunhofer_FDK_AAC#.28lib.29fdk-aac
[librtmp]:https://rtmpdump.mplayerhq.hu/librtmp.3.html
[license]:https://github.com/ksvc/UnionMobileStreaming_Android/blob/master/LICENSE
[wiki]:https://github.com/ksvc/UnionMobileStreaming_Android/wiki
[issues]:https://github.com/ksvc/UnionMobileStreaming_Android/issues
