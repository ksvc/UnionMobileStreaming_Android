# Union Mobile Streaming SDK(Android)

全模块化的移动直播推流方案，支持任意编码codec和推流发送模块的接入。

* [About](#1.about)
* [License](#2.license)

## 1.about
[UnionMobileStreaming][ums]是全模块化的移动直播推流方案，完成了移动平台音视频数据的采集、处理、编码和推流工作。

基于模块化设计，模块功能分为：

* 采集
  * 音视频raw数据处理
  * 音视频raw数据混合

* 处理

* 编码
  * 支持基于MediaCodec的视频硬编码
  * 支持基于MediaCodec的音频硬编码
  * 支持基于发送速率的编码码控
  * 支持第三方视频软编码实现（例如[x264][x264]、[ksc265][ksc265}）
  * 支持第三方音频软编码实现（例如[fdk-aac][fdk-aac]）
  
* 推流
  * 支持第三方发送实现（例如[librtmp][librtmp]）
  * 支持第三方UDP发送
  * 支持任意发送协议

## 2.license

[ums]:https://github.com/ksvc/UnionMobileStreaming_Android
[ksc265]:https://github.com/ksvc/ks265codec
[x264]:https://www.videolan.org/developers/x264.html
[fdk-aac]:http://wiki.hydrogenaud.io/index.php?title=Fraunhofer_FDK_AAC#.28lib.29fdk-aac
[librtmp]:https://rtmpdump.mplayerhq.hu/librtmp.3.html
[wiki]:https://github.com/ksvc/UnionMobileStreaming_Android/wiki
[issues]:https://github.com/ksvc/UnionMobileStreaming_Android/issues
