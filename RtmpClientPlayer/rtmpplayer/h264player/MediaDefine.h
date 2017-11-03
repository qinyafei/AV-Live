/** $Id: //depot/NVS/v2.1/bsrExplorer/bsrMedia/media/MediaDef.h#9 $ $DateTime: 2009/01/13 16:49:56 $
 *  @file MediaDef.h
 *  @brief 媒体库的头文件
 *  @version 2.0.0
 *  @since 1.0.0
 *  @author dongming<DongM@bstar.com.cn> 
 *  @date 2007-07-17    Created it
 */

#pragma once
#include <MMSystem.h>

#define WM_VIDEO_STOP       (WM_USER + 1401)     // 停止
#define WM_VIDEO_D6T_NOTICE (WM_USER + 1403)     // BSTP应答消息
#define WM_VIDEO_RTP_MES    (WM_USER + 1408)     // 更新视频信息
#define WM_VIDEO_NO_DATA    (WM_USER + 1418)     // 视频播放无数据
#define WM_VIDEO_PS_CHANGED (WM_USER + 1430)     // 播放状态改变

//////////////////////////////////////////////////////////////////////////
// 视频标题的位置
enum TitlePos
{
  TP_CENTER           = 0,   // 居中
  TP_LEFT_TOP,               // 左上
  TP_RIGHT_TOP,              // 右上
  TP_LEFT_BOTTOM,            // 左下
  TP_RIGHT_BOTTOM,           // 右下
};

//////////////////////////////////////////////////////////////////////////
// 播放状态
enum PlayStatus
{
  PS_NORMAL          = 0,    // 正常播放
  PS_PAUSE,                  // 暂停
  PS_STOP,                   // 停止播放
  PS_FAST_2,                 // 2倍速
  PS_FAST_4,                 // 4倍速
  PS_FAST_8,                 // 8倍速
  PS_FAST_16,                // 16倍速
  PS_SLOW_2,                 // 1/2倍速
  PS_SLOW_4,                 // 1/4倍速
  PS_SLOW_8,                 // 1/8倍速
  PS_SEEK,                   // 定位
  PS_FRAME_NEXT,             // 下一帧
  PS_FRAME_PRE,              // 上一帧
  PS_FRAME_I,                // 仅拨I帧
};

//////////////////////////////////////////////////////////////////////////
// 解码器版本
typedef enum MDecodeType
{
  MDECODE_NULL  = 0,
  MDECODE_,
  MDECODE_BSR   = 100,  // 蓝色之星(旧)
  MDECODE_BSK,          // 蓝色之星(v6.x)
  MDECODE_BSX,
  MDECODE_BSA,
  MDECODE_HK    = 200,  // 海康hc卡
  MDECODE_HK_H  = 201,  // 海康h卡
  MDECODE_HK_9000 = 202,// 海康9000
  MDECODE_HK_9000A = 203,// 品科OEM海康9000
  MDECODE_HK_IPC = 204,// 海康IPC
  MDECODE_HK_8000 = 205,
  MDECODE_HK_793PFWD = 206,
  MDECODE_HK_9000ST  = 207,
  MDECODE_HK_9000AS  = 208,   //对应海康PEAQE-9000A类型

  MDECODE_DH    = 300,  // 大华
  MDECODE_DL    = 400,
  MDECODE_LC    = 500,
  MDECODE_XVID  = 600,
  MDECODE_ZAB   = 700,
  MDECODE_SNC   = 800, //sony 设备
  MDECODE_XLINK = 900, //xilink 设备
  MDECODE_TM    = 1000, //图敏设备
  MDECODE_SHANY =  1100,//shany设备
  MDECODE_PSC =   1200, //松下设备
  MDECODE_ZXW =   1300, //中星微设备
  MDECODE_ZSLC =  1400, //中视里程解码
  MDECODE_ZSV5 =  1401,
  MDECODE_SAM  =  1500,
  MDECODE_WQ   =  1600,
  MDECODE_BK	 =	1700, // 立迈板卡(百科)
  MDECODE_BOS  =  1800, // BOSCH IPC

  MDECODE_SHX  =  1900, // 声讯
  MDECODE_ONVIF = 2000, // 支持 ONVIF 的设备
	MDECODE_JZ   =  2100,//九州
	MDECODE_HB   =  2200,//汉邦

  MDECODE_HW = 2300,//皓维(带有海康解码卡)
  MDECODE_TJTY = 2400, // 同济天跃nvr
	MDECODE_SNE = 2500, //索尼老ipc
	MDECODE_YS = 2600, // 宇视
  MDECODE_TZ = 2700, // 同尊平台ipc
  MDECODE_TDY = 2800, //天地伟业
  MDECODE_TDY_CENTER_FILE = 2801, //天地伟业中心下载的文件

}MDT;

