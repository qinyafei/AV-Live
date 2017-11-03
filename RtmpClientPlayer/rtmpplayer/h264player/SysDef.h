#pragma once

//////////////////////////////////////////////////////////////////////////
// 以下代码出自蓝星音视频数据格式标准
struct magicInfo_t
{
  char logo[8];    // 固定为"BLUESKY"
  char vcodec;     // 视频编码，与_videoInfo_t.codec相同
  char acodec;     // 视频编码，与_audioInfo_t.codec相同
  char media;      // 片段内包含的媒体类型(bitmap): bit1=视频(0x01); bit2=音频(0x02); bit3=语音(0x04); bit4=智能(0x08); bit8=索引
  char version;    // 头信息版本号，当前为, 其头长度固定为bytes
  char res[4];     // (v1.3+)
};
//统计信息
struct sumInfo_t
{
  unsigned long long size; // 文件大小(单位字节,该字段是文件首地址
  unsigned int time;       // 录制时的系统时间,绝对值,time(NULL)返回值, time_t为bit
  unsigned long length;    // 录像时间长度,秒
  char vendor[4];          // 录像内容的厂商信息，DH|HK|DL|BS|BSK|BSA(PNX-mux)|BSN(-p440)
  char nextClip;           // (v1.3+) 0: 无；1：有
  char res[3];             // (v1.3+) 保留字段
};
//视频
struct videoInfo_t
{
  char codec;            // 编码方式: 1(mpeg4) | 3(h263) | 4(h264) |11(带字节SOLO帧头的mpeg4)
  char resolution;       // 分辨率: 1:CIF; 2:2CIF; 3:1/2D1; 4:D1, 5:QCIF, NTSC时+128
  char frameRate;        // 帧率
  char flags;            // 标志位，暂保留为
  unsigned long bitRate; // 码率(kb/s):
};
//音频
struct audioInfo_t
{
  char codec;            // 编码方式: 编码：(ogg) | 2(mp2) | 3(mp3) | 11(g.711) | 21(g.721) | 23(g.723) | 29(g.729)
  char resolution;       // 8-5bit为通道数：(单通道(0)|双通道(1|...)，4-1bit为采样宽度：bit(0)|16bit(1)|32bit(2)|24bit(3)|64bit(3)...
  char frameRate;        // 采样率(khz): 如/16/32/44
  char flags;            // 标志位，暂保留为
  unsigned long bitRate; // 码率(kb/s):
};
//网络
struct netInfo_t
{
  unsigned long clientAddr; // 录制主机的IP地址
  unsigned long srcAddr;    // 视频源IP地址,录像机磁盘录像时为
  short srcPort;            // 视频源端口
  short dstPort;            // 视频目的端口
  char proto;               // 网络协议: 0x00: 本地磁盘，x01: RTP, 0x0x02: TCP
  char res[3];              //(v1.3+)保留
};
//源信息
struct sourceInfo_t
{
  int district; // 地区id,录像机磁盘录像时为
  int site;     // 站点id,录像机磁盘录像时为
  int device;   // 设备id, 录像机出厂编号
  int camera;   // 镜头id, 通道编号(1-32)
};
//起因
struct issuerInfo_t
{
  int recordType;   // 录像起因类型: 1:报警; 2:录像计划, 3: 手工录像，: 移动侦测录像
  int recordId;     // 录像计划id或报警的id
};
//文字
struct textInfo_t
{
  char host[16];     // 录像主机名(gethostname)，如超过字节取前字节，设备名
  char user[16];     // 录制的用户名，如超过字节取前字节，
  char title[64];    // (v1.3+) camera名, 通道名，如超过字节取前字节，
  char keywords[16]; // (v1.3+) 关键词与备注
};
//实际头部信息结构
struct meta_t
{
  struct magicInfo_t magicInfo;
  struct sumInfo_t sumInfo;
  struct videoInfo_t videoInfo;
  struct audioInfo_t audioInfo;
  struct netInfo_t netInfo;
  struct sourceInfo_t sourceInfo;
  struct issuerInfo_t issuerInfo;
  struct textInfo_t textInfo;
};
//对外头部信息总结构
struct clipHeader_t
{
  union
  {
    struct meta_t hdata;
    char padding[256]; //用来强制clipHeader_t大小为Bytes.
  }u;
};
//15自有时间格式
typedef struct DATETIME
{
  WORD  year;
  WORD  month   :4;
  WORD  day     :5;
  WORD  hour    :5;
  WORD  minute  :6;
  WORD  second  :6;
  WORD  week    :3;
}DATETIME;
//////////////////////////////////////////////////////////////////////////
// rtp mes header
typedef struct RTP_MSG
{
  short subtype;
  short length;
  char  body[1];
}*LPRTP_MSG;

struct rtp_hdr_t
{
  unsigned char tc1;    // 1 byte
  unsigned char tc2;    // 1 byte
  unsigned short seq;    /* sequence number */
  unsigned int ts;    /* timestamp */
  unsigned int ssrc;    /* synchronization source */

  RTP_MSG payload;
};

struct FrameStruct
{
  BOOL     iNeedIFrame;
  
  unsigned char  encType;  //视频帧编码类型
  unsigned char  res;
  unsigned short iType;    //视频帧类型
  int       iLen;
  int       iTimeStamp;
  int       iTick;
  int       iWidth;
  int       iHeight;
  int       isequence;
  int       ichannel;
  int       idevice;
  int       iInterlace;
  BYTE*     pBuf;
};
