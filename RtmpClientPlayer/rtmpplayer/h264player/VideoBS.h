/** $Id: //depot/NVS/v2.1/bsrExplorer/bsrMedia/media/VideoBS.h#7 $ $DateTime: 2009/01/09 18:55:57 $
*  @file VideoBS.h
*  @brief 视频播放
*  @version 1.0.0
*  @since 1.0.0
*  @author dongming<DongM@bstar.com.cn> 
*  @date 2007-07-17    Created it
*/

/************************************************************ 
*  @note
*   Copyright 2005, BeiJing Bluestar Corporation, Limited	
*   ALL RIGHTS RESERVED			
*   Permission is hereby granted to licensees of BeiJing Bluestar, Inc.
*	 products to use or abstract this computer program for the sole purpose
*	 of implementing a product based on BeiJing Bluestar, Inc. products.
*	 No other rights to reproduce, use, or disseminate this computer program,
*	 whether in part or in whole, are granted. BeiJing Bluestar, Inc. 
*	 makes no representation or warranties with respect to the performance of
*	 this computer program, and specifically disclaims any responsibility for 
*	 any damages, special or consequential,connected with the use of this program.
*   For details, see http://www.bstar.com.cn/ 
***********************************************************/

#ifndef _VIDEO_BS_H_
#define _VIDEO_BS_H_
#pragma once

#ifndef USE_D3D
#define USE_D3D
#endif

//#undef USE_D3D

#include <afxmt.h>

#define CIFF_XDIM	352
#define CIFF_YDIM	288

#define ARROW_DIRECT_DOWN 1
#define ARROW_DIRECT_UP   2
#define ARROW_DIRECT_BOTH 3

 const double   PI = 3.1415925;

#ifdef USE_D3D
	#include "D3D/D3D9RenderImpl.h"
#else
	#include <ddraw.h>
#endif

#include <deque>

#include "MediaDefine.h"
#include "SysDef.h"

#include "./bsDecoder/bsDecoder.h"


using namespace std;

#define DECODE_BUFSIZE (1024*576*3/2)

#ifndef POST_SHARP 
#define POST_SHARP
#endif

#undef POST_SHARP

typedef void (CALLBACK *PCBFunGetDecFrameData)(LPDEC_FRAME_S lpFrameData, LPVOID lpUser);
typedef void (CALLBACK *BSDecYUVDataCB)(long nPort, char *pY, char *pU, char *pV, int width, int height, int yPitch, int uPitch, int vPitch, void* pUser);
typedef void (CALLBACK *BSDec17DataCB)(char *pBuf, int iLen, void* pUser);

class CVideoBS
{
public:
  CVideoBS(int iIndex);
  ~CVideoBS(void);
  enum VideoType {VT_NULL, VT_NTSC, VT_PAL, VT_SCEAM};
  enum SurfaceFormat {SF_YUV, SF_RGB};
  enum SurfaceRestorCtrl {RES_NULL, RES_NORMAL, RES_LOST, RES_RESTOR};
  enum DecoderResolution {DECODE_720P , DECODE_1080P};

public:
  // 初始化
  BOOL Init(HWND hWnd, int iDecoder=MDECODE_NULL,DWORD dwFlag=0xf );
  // 播放一帧
  BOOL DisPlay(FrameStruct* pFrame, BOOL bIgnore );
	// 播放最后一帧能解码的数据
	BOOL DisPlayLaseFrame();