// 厂商
typedef enum MVendorType
{
  MVENDOR_NULL = 0,
  MVENDOR_,
  MVENDOR_BS   = 100,
  MVENDOR_HK   = 200,
  MVENDOR_DH   = 300,
  MVENDOR_DL   = 400,
  MVENDOR_LC   = 500,
  MVENDOR_ZAB  = 600,
  MVENDOR_SNC  = 700,  //sony
  MVENDOR_XLINK = 800, //xilink
  MVENDOR_TM    = 900, //图敏设备
  MVENDOR_SHANY = 1000, //shany设备
  MVENDOR_PSC =   1100,   //松下设备
  MVENDOR_ZXW =   1200,   //中星微设备
  MVENDOR_ZSLC =  1300,
  MVENDOR_SAM  =  1400,
  MVENDOR_WQ   =  1500,
  MVENDOR_BK	 =	1600, // 立迈板卡(百科)
  MVENDOR_BOS  = 1700,
  MVENDOR_SHX  = 1800,   // 声讯
  MVENDOR_ONVIF = 1900, // 支持 ONVIF 的厂商, 统一是这个.
  MVENDOR_JZ = 2000,
  MVENDOR_HB = 2100,  // 汉邦
  MVENDOR_TDY  = 2200,  //天地伟业

}MVT;

//////////////////////////////////////////////////////////////////////////
// 数据源类型
typedef enum MDataSource
{
  MDATA_NULL   = 0,
  MDATA_FILE   = 1,          // 本地文件
  MDATA_TCP    = 2,          // TCP连接
  MDATA_UDP    = 3,          // UDP连接
  MDATA_RTP    = 4,          // RTP连接
  MDATA_ALARM  = 5,          // 报警连接
  MDATA_RTP104 = 6,          // RTP104
  MDATA_PIPE   = 7,          // 压栈方式
  MDATA_RTPLIMIT = 8,        //limit数据

}MDS;
//MDATA_RTP440 = 8,          // RTP440
//////////////////////////////////////////////////////////////////////////
// 帧类型
typedef enum MFrameType
{
  MFT_UNKNOWN = 0,
  MFT_I_FRAME = 1,
  MFT_P_FRAME = 2,
  MFT_B_FRAME = 3,
}MFT;

//////////////////////////////////////////////////////////////////////////
// ResetBuffer Type
typedef enum MResetBufferType
{
  MRBT_ALL = 0,
  MRBT_BUF_VIDEO_SRC = 1,
  MRBT_BUF_AUDIO_SRC = 2,
  MRBT_BUF_VIDEO_RENDER = 3,
  MRBT_BUF_AUDIO_RENDER = 4,
}MRBT;
//////////////////////////////////////////////////////////////////////////
// 状态显示路径
const CString g_StatusBmpName = "bmp\\video_status.bmp";

//////////////////////////////////////////////////////////////////////////
// 数据长度
const static int SIZE_4K       = 1024 * 4;
const static int SIZE_16K      = 1024 * 16;
const static int SIZE_32K      = 1024 * 32;
static const int SIZE_40K      = 1024 * 40;   // 40k
static const int SIZE_64K      = 1024 * 64;   // 64k
static const int SIZE_80K      = 1024 * 80;   // 80k
static const int SIZE_128K     = 1024 * 128;  // 128k
static const int SIZE_256K     = 1024 * 256;  // 256K
static const int SIZE_512K     = 1024 * 512;  // 512K
static const int SIZE_1M       = 1024 * 1024; // 1M
static const int SIZE_FRAME_Q  = 50;          // 原始帧缓冲
static const int SIZE_4M       = 1024*1024*4;
static const int SIZE_5M       = 1025*1024*5;
static const int SIZE_35M      = 1024*1024*35;
static const int SIZE_70M      = 1024*1024*70;

