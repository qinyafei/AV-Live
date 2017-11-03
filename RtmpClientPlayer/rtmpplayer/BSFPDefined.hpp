/**
 * BSFP相关宏定义, 必须与公司文档保持一致. 
 * 扩展部分需单独注明. 
 */


#ifndef BSFPDEFINED_HPP_
#define BSFPDEFINED_HPP_

#include <time.h>
#define BSTP_HEADER_MARK  0xBF9D1FDB



/*
  BSTPHeader::type 值
*/
#define BSTP_TYPE_AUDIO       1   // 伴音
#define BSTP_TYPE_VIDEO       2   // 视频
#define BSTP_TYPE_INTELLIGENT 4   // 音视频智能的帧
#define BSTP_TYPE_SPEECH_C2D  6   // 语音对讲,客户端->设备
#define BSTP_TYPE_MESSAGE     9   // 消息帧,如心跳.
#define BSTP_TYPE_COMMAND     10  // 命令请求/应答
#define BSTP_TYPE_SYNC        21  // 视频信息通知
#define BSTP_TYPE_CFG         22  // 配置信息帧(包括解码用配置,扩展用视频附加数据等)



/*
  视频帧时,编码
  BSTPHeader::format[0] 值
*/
#define BSTP_FORMAT_MPEG4   1
#define BSTP_FORMAT_H263    3
#define BSTP_FORMAT_H264    4

/*
  视频帧时,分辨率
  BSTPHeader::format[1] 值
*/
#define BSTP_FORMAT_DCIF   1
#define BSTP_FORMAT_CIF    2
#define BSTP_FORMAT_2CIF   3
#define BSTP_FORMAT_D1     4
#define BSTP_FORMAT_HALFD1 5
#define BSTP_FORMAT_720I   11
#define BSTP_FORMAT_720P   12
#define BSTP_FORMAT_1080I  13
#define BSTP_FORMAT_1080P  14


/*
  视频帧时,帧类型
  BSTPHeader::format[3] 值
*/
#define BSTP_FORMAT_VIDEO_I   1   // i帧
#define BSTP_FORMAT_VIDEO_B   2   // b帧
#define BSTP_FORMAT_VIDEO_P   3   // p帧



/*
  音频帧时,帧类型
  BSTPHeader::format[0] 值
*/
#define BSTP_FORMAT_AUDIO_PCM     0
#define BSTP_FORMAT_AUDIO_OGG     1
#define BSTP_FORMAT_AUDIO_MP2     2
#define BSTP_FORMAT_AUDIO_MP3     3
#define BSTP_FORMAT_AUDIO_AAC     4

#define BSTP_FORMAT_AUDIO_G711u   9 //不带4字节的标准G.711u
#define BSTP_FORMAT_AUDIO_G711u4  109 //带4字节头的G.711u
#define BSTP_FORMAT_AUDIO_ADPCM   10 // 带4字节头的adpcm
#define BSTP_FORMAT_AUDIO_G711    11
#define BSTP_FORMAT_AUDIO_G719    19
#define BSTP_FORMAT_AUDIO_G726    26
#define BSTP_FORMAT_AUDIO_G729    29
#define BSTP_FORMAT_AUDIO_G711a4  111 // 带4字节头的G.711a

// 必须为28字节.
struct BSTPHeader
{
  /*
  固定值: 0xBF9D1FDB
  所有厂商流均是此标志
  */
  unsigned int mark;


  /*
  1=伴音,2=视频,9=控制信息帧
  对于非蓝星的流,不区分伴音和视频,均置为2.

  另外,对于蓝星的语音对讲,6=语音对讲帧(设备->客户端),1=语音对讲帧(客户端->设备,这时device字段为0xFF00)
  */
  char type;


  /*
  通道号: -127-128
  该标志目前没有使用
  */
  char channel;

  /*
  设备序号
  用此标示该流的设备厂商类型.
  在此之前,0xFF00已用在蓝星的语音对讲中,
  蓝星的设备传输流填充时,可能置为0x0000,也可能置为0x0001,不确定是否还有其它值.


  第三方厂商按如下方式定义,
  目前仅用高8位识别厂商,低8位暂置为0x00(为具体流类型预留,用来给客户端明确解码器类型).

  蓝星:
  0xEF00 - 0xEFFF (0xEF01 - 15系列, 0xEF02 - 17系列, 0xEF03 - Limit系列)

  海康,含使用海康板卡的工控机:
  0xEE00 - 0xEEFF

  大华,含使用大华板卡的工控机:
  0xED00 - 0xEDFF

  朗驰:
  0xEC00 - 0xECFF

  大立:
  0xEB00 - 0xEBFF

  同济天跃:
  0xEA00 - 0xEAFF

  Xilink:
  0xE900 - 0xE9FF

  SNC(sony):  0xE800 - 0xE8FF
  SHANY:      0xE700 - 0xE7FF
  ZXW:        0xE600 - 0xE6FF
  松下(PSC):  0xE500 - 0xE5FF 
  三星:       0xE400 - 0xE4FF
  中视里程:    0xE300 - 0xE3FF
  */
  short device;

  /*
  本帧长度,指后续负载长度,bytes
  */
  unsigned int length;

  /*
  该类型帧在该通道(channel)的序号,连续递增
  */
  unsigned int sequence;

  /*
  该帧捕获的系统时间，为time_t标准时间
  对于蓝星的TCP流,该值为前端设备填充,UDP流,则服务器填充,
  第三方厂商全部为服务器填充.
  */
  unsigned int timeStamp;

  /*
  该帧捕获的系统时钟计数，用于音视频精确较时或对屏,
  如服务器填充,则记录微秒
  */
  unsigned int tick;

  /*
  帧的具体格式信息, 具体需见BSFP文档.
  音频帧:           ~~~~~~~~~~~~~~~~
  编码1字节: 2(mp2), 3(mp3) ...
  分辨率1字节:   1-4bits为采样宽度, ：8bit(0)|16bit(1)|32bit(2)|24bit(3)|64bit(3) 
                 5-8bits为通道数(1为双通道, 0是单通道)
  帧率1字节: 2(2k), 4(4k), 8(8k), 16(16k) 
  码率1字节 

  视频帧:
  编码1字节：  1(mpeg4), 3(h263), 4(h264), 11(P440 带64 字节SOLO 帧头的mpeg4)
  分辨率1字节: PAL DCIF(1), CIF(2),2CIF(3),D1(4),halfD1(5)
  帧率1字节：  1-30 的实际取值,单位为帧/秒,如25,
  帧类型1字节：1(I 帧), 2(B 帧), 3(P 帧)

  */
  char format[4];

};


#define IS_BSTP_IFRAME(bstp)  ((bstp).type == BSTP_TYPE_VIDEO && (bstp).format[3] == BSTP_FORMAT_VIDEO_I)


#endif