  // 获取当前视频的尺寸
  inline void GetFrameSize(DWORD& dwWidth, DWORD& dwHeight);
  // 设置帧的原始大小
  BOOL SetFrameSize(DWORD dwWidth, DWORD dwHeight);
  // 获取当前视频的帧率
  inline int  GetFrameRate();
  // 设置当前视频右上角的状态图标
  void SetStatus(char c, BOOL bFlag);
  // 设置当前视频的标题
  void SetTitle(TitlePos iPos, LPCSTR lpTitle, DWORD dwColor, int iX=0, int iY=0,int nLine = 0);
  // 处理任意地方多行标题显示问题
  void PreTextOut(HDC hdc, CString strTitle, int &iWidth, int &iHeigth);
  // 截图
  BOOL Capture(LPCSTR lpPath, DWORD dwWidth=0, DWORD dwHeight=0);
  BOOL CaptureEx(LPCSTR lpPath, DWORD dwWidth=0, DWORD dwHeight=0);
  // 将一张内存DC保存为bmp
  static BOOL SaveBitmaps(LPCSTR lpSavePath, DWORD dwSaveWidth, DWORD dwSaveHeight, HDC hSrcDC, DWORD dwSrcWidth, DWORD dwSrcHeight);
  // 将一块内存保存为bmp
  static BOOL SaveBitmapsEx(LPCSTR lpSavePath, DWORD dwSaveWidth, DWORD dwSaveHeight, DWORD dwWidth, DWORD dwHeight, PBYTE hSrcBuf, DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwSrcStide);
  // 解一帧
  BOOL DecodeFrame(FrameStruct* pFrame, BYTE* pVideoFrame = NULL);
  // 显示一帧BMP
  BOOL DisPlayBmp(FrameStruct* pFrame);
  // 显示模式是否是YUV
  BOOL IsYuvMode()  { return !m_bRGB;}
  // 设置视频格式 0.yuv, 1.rgb
  void SetSurfaceFormat(SurfaceFormat format);
  // 显示隐藏字幕
  void ShowCaption(BOOL bShow);
  // 获取视频的制式类型
  inline VideoType GetVideoType();

	//回调解码后数据
	void SetCBDecFrameFun(PCBFunGetDecFrameData pCBFun, LPVOID pUserData);

  //添加获取surface区域大小
  inline void GetSurfaceSize( DWORD &width, DWORD &height ); 

  //直接显示YUV数据
  void RenderYV12( char *pY, char *pU, char *pV, int width, int height, int yPitch, int uPitch, int vPitch ); 

	void SetRealDecoder(int iDecoderId); // 设置真实的DecoderID
	int GetRealDecoder();                // 获取真正的DecoderID

	//判断是否成功进行了视频解码
	inline bool VideoIsReady( ) { return m_bDecodeFrame; }
	void SetDelayTime(int delay, DWORD dwTmCur, int iflag);
	void DestroyD3DReneder( );

private:
  //////////////////////////////////////////////////////////////////////////
  // 解码部分

  // 初始化bstar解码库
  BOOL InitBsDecoder();
  // 释放bstar解码库
  void FreeBsDecoder();
  // 显示一帧
  BOOL DisplayFrame(FrameStruct* pFrame, BOOL bDecode = TRUE, BOOL bIgnore = FALSE );
  // 帧数统计
  void StatisticFrameRate();
  // 播放对象是否准备就绪
  BOOL DisplayIsReady();
  // 差值
  void DoInterlace(int interlace);
  // 初始化视频右上的状态图标

  //Limit解码
	void FreeLimitDecoder();
	BOOL InitLimitDecoder();

  //
  static void YUV2RGB(unsigned char *pdata, int stride, int width, int height, unsigned char *pdst);

  //绘制图标
  void DrawTransBitmap( HDC hdcDest,     
	  int nXOriginDest,   // 目标X偏移
	  int nYOriginDest,   // 目标Y偏移
	  int nWidthDest,     // 目标宽度
	  int nHeightDest,    // 目标高度
	  HDC hdcSrc,         // 源DC
	  int nXOriginSrc,    // 源X起点
	  int nYOriginSrc,    // 源Y起点
	  int nWidthSrc,      // 源宽度
	  int nHeightSrc,     // 源高度
	  UINT crTransparent  // 透明色,COLORREF类型
	  );

#ifdef POST_SHARP
  void __cdecl FilterSharpen_SSE(unsigned __int8 *src, unsigned __int8 *dst, int width, int height, int filterStrength);
#endif

  void WaitDelay(int iDecodeTime);
  int   m_iDelayTime;
  DWORD m_iDwTmCur;
  BOOL  m_bRefresh;
  int      m_iDelayflag;
private:
  BOOL                  m_bExit;

 D3D9RenderImpl *m_pD3DRender;

  int                   m_iIndex;
  HWND                  m_hWnd;      // 显示的窗口句柄
  RECT                  m_rcSrc;     // 解出来的视频的原始区域
  RECT                  m_rcDes;     // 需要显示视频的目标区域
  RECT                  m_rectOldDes;