//////////////////////////////////////////////////////////////////////////
// 错误代码
enum MECode
{
  MERROR                = -1,      // 操作失败
  MERROR_NOERROR        = 0,       // 操作成功
  MERROR_PARAM_INVALID,            // 空参数
  MERROR_URL_UNKNOWN,              // URL不可识别
  MERROR_HK             = 100,     // 海康的错误代码
  MERROR_HK_NO_DLL      = 101,     // 海康-没找到hik.dll
  MERROR_HK_OPEN        = 130,     // 海康-开打文件出错
  MERROR_HK_WRITE       = 131,     // 海康-写文件出错
  MERROR_BS             = 200,     // 蓝色之星的错误代码
  MERROR_DH             = 300,     // 大华的错误代码
  MERROR_LC             = 400,     // 郎驰的错误代码
  MERROR_XVID           = 400,     // 郎驰的错误代码
	MERROR_LM							= 500,		 // 立迈板卡(百科)的错误代码
  MERROR_DC_SOCKET      = 10000,   // SOCKET错误
  MERROR_DC_FILE        = 11000,
};

//////////////////////////////////////////////////////////////////////////
// 蓝色星际 Dvr v6.0 数据格式
enum DHv6Type
{
  D6T_UNKNOWN  = 0,
  D6T_AUDIO,
  D6T_VIDEO,
  D6T_AUDIO_LIMIT       = 3,
  D6T_IAEvent           = 4,
  D6T_SPEECH            = 6,
  D6T_NOTICE            = 9,
  D6T_CMD_REQUEST_REPLY = 10,
  D6T_REPORT,
  D6T_AUDIO_G726,
};
enum DHv6Frame
{
  D6V_FRAME_UNKNOWN = 0,
  D6V_FRAME_I,
  D6V_FRAME_B,
  D6V_FRAME_P,
};

#define PB_MSG_END        0x01  // 回放结束,已无录像文件可以回放
#define PB_MSG_FUTURE     0x02  // 未来的回放时间
#define PB_MSG_RECORDERR  0x03  // 错误的录像格式
#define PB_MSG_CTRLERR    0x04  // 错误的控制消息
#define PB_MSG_SEEKOK     0x05  // 用户seek后的返回
#define PB_MSG_IFRAMER    0x06  // 用户设为I帧播放了
#define PB_MSG_PLAYBACK   0x07  // 用户设为常规播放了
#define PB_MSG_RECEDE     0x08  // 用户设为退放了

//////////////////////////////////////////////////////////////////////////
// 状态统计
struct tagStatus
{
  PlayStatus iPS;                  // 当前播放状态
  DWORD dwFrameRate;               // 帧率
  DWORD dwBitRate;                 // 码率
  DWORD dwElapsedTime;             // 逝去时间
  DWORD dwTotalTime;               // 总时间
  DWORD dwWidth;                   // 帧原始宽度
  DWORD dwHeight;                  // 帧原始高度
};

// RTP_MSG.subtype = 1的结构
typedef struct RTP_MSG_SUB_01
{
  char          name[32];
  int           label;
  int           camera;
  int           site;
  unsigned long ipAddr;
  char          path[64];
  char          deviceType[16];    // mysql,system table 定义.
}*LPRTP_MSG_SUB_01;

// RTP_MSG.subtype = 255的结构
typedef struct RTP_MSG_SUB_FF
{
  int camera; // 镜头编号
  int issure; // 窗口号
  int type;   // 播放类型
}close_msg_t, *LPRTP_MSG_SUB_FF;


//回调解码后数据结构体
typedef struct _DEC_FRAME_S
{
	unsigned char* pY;
	unsigned char* pU;
	unsigned char* pV;
	unsigned int  uWidth;
	unsigned int  uHeight;
	unsigned int  uYStride;
	unsigned int  uUVStride;
}DEC_FRAME_S, *LPDEC_FRAME_S;
typedef struct portAgentReceiveInfo
{
	int port;
	int ipAddr;
	char res[32];
}AgentRecvInfo;