  void*                 m_hDecode;
  BYTE*                 m_pDecodeBuf;
  BOOL                  m_bDecodeOutBufIsOk;
  BYTE*                 m_pDecodeOutBuf;
  BYTE*                 m_pInterlaceBuf;
  int                   m_iDecodeOutBufSize;
  int                   m_iDecodeOutStide;
  int                   m_iDecodeOutWidth;
  int                   m_iDecodeOutHeight;
  BOOL                  m_bRGB;
  SurfaceFormat         m_iSurfaceFormat;

	PCBFunGetDecFrameData m_pCBFun;          //回调解码后的数据
	BSDecYUVDataCB        m_pBSDecYUVDataCB; //回调limit鸟瞰yuv数据
	BSDec17DataCB         m_pBS17DataCB;     //回调17解码前数据
	LPVOID                m_pUserData;

  CCriticalSection      m_csInit;
  CCriticalSection      m_csInitAI;
  CCriticalSection      m_csDisplay;
  CString               m_sErrMes;
  
  SurfaceRestorCtrl     m_iThreeCtrl; // 3态控制量是否恢复表面  

 
  // 图像剪切设置
  BOOL           m_bCutPixel;     // 剪切是否开启
  POINT          m_ptCutLeftTop;
  POINT          m_ptCutRightBottom;

  // 状态统计
  DWORD            m_dwFrameRate;              // 帧率(F/s)
  DWORD            m_dwFramePlayed;            // 已播放帧数
  static const int TIME_PER_STAT = 1;          // 信息的刷新时间
  static const int SAMPLE_OF_STAT = 30;        // 信息的采集样本
  DWORD            m_dwFramePlayedStatSample[SAMPLE_OF_STAT];   // 一次统计时间内，播放的帧数
  int              m_iCurFrameRateStatSample;  // 本次帧率采集样本
  DWORD            m_dwPreFrameRateStatTm;     // 上一次帧率统计时间
  int              m_iNoDataSeconds;           // 统计没有数据的时间


  BOOL	m_bResetDD;
  BOOL  m_bDoDiagnostics;       // 已切换到下个镜头 可以取了
	BOOL  m_bOpenDiagnostics;     //是否开启视频诊断
  long m_nOldWidth;
  long m_nOldHeight;

	int  m_iDecoder;
	bsDecFrameInfo  m_dec_frame;

   //添加HIS DEINTERLACE
   void *m_pHisDeinterlace;
   //DEINTERLACE_PARA_S  m_hisDeInParam;        //Deinterlace parameters

	 //BASE_PIC_INFO m_tagPicInfo;
	 int m_iRealDecoder; // 原来真正的decoder
	 int m_DecoderResolution; // 当前解码分辨
	 unsigned char* m_pRgbBuf;
	 int m_rgbSize;

	 bool m_bDecodeFrame;

	 int  m_videoWidth;
	 int  m_videoHeight;
public:
	 CString m_sTitle[MAX_TITLE_POS][MAX_TITLE_LINE];
	 DWORD   m_dwTitleColor[MAX_TITLE_POS];

};

inline void
CVideoBS::GetFrameSize(DWORD& dwWidth, DWORD& dwHeight)
{
  dwWidth  = m_rcSrc.right - m_rcSrc.left;
  dwHeight = m_rcSrc.bottom - m_rcSrc.top;
}

inline int
CVideoBS::GetFrameRate()
{
  return m_dwFrameRate;
}

inline CVideoBS::VideoType
CVideoBS::GetVideoType()
{
  int iHeight = m_rcSrc.bottom - m_rcSrc.top;
  if (iHeight % 120 == 0)
  {
		if( iHeight == 1080 || iHeight == 720 || iHeight == 240 )
		{
			return VT_PAL;
		}

    return VT_NTSC;
  }
  else if (iHeight % 144 == 0)
  {
    return VT_PAL;
  }
  return VT_NULL;
}

inline void 
CVideoBS::GetSurfaceSize( DWORD &width, DWORD &height )
{
  //赋值
  width  = m_iDecodeOutWidth;
  height = m_iDecodeOutHeight;
}
#endif
