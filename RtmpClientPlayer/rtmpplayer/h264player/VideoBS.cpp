*/* $Id: //depot/NVS/v2.1/bsrExplorer/bsrMedia/media/VideoBS.cpp#20 $ $DateTime: 2009/01/09 18:55:57 $
 *  @file VideoBS.cpp
 *  @brief 视频播放
 *  @version 1.0.0
 *  @since 1.0.0
 *  @author dongming<dongm@bstar.com.cn> 
 *  @date 2007-07-17    Created it
 */

 /************************************************************
 *  @note
 *   Copyright 2005, BeiJing Bluestar Corporation, Limited  
 *   ALL RIGHTS RESERVED      
 *   Permission is hereby granted to licensees of BeiJing Bluestar, Inc.
 *   products to use or abstract this computer program for the sole purpose
 *   of implementing a product based on BeiJing Bluestar, Inc. products.
 *   No other rights to reproduce, use, or disseminate this computer program,
 *   whether in part or in whole, are granted. BeiJing Bluestar, Inc. 
 *   makes no representation or warranties with respect to the performance of
 *   this computer program, and specifically disclaims any responsibility for 
 *   any damages, special or consequential,connected with the use of this program.
 *   For details, see http://www.bstar.com.cn/ 
 ***********************************************************/


#include "StdAfx.h"
#include "VideoBS.h"

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")
#include <atlimage.h>
#include <math.h>
#include "SysCom.h"
#include <Mmsystem.h>
 //#include "vd.h"

#define FFMPEG

#ifdef FFMPEG
#pragma comment(lib, "ffmpegDecoder.lib")
#else
#pragma comment(lib, "media/limit/hi_h264dec_w_nologo_nomark.lib")
#endif

 //#pragma comment(lib, "media/codecs.lib")

 //#pragma comment(lib, "DllDeinterlace.lib")
#ifdef _DEBUG
#pragma comment(lib, "./Debug/bsDecoder.lib" )
#else
#endif


 //ff_Hi264DecCreate
 // 调试视频诊断1：验证yuv输入的是否正确
 // #define _DEBUG_DIAGNOSTICS_1_

#if 0
#ifdef _DEBUG
#undef THIS_FILE
 static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif


CVideoBS::CVideoBS(int iIndex)
{
	m_bExit = FALSE;

	m_pD3DRender = NULL;

	m_iIndex = iIndex;
	m_hWnd = NULL;
	memset(&m_rcSrc, 0, sizeof(RECT));
	memset(&m_rcDes, 0, sizeof(RECT));
	memset(&m_rectOldDes, 0, sizeof(RECT));

	m_hDecode = NULL;
	m_pDecodeBuf = NULL;
	m_pDecodeOutBuf = NULL;
	m_bDecodeOutBufIsOk = FALSE;
	m_pInterlaceBuf = NULL;
	m_iDecodeOutBufSize = 0;
	m_iDecodeOutStide = 0;
	m_iDecodeOutWidth = 0;
	m_iDecodeOutHeight = 0;
	m_bRGB = FALSE;
	m_iSurfaceFormat = SF_YUV;

	m_pCBFun = NULL;

	memset(m_dwFramePlayedStatSample, 0, sizeof(DWORD) * SAMPLE_OF_STAT);
	m_iCurFrameRateStatSample = 0;
	m_dwFrameRate = 0;
	m_dwFramePlayed = 0;

	m_bCutPixel = FALSE;
	memset(&m_ptCutLeftTop, 0, sizeof(POINT));
	memset(&m_ptCutRightBottom, 0, sizeof(POINT));

	m_iThreeCtrl = RES_NORMAL;
	m_bResetDD = FALSE;
	m_bDoDiagnostics = FALSE;
	m_nOldWidth = 0;
	m_nOldHeight = 0;
	//m_iDecoder = CMediaBS::GetDecoderType();

	m_pUserData = NULL;

	m_pBSDecYUVDataCB = NULL;

	m_iRealDecoder = -1;

	m_DecoderResolution = DECODE_720P;

	m_rgbSize = 1024*1024*3;
	m_pRgbBuf = NULL;
	memset(&m_dec_frame, 0, sizeof(bsDecFrameInfo));

	m_bDecodeFrame = false;
	m_iDelayTime = 0;
	m_bRefresh = FALSE;
	m_iDwTmCur = 0;
	m_iDelayflag = 0;


	m_videoWidth  = 0;
	m_videoHeight = 0;

	for( int i = 0; i < MAX_TITLE_POS; i++ )
	{
		m_dwTitleColor[i] = 0xff0000ff; //默认为蓝色
	}

	memset( &m_dec_frame, 0, sizeof(m_dec_frame) );
	for (int i = 0;i<MAX_TITLE_POS; i++)
	{
		for (int j = 0;j< MAX_TITLE_LINE;j++)
		{
			m_sTitle[i][j] == _T("");
		}
	}

}
CVideoBS::~CVideoBS(void)
{
	m_csInit.Lock( );
	m_bExit = TRUE;

#ifndef USE_D3D
	SAFE_RELEASE(m_pPSur);
	SAFE_RELEASE(m_pBBuf);
	SAFE_RELEASE(m_pOPla);
	SAFE_RELEASE(m_pDD7);
#endif

	SAFE_DELETE_ARRAY(m_pDecodeBuf);	
	SAFE_DELETE_ARRAY(m_pDecodeOutBuf);

	if (m_iDecoder == MDECODE_BSX)
	{
		FreeLimitDecoder();
	}
	else
	{
		FreeBsDecoder();
	}

	SAFE_DELETE_ARRAY(m_pInterlaceBuf);


	m_csInit.Unlock();
	if(m_pRgbBuf != NULL)
	{
		delete[] m_pRgbBuf;
	}	

	m_pRgbBuf = NULL;

	if( m_pD3DRender != NULL )
	{
		delete m_pD3DRender;
		m_pD3DRender = NULL;
	}
}

void CVideoBS::DestroyD3DReneder( )
{
	if( m_csDisplay.Lock() )
	{
		if( m_pD3DRender != NULL )
		{
			//m_pD3DRender->DiscardResources();
			delete m_pD3DRender;
			m_pD3DRender = NULL;
		}
		m_csDisplay.Unlock( );
	}
}


BOOL
CVideoBS::Init(HWND hWnd, int iDecoder,DWORD dwFlag/* =0 */)
{
	BOOL bResult = FALSE;
	BOOL bInit = FALSE;

	m_iDecoder = iDecoder;

	if (m_csInit.Lock())
	{
		while (TRUE)
		{
			if (hWnd == NULL)
			{
				m_sErrMes.Format("hWnd=NULL.");
				break;
			}

			if( m_hWnd == hWnd && m_pD3DRender != NULL )
			{
				break;
			}

			if( m_pD3DRender != NULL )
			{
				//销毁之前的对象
				delete m_pD3DRender;
				m_pD3DRender = NULL;
			}

			m_pD3DRender = new D3D9RenderImpl( );

			m_pD3DRender->Initialize( hWnd, 0, NULL );

			for( int i = 0; i < MAX_TITLE_POS; i++ )
			{
				for( int j = 0; j < MAX_TITLE_LINE; j++ )
				{
					//m_pD3DRender->SetTitle( (TitlePos)i, m_sTitle[i][j], m_dwTitleColor[i], j );

				}
			}


			if (dwFlag & 0x8)
			{
				//m_iDecoder = iDecoder;
				if (iDecoder == MDECODE_BSX)
				{
					bInit = InitLimitDecoder();
					if (!bInit)
					{
						m_sErrMes.Format("InitBsDecoder failed.");
						break;
					}
				}
				else
				{
					bInit = InitBsDecoder();
					if (!bInit)
					{
						m_sErrMes.Format("InitBsDecoder failed.");
						break;
					}
				}
			}
			m_hWnd = hWnd;
			bResult = TRUE;
			break;
		}
		m_csInit.Unlock();
	}

	if (!bResult)
	{
		Log(3, "[Video%02d] Init() failed, %s\n", m_iIndex + 1, m_sErrMes);
	}
	m_dwPreFrameRateStatTm = timeGetTime();

	return bResult;
}
BOOL 
CVideoBS::DisPlayLaseFrame()
{
	BOOL bResult = FALSE;

	if( m_csDisplay.Lock() )
	{
		do
		{
			if( m_pD3DRender == NULL )
			{
				break;
			}

			//bResult = DisplayFrame(&m_lastFrameInfo, FALSE);
		}while( false );

		m_csDisplay.Unlock();
	}

	return bResult;
}

BOOL
CVideoBS::DisPlay(FrameStruct* pFrame, BOOL bIgnore )
{   

	BOOL bResult = FALSE;
	if (m_csDisplay.Lock())
	{
		if (pFrame != NULL && DisplayIsReady() && m_pD3DRender != NULL )
		{
			//m_iSequence = pFrame->isequence;
			if (IsYuvMode())
			{
				bResult = DisplayFrame(pFrame, TRUE, bIgnore );
			}
			else
			{
				TRACE("不支持的显示方式\r\n");
			}
			//TRACE("isequence = %d\n",pFrame->isequence);

			//if (m_iIADecoder == MDECODE_BSA)
			{
				//ReDrawIAInfo(); //暂时注释，需要修改
			}
		}
		m_csDisplay.Unlock();
	}

	return bResult;
}

#ifndef USE_D3D

BOOL
CVideoBS::InitPrimarySurface(HWND hWnd)
{
	SAFE_RELEASE(m_pPSur);
	if (m_pDD7 != NULL)
	{
		m_pDD7->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
	}
	SAFE_RELEASE(m_pDD7);

	BOOL bResult = FALSE;
	HRESULT hr;
	LPDIRECTDRAWCLIPPER pClipper = NULL;
	while (TRUE)
	{
		int index = GetMonitorCurEx(hWnd);
		index++;
		GUID guid = GetMonitorGuid(index);
		// 创建DirectDraw7对象，以前第一个参数(监视器GUID)为空时导致双屏显示出问题，添加上监视器的GUID可使问题得到解决。
		hr = DirectDrawCreateEx(&guid, (VOID **)&m_pDD7, IID_IDirectDraw7, NULL);
		if (FAILED(hr))
		{
			break;
		}

		// 设置协作等级
		hr = m_pDD7->SetCooperativeLevel(hWnd, DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES);
		if (FAILED(hr))
		{
			break;
		}

		// 创建主表面
		DDSURFACEDESC2 ddsd;
		memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
		ddsd.dwSize            = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags           = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE;
		hr = m_pDD7->CreateSurface(&ddsd, &m_pPSur, NULL);
		if (FAILED(hr))
		{
			break;
		}

		// 创建剪切器
		hr = m_pDD7->CreateClipper(0, &pClipper, NULL);
		if (FAILED(hr))
		{
			break;
		}
		hr = pClipper->SetHWnd(0, hWnd);
		if (FAILED(hr))
		{
			break;
		}
		hr = m_pPSur->SetClipper(pClipper);
		if (FAILED(hr))
		{
			break;
		}
		pClipper->Release();

		bResult = TRUE;
		break;
	}

	if (!bResult)
	{
		SAFE_RELEASE(pClipper);
		SAFE_RELEASE(m_pDD7);
	}

	return bResult;
}
#endif

#define _RGB16BIT565(r,g,b) ((b & 31) + ((g & 63) << 5) + ((r & 31) << 11)) // this builds a 16 bit color value in 5.5.5 format (1-bit alpha mode)
#define _RGB16BIT555(r,g,b) ((b & 31) + ((g & 31) << 5) + ((r & 31) << 10))
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))


#ifndef USE_D3D

BOOL
CVideoBS::InitBackBuffer(HWND hWnd)
{
	GetClientRect(hWnd, &m_rcDes);
	GetClientRect(hWnd, &m_rectOldDes);
	ClientToScreen(hWnd, (POINT*)&m_rectOldDes);
	ClientToScreen(hWnd, (POINT*)&m_rectOldDes+1);

	int cx = m_rcDes.right - m_rcDes.left;
	int cy = m_rcDes.bottom - m_rcDes.top;
	if (cx <= 0 || cy <= 0 || m_rcDes.right < 0 || m_rcDes.top < 0 || m_rcDes.left < 0 || m_rcDes.bottom < 0)
	{
		return FALSE;
	}

	// 创建后表面
	if (!CreateBackSurface(cx, cy, &m_pBBuf))
	{
		return FALSE;
	}
	// 	// 创建状态(字幕)表面
	// 	if (!CreateBackSurface(cx, cy, &m_pBBStatus))
	// 	{
	// 		return FALSE;
	// 	}
	// 	// 创建状态(字幕)表面2
	// 	if (!CreateBackSurface(cx, cy, &m_pBBIAInfo))
	// 	{
	// 		return FALSE;
	// 	}
	// 	DDCOLORKEY ddck, ddck2;
	// 
	// 
	// 	// 测试RGB模式
	// 	DDPIXELFORMAT ddpixel;
	// 	memset(&ddpixel, 0, sizeof(ddpixel));
	// 	ddpixel.dwSize = sizeof(ddpixel);
	// 	m_pBBStatus->GetPixelFormat(&ddpixel);
	// 	if (ddpixel.dwFlags & DDPF_RGB) 
	// 	{     
	// 		switch(ddpixel.dwRGBBitCount)  
	// 		{            
	// 		case 15: // must be 5.5.5 mode    
	// 			ddck.dwColorSpaceLowValue = _RGB16BIT555(255,0,255);
	// 			ddck.dwColorSpaceHighValue = _RGB16BIT555(255,0,255);
	// 			break;         
	// 		case 16: // must be 5.6.5 mode     
	// 			ddck.dwColorSpaceLowValue = _RGB16BIT565(255,0,255);
	// 			ddck.dwColorSpaceHighValue = _RGB16BIT565(255,0,255);
	// 			break;         
	// 		case 24: // must be 8.8.8 mode      
	// 			break;     
	// 		case 32: // must be alpha(8).8.8.8 mode     
	// 			ddck.dwColorSpaceLowValue = _RGB32BIT(0,255,0,255);
	// 			ddck.dwColorSpaceHighValue = _RGB32BIT(255,255,0,255);
	// 			break;      
	// 		default:     
	// 			break;        
	// 		}
	// 	}
	// 
	// 	ddck2.dwColorSpaceLowValue = RGB(255, 255, 255);
	// 	ddck2.dwColorSpaceHighValue = RGB(255, 255, 255);
	// 	m_pBBStatus->SetColorKey(DDCKEY_SRCBLT, &ddck);
	// 	m_pBBIAInfo->SetColorKey(DDCKEY_SRCBLT, &ddck2);

	// 创建字幕字体

	if(!m_bCreateFtCaption)
	{
		m_ftCaption.Detach();
		m_ftCaption.CreateFontIndirect(&CMediaEx::s_ftTitle);
		m_bCreateFtCaption = TRUE;
	}

	return TRUE;
}

BOOL
CVideoBS::CreateBackSurface(DWORD dwWidht, DWORD dwHeight, LPDIRECTDRAWSURFACE7* ppBackSurface)
{
	SAFE_RELEASE(*ppBackSurface);
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize         = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT; 
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY; 
	ddsd.dwWidth = dwWidht;
	ddsd.dwHeight = dwHeight;
	HRESULT hr = m_pDD7->CreateSurface(&ddsd, ppBackSurface, NULL);
	if (FAILED(hr))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL
CVideoBS::InitOffsreenPlain(DWORD dwWidth, DWORD dwHeight)
{
	// rgb
	SAFE_RELEASE(m_pOPla);
	DDSURFACEDESC2 ddsd;
	if (m_iSurfaceFormat == 1)
	{
		memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
		ddsd.dwSize         = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.dwWidth        = dwWidth;
		ddsd.dwHeight       = dwHeight;
		HRESULT hr = m_pDD7->CreateSurface(&ddsd, &m_pOPla, NULL);
		if (FAILED(hr))
		{
			return FALSE;
		}
		m_bRGB = TRUE;
		m_bDecodeOutBufIsOk = TRUE;
	}
	else
	{
		memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
		ddsd.dwSize         = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.dwWidth        = dwWidth;
		ddsd.dwHeight       = dwHeight;
		ddsd.ddpfPixelFormat.dwSize   = sizeof(DDPIXELFORMAT);
		ddsd.ddpfPixelFormat.dwFlags  = DDPF_FOURCC | DDPF_YUV ;
		ddsd.ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y','V','1','2');
		ddsd.ddpfPixelFormat.dwYUVBitCount = 8;
		m_bRGB = FALSE;
		HRESULT hr = m_pDD7->CreateSurface(&ddsd, &m_pOPla, NULL);
		if (FAILED(hr))
		{
			SAFE_RELEASE(m_pOPla);
			memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
			ddsd.dwSize         = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			ddsd.dwWidth        = dwWidth;
			ddsd.dwHeight       = dwHeight;
			HRESULT hr = m_pDD7->CreateSurface(&ddsd, &m_pOPla, NULL);
			if (FAILED(hr))
			{
				return FALSE;
			}
			m_bRGB = TRUE;
		}
	}

	m_pOPla->GetSurfaceDesc(&ddsd);
	m_iDecodeOutWidth = ddsd.dwWidth;
	m_iDecodeOutHeight = ddsd.dwHeight;
	m_iDecodeOutStide = ddsd.lPitch;
	SAFE_DELETE_ARRAY(m_pDecodeOutBuf);
	m_iDecodeOutBufSize = IsYuvMode() ? (m_iDecodeOutStide*m_iDecodeOutHeight*3/2) : (m_iDecodeOutStide*m_iDecodeOutHeight);
	// if( m_iDecoder != MDECODE_BSX )
	{
		m_pDecodeOutBuf = new BYTE[m_iDecodeOutBufSize];
		m_bDecodeOutBufIsOk = TRUE;
	}
	return TRUE;
}

#endif

void
CVideoBS::FreeBsDecoder()
{
	if (m_hDecode != NULL)
	{
		//Bs_Mpeg4_dec_free(m_hDecode);
		m_hDecode = NULL;
	}
}

BOOL
CVideoBS::InitBsDecoder()
{
	FreeBsDecoder();

	SAFE_DELETE_ARRAY(m_pDecodeBuf);
	m_pDecodeBuf = new BYTE[4*1024*1000];
#if 0
	m_hDecode = Bs_Mpeg4_dec_init(CIFF_XDIM * 2
		, CIFF_YDIM * 2
		, m_pDecodeBuf
		, CIFF_XDIM * CIFF_YDIM * 4
		, CIFF_XDIM * CIFF_YDIM * 5
		, CIFF_XDIM * 2
		, CIFF_XDIM);
	if (m_hDecode == NULL)
	{
		return FALSE;
	}
#endif

	return TRUE;
}

void
CVideoBS::FreeLimitDecoder()
{
	if (m_hDecode != NULL)
	{
		bsDecDestroy(m_hDecode);
		m_hDecode = NULL;
	}
}

#define VIDEO_MBSIZE 16
BOOL
CVideoBS::InitLimitDecoder()
{
	FreeLimitDecoder();

	m_hDecode = bsDecCreate( );
	if (m_hDecode == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

#define SAT(x) ( (x)<0 ? 0 : (x)>255 ? 255 : (x) )

#define TO_RGB24(D,Y) \
	D[0] = SAT(Y[0]+Bo); D[1] = SAT(Y[0]-Go); D[2] = SAT(Y[0]+Ro); \
	D[3] = SAT(Y[1]+Bo); D[4] = SAT(Y[1]-Go); D[5] = SAT(Y[1]+Ro); \

void CVideoBS::YUV2RGB(unsigned char *pdata, int stride, int width, int height, unsigned char *pdst)
{
	const unsigned char *Y, *U, *V;
	unsigned char *RGB;
	int x, y;

	if (stride < 0) {
		Y = pdata - stride * (height - 1);
		U = pdata - (stride / 2) * (height * 5 / 2 - 1);
		V = pdata - (stride / 2) * (height * 3 - 1);
	} else {
		Y = pdata;
		U = pdata + stride * height;
		V = pdata + stride * height * 5 / 4;
	}
	RGB = pdst;
	for(y=0; y<height/2; y++)
	{
		unsigned char *pDst1 = RGB;
		unsigned char *pDst2 = RGB+width*3;
		const unsigned char *pYSrc1 = Y;
		const unsigned char *pYSrc2 = Y + stride;
		for(x=0; x<width/2; x++)
		{
			const int Uo = U[x]-128;
			const int Vo = V[x]-128;
			const int Ro = (91181*Uo)>>16;
			const int Go = (22553*Vo + 46801*Uo)>>16;
			const int Bo = (116129*Vo)>>16;

			TO_RGB24(pDst1,   pYSrc1)
				pDst1 +=6;
			pYSrc1+=2;
			TO_RGB24(pDst2,   pYSrc2)
				pDst2 +=6;
			pYSrc2+=2;
		}
		RGB  += width*6;
		Y    += 2*stride;
		U    += stride/2;
		V    += stride/2;
	}
}

BOOL
CVideoBS::SaveBitmapsEx(LPCSTR lpSavePath, DWORD dwSaveWidth, DWORD dwSaveHeight, DWORD dwWidth, DWORD dwHeight
												, PBYTE hSrcBuf, DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwSrcStide)
{
	BOOL bResult = TRUE;
	unsigned char  *rgb;
	rgb = (unsigned char *)malloc(dwSrcWidth * dwSrcHeight * 3);

	YUV2RGB(hSrcBuf, -dwSrcStide, dwSrcWidth, dwSrcHeight, rgb);

	int linebytes = dwSrcWidth * dwSrcHeight * 3;
	BITMAPINFOHEADER bmp_info,bmp_info1;
	bmp_info.biSize			= sizeof(BITMAPINFOHEADER);
	bmp_info.biWidth		= dwSrcWidth;
	bmp_info.biHeight		= dwSrcHeight;
	bmp_info.biPlanes		= 1;
	bmp_info.biBitCount		= 24;
	bmp_info.biCompression	= 0;
	bmp_info.biSizeImage	= linebytes;
	bmp_info.biXPelsPerMeter= 0;
	bmp_info.biYPelsPerMeter= 0;
	bmp_info.biClrImportant	= 0;
	bmp_info.biClrUsed		= 0;

	bmp_info1.biWidth = dwSaveWidth;
	bmp_info1.biHeight = dwSaveHeight;
	DWORD dwSize1 = bmp_info1.biWidth * 3 * bmp_info1.biHeight;   
	BYTE *pBuf1;
	pBuf1 = (PBYTE)malloc(dwSize1);   
	BYTE  *pSrc, *pDest;   
	for(int i = 0; i < bmp_info1.biHeight; i++)   
	{   
		pSrc = rgb + bmp_info.biWidth* 3 * (i+ dwSrcHeight - dwSaveHeight);   
		pDest = pBuf1 + bmp_info1.biWidth * 3 * i;   
		memcpy(pDest, pSrc, bmp_info1.biWidth * 3);   
	}   

	bmp_info.biWidth		= dwSaveWidth;
	bmp_info.biHeight		= dwSaveHeight;
	bmp_info.biSizeImage	= dwSaveWidth * dwSaveHeight * 3;        

	BITMAPINFO BitmapInfo;
	ZeroMemory(&BitmapInfo, sizeof BITMAPINFO);
	memcpy(&BitmapInfo, &bmp_info, sizeof BITMAPINFOHEADER);
	CClientDC dc(NULL);
	HBITMAP hBitmap = CreateDIBitmap(dc.GetSafeHdc(), &bmp_info, CBM_INIT, pBuf1, &BitmapInfo, DIB_RGB_COLORS);

	CBitmap srcBitmap;
	srcBitmap.Attach(hBitmap);
	CDC hmemDC;
	hmemDC.CreateCompatibleDC(NULL);
	hmemDC.SelectObject(&srcBitmap);


	CDC hdesDC;
	hdesDC.CreateCompatibleDC(NULL);
	CBitmap desBitmap;
	desBitmap.CreateCompatibleBitmap(&hmemDC, dwWidth, dwHeight);
	hdesDC.SelectObject(&desBitmap);
	hdesDC.SetStretchBltMode(COLORONCOLOR);

	hdesDC.StretchBlt(0, 0, dwWidth, dwHeight, &hmemDC, 0, 0, bmp_info.biWidth, bmp_info.biHeight, SRCCOPY);

	CImage image;
	image.Attach((HBITMAP)desBitmap.m_hObject);
	//image.Load(lpSavePath);
	if (S_OK != (image.Save(lpSavePath)))
	{
		bResult = FALSE;;
	}
	DeleteObject(desBitmap);
	DeleteDC(hdesDC);
	DeleteDC(hmemDC);
	DeleteObject(srcBitmap);
	DeleteObject(hBitmap);
	desBitmap.Detach();
	hdesDC.Detach();
	hmemDC.Detach();
	srcBitmap.Detach();

	free(rgb);
	free(pBuf1);
	return bResult;
}

BOOL
CVideoBS::SaveBitmaps(LPCSTR lpSavePath, DWORD dwSaveWidth, DWORD dwSaveHeight, HDC hSrcDC, DWORD dwSrcWidth, DWORD dwSrcHeight)
{
	BOOL bRet = FALSE;
	CDC dc, dc2;
	dc.CreateCompatibleDC(NULL);
	dc2.Attach(hSrcDC);
	CBitmap cbm, *pOldBm;
	cbm.CreateCompatibleBitmap(&dc2, dwSaveWidth, dwSaveHeight);
	pOldBm = dc.SelectObject(&cbm);
	dc.SetStretchBltMode(COLORONCOLOR);
	dc.StretchBlt(0, 0, dwSaveWidth, dwSaveHeight, &dc2, 0, 0, dwSrcWidth, dwSrcHeight, SRCCOPY);
	dc.SelectObject(pOldBm);

	CImage image;
	image.Attach((HBITMAP)cbm.m_hObject);
	HRESULT hr = image.Save(lpSavePath);
	if (S_OK == hr/*(image.Save(lpSavePath))*/)
	{
		bRet = TRUE;
	}
	dc2.Detach();
	return bRet;
}

BOOL
CVideoBS::DisplayIsReady()
{

	return TRUE;
}



BOOL
CVideoBS::DisplayFrame(FrameStruct* pFrame,BOOL bDecode /*= TRUE*/, BOOL bIgnore /*= FALSE*/)
{
	DWORD dwDecodeTime = 0, dwPlayTime = 0;
	DWORD dwDecodeStart = 0,  dwDecodeEnd = 0;
	DWORD dwPlayStart = 0, dwPlayEnd = 0;
	StatisticFrameRate();
	//memset(&m_dec_frame, 0, sizeof(hiH264_DEC_FRAME_S));
	BOOL bResult = FALSE;
	BOOL bWindowsChanged = FALSE;
	HRESULT hr = FALSE;

	BYTE *videoPt = NULL;
	int videoPitch = 0;

	if( !m_csInit.Lock( ))
	{
		OutputDebugString("m_csInit.Lock() fail \n");
		return FALSE;
	}

	do 
	{
		if (!DisplayIsReady())
		{
			m_sErrMes.Format("1surface=NULL.\n");
			break;
		}

		if (m_iThreeCtrl == RES_LOST)
		{
			m_iThreeCtrl = RES_RESTOR;
		}

		dwDecodeStart = ::timeGetTime();
		//如果是海思码流，先解码后copy
		if (m_iDecoder == MDECODE_BSX && bDecode)
		{
			DecodeFrame(pFrame, NULL);
		}

		if( bIgnore )
		{
			m_csInit.Unlock( );
			return TRUE;
		}

		m_csInit.Unlock( );

		dwDecodeEnd = ::timeGetTime();

		this->WaitDelay(dwDecodeEnd - dwDecodeStart);
		dwPlayStart = timeGetTime();

		if( !m_csInit.Lock() )
		{
			break;
		}


		if (m_iDecoder == MDECODE_BSX)
		{
			if( m_dec_frame.pY == NULL || m_dec_frame.pU == NULL || m_dec_frame.pV == NULL  )
			{
				break;
			}

			if(pFrame != NULL)
			{
				//对于LC的场编码D1数据，需要做deInterlace，判断方法如下。由于录像机无法区分场编码D1数据和帧编码D1数据。
				//如果BS数据支持D1以上数据解码显示功能，此处代码需要调整。
				//场编码判定 如果海思 分辨率: 704*576 640*480 并且解码后的高度与解码前的不等（变为一半）

				if((pFrame->iWidth == 704 && pFrame->iHeight == 576 &&pFrame->iHeight == m_dec_frame.uHeight*2&& m_dec_frame.uWidth>352)   || 
					(pFrame->iWidth == 640 && pFrame->iHeight == 480 &&pFrame->iHeight == m_dec_frame.uHeight*2&& m_dec_frame.uWidth>320) ||
					(m_dec_frame.uHeight == 288 && m_dec_frame.uWidth == 720))
				{
#if 0
					if( m_pHisDeinterlace != NULL )
					{
						if( m_hisDeInParam.iFieldWidth != m_dec_frame.uWidth || m_hisDeInParam.iFieldHeight != m_dec_frame.uHeight || m_hisDeInParam.iDstYPitch != videoPitch )
						{
							HI_ReleaseDeinterlace( m_pHisDeinterlace );
							m_pHisDeinterlace = NULL;
						}
					}
#endif


					hr = m_pD3DRender->GetVideoPt( m_dec_frame.uWidth, m_dec_frame.uHeight*2, &videoPt, videoPitch );
					if( hr != S_OK )
					{
						break;
					}

#if 0
					if( m_pHisDeinterlace == NULL )
					{
						m_hisDeInParam.iFieldWidth   = m_dec_frame.uWidth;
						m_hisDeInParam.iFieldHeight  = m_dec_frame.uHeight;
						m_hisDeInParam.iSrcYPitch    = m_dec_frame.uYStride * 2;
						m_hisDeInParam.iSrcUVPitch   = m_dec_frame.uUVStride * 2;
						m_hisDeInParam.iDstYPitch    = videoPitch;
						m_hisDeInParam.iDstUVPitch   = videoPitch/2;

						HI_InitDeinterlace( &m_pHisDeinterlace, m_hisDeInParam );
					}

					if( m_pHisDeinterlace != NULL )
					{
						DEINTERLACE_FRAME_S stDstFrame;
						stDstFrame.pszY = (unsigned char*)videoPt;
						stDstFrame.pszV = stDstFrame.pszY + videoPitch * m_dec_frame.uHeight * 2;
						stDstFrame.pszU = stDstFrame.pszY + videoPitch * m_dec_frame.uHeight * 2 * 5 / 4;

						// Deinterlace
						HI_Deinterlace( m_pHisDeinterlace, stDstFrame, m_dec_frame.pY, m_dec_frame.pU, m_dec_frame.pV, PIC_INTERLACED_ODD );

						HI_Deinterlace( m_pHisDeinterlace, stDstFrame, 
							m_dec_frame.pY + m_dec_frame.uWidth, 
							m_dec_frame.pU + m_dec_frame.uUVStride, 
							m_dec_frame.pV + m_dec_frame.uUVStride, PIC_INTERLACED_EVEN );
					}
#endif

					m_pD3DRender->ReleaseVideoPt();
				}
				else
				{
					//不需要deinterlace的数据直接显示
					m_pD3DRender->FillData( m_dec_frame.pY, m_dec_frame.pV, m_dec_frame.pU,
						m_dec_frame.uWidth, m_dec_frame.uHeight, 
						m_dec_frame.uYStride, m_dec_frame.uUVStride, m_dec_frame.uUVStride );
				}
			}

		}else //如果是mpeg4解码
		{
			if(bDecode)
			{
				//若解码
				if( m_pDecodeOutBuf == NULL )
				{ 
					m_iDecodeOutBufSize = CIFF_XDIM*CIFF_YDIM*3;
					m_pDecodeOutBuf     = new BYTE[m_iDecodeOutBufSize];
					m_iDecodeOutHeight = CIFF_YDIM;
					m_iDecodeOutStide  = CIFF_XDIM*2;

				}else if( m_iDecodeOutBufSize != CIFF_XDIM*CIFF_YDIM*3 )
				{
					delete []m_pDecodeOutBuf;

					m_pDecodeOutBuf = NULL;
					m_iDecodeOutBufSize =  CIFF_XDIM*CIFF_YDIM*3 ;
					m_pDecodeOutBuf     = new BYTE[m_iDecodeOutBufSize];
				}

				if (!m_bOpenDiagnostics && pFrame != NULL)
				{
					hr = m_pD3DRender->GetVideoPt( CIFF_XDIM*2, CIFF_YDIM, &videoPt, videoPitch );
					if( hr != S_OK )
					{
						m_iDecodeOutHeight = CIFF_YDIM;
						m_iDecodeOutStide  = CIFF_XDIM*2;
						DecodeFrame(pFrame, NULL);
						break;
					}

					m_iDecodeOutHeight =  CIFF_YDIM;
					m_iDecodeOutStide  =  videoPitch;

					DecodeFrame(pFrame, (BYTE*)(videoPt));  //不启用视频诊断 直接解码到表面
					m_pD3DRender->ReleaseVideoPt( m_rcSrc, true );
				}
				else
				{
					if (pFrame != NULL)  
					{
						DecodeFrame(pFrame, NULL);

						BYTE *pY = m_pDecodeOutBuf;
						BYTE *pV = m_pDecodeOutBuf + m_iDecodeOutHeight*m_iDecodeOutStide;
						BYTE *pU = pV + m_iDecodeOutHeight*m_iDecodeOutStide;

						m_pD3DRender->FillData( pY, pV, pU, pFrame->iWidth, pFrame->iHeight, 
							pFrame->iWidth, pFrame->iWidth/2, pFrame->iWidth/2 );

					}
				}
			}else if( m_pDecodeOutBuf != NULL )
			{	
				//显示上一帧视频
				if( pFrame != NULL && pFrame->iWidth != 0 )
				{
					BYTE *pY = m_pDecodeOutBuf;
					BYTE *pV = m_pDecodeOutBuf + pFrame->iWidth*pFrame->iHeight;
					BYTE *pU = pV + pFrame->iWidth*pFrame->iHeight/4;

					m_pD3DRender->FillData( pY, pV, pU, pFrame->iWidth, pFrame->iHeight, 
						pFrame->iWidth, pFrame->iWidth/2, pFrame->iWidth/2 );
				}
			}
		}//mpeg4解码

		bResult = true;

	}while( false );

	m_csInit.Unlock();


#if 0
	dwPlayEnd = timeGetTime();
	CString strDebug;
	strDebug.Format("##@@ index=%d @@Play time:%d, Decode time :%d, nowtm=%d\n", m_iIndex, dwPlayEnd - dwPlayStart, dwDecodeEnd - dwDecodeStart, dwPlayEnd);
	OutputDebugString(strDebug);
#endif

	return bResult;
}


//#include "../media/vd.h"

void 
CVideoBS::RenderYV12( char *pY, char *pU, char *pV, int width, int height, int yPitch, int uPitch, int vPitch )
{
	StatisticFrameRate();
	BOOL bResult = FALSE;
	BOOL bWindowsChanged = FALSE;
	HRESULT hr = FALSE;

	if( !m_csInit.Lock( ))
	{
		return ;
	}

	do 
	{
		if( m_pD3DRender == NULL )
		{
			m_pD3DRender = new D3D9RenderImpl( );
			m_pD3DRender->Initialize( m_hWnd, 0, NULL );
		}

		m_pD3DRender->FillData( (BYTE*)pY, (BYTE*)pU, (BYTE*)pV, width, height, yPitch, uPitch, vPitch );

	}while( false );

	m_csInit.Unlock();


	return ;
}

void
CVideoBS::SetStatus(char c, BOOL bFlag)
{
	if (m_csInit.Lock())
	{
		if( m_pD3DRender != NULL )
		{
			m_pD3DRender->SetStatus( c, bFlag );
		}

		m_csInit.Unlock();
	}
}

void
CVideoBS::SetTitle(TitlePos iPos, LPCSTR lpTitle, DWORD dwColor, int iX/* =0 */, int iY/* =0 */,int nLine)
{
	CString strTitle = lpTitle;
	if (m_csInit.Lock())
	{
		if (m_sTitle[(int)iPos][nLine] == "%Ft" || m_sTitle[(int)iPos][nLine] == ">" || m_sTitle[(int)iPos][nLine] == "||" )
		{


			if( m_pD3DRender != NULL )
			{
				m_pD3DRender->m_sTitle[iPos][nLine]= "";
				m_sTitle[iPos][nLine] = "";
			}
		}
#if 0
		if (m_sTitle[iPos][nLine] != "" && strTitle !="")
		{
			m_sTitle[iPos][nLine] += ",";
			m_sTitle[iPos][nLine] += strTitle;
		}else 
#endif
		{
			m_sTitle[iPos][nLine] = strTitle;
		}

		if( m_pD3DRender != NULL /*&& strTitle != ""*/)
		{
			//m_pD3DRender->SetTitle( iPos, lpTitle, dwColor, nLine );
		}

		m_csInit.Unlock();
	}

	//ReDrawStatus(); ////暂时注释，需要修改
}

void
CVideoBS::PreTextOut(HDC hdc, CString strTitle, int &iWidth, int &iHeigth)
{
	iWidth = 0;
	iHeigth = 0;
	int iStart = 0;
	tagSIZE sz;
	while(TRUE)
	{
		int pos = strTitle.Find(_T("\n"), iStart);
		if (pos == -1)
		{
			CString str = strTitle.Right(strTitle.GetLength() - iStart);
			GetTextExtentPoint(hdc, str, str.GetLength(), &sz);
			iWidth = max(sz.cx, iWidth);
			iHeigth += sz.cy;
			break;
		}

		CString str = strTitle.Mid(iStart, pos - iStart);
		GetTextExtentPoint(hdc, str, str.GetLength(), &sz);
		iWidth = max(sz.cx, iWidth);
		iHeigth += sz.cy;

		iStart = pos + 1;
	}
}



BOOL
CVideoBS::Capture(LPCSTR lpPath, DWORD dwWidth/* =0 */, DWORD dwHeight/* =0 */)
{ 
	if (!DisplayIsReady())
	{
		return FALSE;
	}

	BOOL bResult = FALSE;

	return bResult;
}


BOOL
CVideoBS::CaptureEx(LPCSTR lpPath, DWORD dwWidth/* =0 */, DWORD dwHeight/* =0 */)
{ 
	if (!DisplayIsReady())
	{
		return FALSE;
	}

	//需要添加抓图函数
	BOOL bResult = FALSE;

	if( m_csInit.Lock() )
	{
		if( m_pD3DRender != NULL )
		{
			bResult = m_pD3DRender->CaptureEx( lpPath, dwWidth, dwHeight );
		}

		m_csInit.Unlock();
	}

	return bResult;
}


void
CVideoBS::StatisticFrameRate()
{
	DWORD dwCurTm = timeGetTime();
	m_dwFramePlayedStatSample[m_iCurFrameRateStatSample]++;
	if(dwCurTm > m_dwPreFrameRateStatTm + 1000 * TIME_PER_STAT)
	{
		DWORD dwTotalFrame = 0;
		int iAvailability = 0;
		for (int i = 0; i < SAMPLE_OF_STAT; i++)
		{
			if (m_dwFramePlayedStatSample[i] > 0)
			{
				dwTotalFrame += m_dwFramePlayedStatSample[i];
				iAvailability++;
			}
		}
		iAvailability = (iAvailability == 0) ? 1 : iAvailability;
		m_dwFrameRate = dwTotalFrame / iAvailability;
		m_dwFramePlayed += m_dwFramePlayedStatSample[m_iCurFrameRateStatSample];
		m_iCurFrameRateStatSample = (m_iCurFrameRateStatSample + 1) % SAMPLE_OF_STAT;
		m_dwFramePlayedStatSample[m_iCurFrameRateStatSample] = 0;
		m_dwPreFrameRateStatTm = dwCurTm;
	}
}


void CVideoBS::SetCBDecFrameFun(PCBFunGetDecFrameData pCBFun, LPVOID pUserData)
{
	m_pCBFun = pCBFun;
	m_pUserData = pUserData;
}


BOOL
CVideoBS::DecodeFrame(FrameStruct* pFrame, BYTE* pVideoFrame)
{
	BOOL bResult = FALSE;
	int iLoop = 0;
	int iRet = 1;
	BYTE *pOutPutFrameBuf = NULL;
	while (TRUE)
	{
		if( m_hDecode == NULL )
		{
			break;
		}
		if (pFrame == NULL)
		{
			break;
		}
		if (m_iDecoder == MDECODE_BSX)
		{
			// 只复制信息，不需要yuv数据
			//m_lastFrameInfo = *pFrame; 
			memset(&m_dec_frame,0,sizeof(bsDecFrameInfo));

			iRet = bsDecFrame( m_hDecode, (char*)pFrame->pBuf, pFrame->iLen, (unsigned char) pFrame->encType, &m_dec_frame );


			if (iRet == 0 )
			{
				m_dec_frame.uHeight -= m_dec_frame.uCroppingBottomOffset;
				// 三星的1080p裁掉下边的花屏
				if (MDECODE_SAM == GetRealDecoder())
				{
					if (m_dec_frame.uHeight == 1088)
					{
						m_dec_frame.uHeight = 1058;
					}
				}
				// 松下800*600 解码出 800*608 下边沿出现花屏
				if(MDECODE_PSC == GetRealDecoder())
				{
					if (m_dec_frame.uHeight == 608)
					{
						m_dec_frame.uHeight = 600;
					}
				}
				m_rcSrc.left = 0;
				m_rcSrc.top = 0;
				m_rcSrc.right = m_dec_frame.uWidth;
				if(pFrame->iHeight != m_dec_frame.uHeight &&
					pFrame->iHeight == 2*m_dec_frame.uHeight &&
					m_dec_frame.uWidth > 352)
				{
					m_rcSrc.bottom = pFrame->iHeight;
				}
				else
				{
					m_rcSrc.bottom = m_dec_frame.uHeight;
				}
				m_bDecodeOutBufIsOk = TRUE;

				//解码数据回调
				if (m_pCBFun != NULL)
				{
					DEC_FRAME_S decFrame;
					decFrame.pY = m_dec_frame.pV;
					decFrame.pU = m_dec_frame.pU;
					decFrame.pV = m_dec_frame.pV;
					decFrame.uWidth = m_dec_frame.uWidth;
					decFrame.uHeight = m_dec_frame.uHeight;
					decFrame.uYStride = m_dec_frame.uYStride;
					decFrame.uUVStride = m_dec_frame.uUVStride;
					m_pCBFun(&decFrame, m_pUserData);
				}

#if 0
				if (NULL != m_pUser && NULL != m_pBSDecYUVDataCB)
				{
					//回调limit解码yuv数据
					int tempHeight = m_dec_frame.uHeight;
					pFrame->iWidth = 720;
					pFrame->iHeight = 576;

					if((pFrame->iWidth == 704 && pFrame->iHeight == 576 &&pFrame->iHeight == m_dec_frame.uHeight*2&& m_dec_frame.uWidth>352)   || 
						(pFrame->iWidth == 640 && pFrame->iHeight == 480 &&pFrame->iHeight == m_dec_frame.uHeight*2&& m_dec_frame.uWidth>320)||
						(pFrame->iWidth == 720 && pFrame->iHeight == 576 &&pFrame->iHeight == m_dec_frame.uHeight*2&& m_dec_frame.uWidth>320))
					{
						tempHeight += tempHeight;
					}
					//1表示显示时uv相反
					m_pBSDecYUVDataCB(1,
						(char*)m_dec_frame.pY,
						(char*)m_dec_frame.pU,
						(char*)m_dec_frame.pV,
						(int)m_dec_frame.uWidth,
						(int)tempHeight,
						(int)m_dec_frame.uYStride,
						(int)(m_dec_frame.uUVStride),
						(int)(m_dec_frame.uUVStride),
						m_pUser);
				}
#endif

				m_bDecodeFrame = true;
			}
			else
			{

				//TRACE("err\n");
				m_bDecodeFrame = false;
				bResult = FALSE;
			}

			bResult = TRUE;
			break;
		}
		else
		{
#if 0
			if ( !m_bOpenDiagnostics && pVideoFrame != NULL )  //没有开启视频诊断，则直接拷贝到ddraw表面
			{
				pOutPutFrameBuf = pVideoFrame;
			}
			else
			{
				m_iDecodeOutHeight = CIFF_YDIM;
				m_iDecodeOutStide  = CIFF_XDIM*2;
				pOutPutFrameBuf = m_pDecodeOutBuf;
			}
			int iCansu4 = (pFrame != NULL) ? 0 : LAST_FRAME;
			//BASE_PIC_INFO tagPicInfo;
			memset(&m_tagPicInfo, 0, sizeof(BASE_PIC_INFO));
			if (!m_bRGB)
			{
				m_tagPicInfo.pimg_out[0] = (BYTE *)pOutPutFrameBuf;
				m_tagPicInfo.pimg_out[1] = (BYTE *)pOutPutFrameBuf + m_iDecodeOutHeight*m_iDecodeOutStide;
				m_tagPicInfo.pimg_out[2] = (BYTE *)pOutPutFrameBuf + m_iDecodeOutHeight*m_iDecodeOutStide*5/4;
				m_tagPicInfo.out_stride[0] = m_iDecodeOutStide;
				m_tagPicInfo.out_stride[1] = m_iDecodeOutStide/2;
				m_tagPicInfo.out_stride[2] = m_iDecodeOutStide/2;
			}
			else
			{
				iCansu4 = iCansu4 | OUTRGB_IMAGE;
				m_tagPicInfo.pimg_out[0] = (BYTE *)pOutPutFrameBuf;
				m_tagPicInfo.out_stride[0] = m_iDecodeOutStide * 3;
			}

			iRet = Bs_Mpeg4_dec_frame(m_hDecode
				, ((pFrame != NULL) ? pFrame->pBuf : NULL)
				, ((pFrame != NULL) ? pFrame->iLen : 0)
				, pOutPutFrameBuf
				, iCansu4
				, &m_tagPicInfo);

			if (m_tagPicInfo.ready == 0)
			{
				iRet = Bs_Mpeg4_dec_frame(m_hDecode
					, ((pFrame != NULL) ? pFrame->pBuf : NULL)
					, ((pFrame != NULL) ? pFrame->iLen : 0)
					, pOutPutFrameBuf
					, iCansu4
					, &m_tagPicInfo);
			}

			if (iRet == 0)
			{
				m_videoWidth = 0;
				m_videoHeight = 0;
				m_bDecodeFrame = false;
				iLoop++;
				if (iLoop < 5)
				{
					m_bDecodeOutBufIsOk = FALSE;
					continue;
				}

			}
			else
			{
				m_bDecodeFrame = true;
				m_bDecodeOutBufIsOk = TRUE;

				//解码后数据回调
				if (m_pCBFun != NULL)
				{
					DEC_FRAME_S decFrame;
					decFrame.pY = m_tagPicInfo.pimg_out[0];
					decFrame.pU = m_tagPicInfo.pimg_out[1];
					decFrame.pV = m_tagPicInfo.pimg_out[2];
					decFrame.uWidth = m_tagPicInfo.Width;
					decFrame.uHeight = m_tagPicInfo.Height;
					decFrame.uYStride = m_iDecodeOutStide;
					decFrame.uUVStride = m_iDecodeOutStide/2;
					m_pCBFun(&decFrame, m_pUserData);
				}

				if (NULL != m_pUser && NULL != m_pBS17DataCB)
				{
					//回调17解码前数据
					//DWORD startTime = GetTickCount();
					m_pBS17DataCB(
						(char*)pFrame->pBuf,
						pFrame->iLen,
						m_pUser);
					//TRACE("const time = %d\n", GetTickCount() - startTime );
				}
			}

			if (m_tagPicInfo.ready == 0)
			{
				m_rcSrc.left   = 0;
				m_rcSrc.top    = 0;
				m_rcSrc.right  = 0;
				m_rcSrc.bottom = 0;
				break;
			}

			m_rcSrc.left = 0;
			m_rcSrc.top = 0;
			m_rcSrc.right = m_tagPicInfo.Width;
			m_rcSrc.bottom = m_tagPicInfo.Height;

			//DoDiagnostics();
			DoInterlace((pFrame != NULL) ? pFrame->iInterlace : 0);
			bResult = TRUE;
			break;
#endif
		}

	}

	return bResult;
}

void
CVideoBS::DoInterlace(int interlace)
{
#ifdef _USE_INTERLACE_
	if (interlace)
	{
		if (m_pDecodeOutBuf == NULL)
		{
			m_pInterlaceBuf = new BYTE[m_iDecodeOutBufSize];
		}
		memcpy_s(m_pInterlaceBuf, m_pDecodeOutBuf, m_iDecodeOutBufSize);
		DeinterlaceBlend(m_pDecodeOutBuf, m_pInterlaceBuf, m_iDecodeOutStide, m_iDecodeOutHeight, m_iDecodeOutStide, m_iDecodeOutStide);
	}
#endif // _USE_INTERLACE_
}

BOOL
CVideoBS::DisPlayBmp(FrameStruct* pFrame)
{
	int decodecsize = DECODE_BUFSIZE;
	if(pFrame != NULL)
	{
		memcpy_s(m_pDecodeOutBuf, m_iDecodeOutBufSize, pFrame->pBuf, decodecsize/*884736*/);
	}
	//   m_rcSrc.left = 0;
	//   m_rcSrc.top = 0;
	//   m_rcSrc.right = pFrame->iWidth;
	//   m_rcSrc.bottom = pFrame->iHeight;

	return DisplayFrame(NULL);
}

void
CVideoBS::ShowCaption(BOOL bShow)
{
	if( m_pD3DRender != NULL )
	{
		m_pD3DRender->ShowCaption( bShow );
	}
}



void
CVideoBS::SetSurfaceFormat(SurfaceFormat format)
{
	m_iSurfaceFormat = format;
}

BOOL
CVideoBS::SetFrameSize(DWORD dwWidth, DWORD dwHeight)
{
	BOOL bResult = FALSE;

	return bResult;
}


#ifdef POST_SHARP


int dumy[2] = {0, 0};
int strength[2] = {0, 0};

void __cdecl CVideoBS::FilterSharpen_SSE(unsigned __int8 *src, unsigned __int8 *dst, int width, int height, int filterStrength)
{
	strength[0] = filterStrength;//strength
	__asm
	{
		mov     edi, width; //width
		shr     edi, 3;
		sub     edi, 2; 
		mov     eax, src;//src 
		mov     ecx, dst;//dst
		mov     edx, height;//height

JLOOP:
		mov     ebx, edi

			movq    mm0, qword ptr [eax-1]
		movq    mm1, qword ptr [eax]
		movq    mm2, qword ptr [eax+1]
		add     eax, 8

			movq    mm4, mm0
			movq    mm5, mm1
			movq    mm6, mm2
			punpcklbw mm0, dumy
			punpcklbw mm1, dumy
			punpcklbw mm2, dumy
			punpckhbw mm4, dumy
			punpckhbw mm5, dumy
			punpckhbw mm6, dumy
			pshufw  mm0, mm0, 0E5h
			movq    mm3, mm1
			movq    mm7, mm5
			paddw   mm0, mm2
			psllw   mm1, 1
			psubw   mm0, mm1
			psraw   mm0, strength
			psubw   mm3, mm0
			paddw   mm4, mm6
			psllw   mm5, 1
			psubw   mm4, mm5
			psraw   mm4, strength
			psubw   mm7, mm4
			packuswb mm3, mm7

NLOOP:
		movq    mm0, qword ptr [eax-1]
		movq    mm1, qword ptr [eax]
		movq    mm2, qword ptr [eax+1]
		add     eax, 8

			movq    qword ptr [ecx], mm3 
			add     ecx, 8 

			movq    mm4, mm0
			movq    mm5, mm1
			movq    mm6, mm2
			punpcklbw mm0, dumy
			punpcklbw mm1, dumy
			punpcklbw mm2, dumy
			punpckhbw mm4, dumy
			punpckhbw mm5, dumy
			punpckhbw mm6, dumy
			movq    mm3, mm1
			movq    mm7, mm5
			paddw   mm0, mm2
			psllw   mm1, 1
			psubw   mm0, mm1
			psraw   mm0, strength
			psubw   mm3, mm0
			paddw   mm4, mm6
			psllw   mm5, 1
			psubw   mm4, mm5
			psraw   mm4, strength
			psubw   mm7, mm4
			packuswb mm3, mm7
			sub     ebx, 1 
			jnz     NLOOP

			movq    mm0, qword ptr [eax-1]
		movq    mm1, qword ptr [eax]
		movq    mm2, qword ptr [eax+1]
		add     eax, 8

			movq    qword ptr [ecx], mm3 
			add     ecx, 8

			movq    mm4, mm0
			movq    mm5, mm1
			movq    mm6, mm2
			punpcklbw mm0, dumy
			punpcklbw mm1, dumy
			punpcklbw mm2, dumy
			punpckhbw mm4, dumy
			punpckhbw mm5, dumy
			punpckhbw mm6, dumy
			pshufw  mm6, mm6, 0A4h
			movq    mm3, mm1
			movq    mm7, mm5
			paddw   mm0, mm2
			psllw   mm1, 1
			psubw   mm0, mm1
			psraw   mm0, strength
			psubw   mm3, mm0
			paddw   mm4, mm6
			psllw   mm5, 1
			psubw   mm4, mm5
			psraw   mm4, strength
			psubw   mm7, mm4
			packuswb mm3, mm7
			movq    qword ptr [ecx], mm3
			add     ecx, 8
			sub     edx, 1 
			jnz     JLOOP

			emms
	}
}

#endif



void CVideoBS::SetRealDecoder(int iDecoderId)
{
	m_iRealDecoder = iDecoderId;
}

int CVideoBS::GetRealDecoder()
{
	return m_iRealDecoder;
}




/**
* @fn void CRealTransfer::DrawTransBitmap
* @brief  画透明位图
* @param[in]  hdcDest  目标DC
* @param[in]  int nXOriginDest 目标X偏移
* @param[in]  int nYOriginDest 目标Y偏移
* @param[in]  int nWidthDest   目标宽度
* @param[in]  int nHeightDest  目标高度
* @param[in]  HDC hdcSrc,      源DC
* @param[in]  int nXOriginSrc, 源X起点
* @param[in]  int nYOriginSrc  源Y起点
* @param[in]  int nWidthSrc    源宽度
* @param[in]  int nHeightSrc   源高度
* @param[in]  UINT crTransparent  透明色,COLORREF类型
* @return void
*/
void CVideoBS::DrawTransBitmap( HDC hdcDest,     
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
															 )
{
	do 
	{
		if (nWidthDest < 1) 
			break;

		if (nWidthSrc < 1) 
			break;

		if (nHeightDest < 1) 
			break;

		if (nHeightSrc < 1)
			break;

		HDC dc = CreateCompatibleDC(NULL);
		HBITMAP bitmap = CreateBitmap(nWidthSrc, nHeightSrc, 1, GetDeviceCaps(dc,
			BITSPIXEL), NULL);
		if (bitmap == NULL)
		{
			DeleteDC(dc);   
			break;
		}

		HBITMAP oldBitmap = (HBITMAP)SelectObject(dc, bitmap);
		if (!BitBlt(dc, 0, 0, nWidthSrc, nHeightSrc, hdcSrc, nXOriginSrc,
			nYOriginSrc, SRCCOPY))
		{
			SelectObject(dc, oldBitmap);
			DeleteObject(bitmap);        
			DeleteDC(dc);               
			break;
		}

		HDC maskDC = CreateCompatibleDC(NULL);
		HBITMAP maskBitmap = CreateBitmap(nWidthSrc, nHeightSrc, 1, 1, NULL);
		if (maskBitmap == NULL)
		{
			SelectObject(dc, oldBitmap);
			DeleteObject(bitmap);        
			DeleteDC(dc);               
			DeleteDC(maskDC);            
			break;
		}

		HBITMAP oldMask =  (HBITMAP)SelectObject(maskDC, maskBitmap);
		SetBkColor(maskDC, RGB(0,0,0));
		SetTextColor(maskDC, RGB(255,255,255));
		if (!BitBlt(maskDC, 0,0,nWidthSrc,nHeightSrc,NULL,0,0,BLACKNESS))
		{
			SelectObject(maskDC, oldMask);
			DeleteObject(maskBitmap);      
			DeleteDC(maskDC);              
			SelectObject(dc, oldBitmap);   
			DeleteObject(bitmap);         
			DeleteDC(dc);                  
			break;
		}

		SetBkColor(dc, crTransparent);
		BitBlt(maskDC, 0,0,nWidthSrc,nHeightSrc,dc,0,0,SRCINVERT);
		SetBkColor(dc, RGB(0,0,0));
		SetTextColor(dc, RGB(255,255,255));
		BitBlt(dc, 0,0,nWidthSrc,nHeightSrc,maskDC,0,0,SRCAND);
		HDC newMaskDC = CreateCompatibleDC(NULL);
		HBITMAP newMask;
		newMask = CreateBitmap(nWidthDest, nHeightDest, 1,
			GetDeviceCaps(newMaskDC, BITSPIXEL), NULL);
		if (newMask == NULL)
		{
			SelectObject(dc, oldBitmap);
			DeleteDC(dc);
			SelectObject(maskDC, oldMask);
			DeleteDC(maskDC);
			DeleteDC(newMaskDC);
			DeleteObject(bitmap);     
			DeleteObject(maskBitmap);
			break;
		}

		SetStretchBltMode(newMaskDC, COLORONCOLOR);
		HBITMAP oldNewMask = (HBITMAP) SelectObject(newMaskDC, newMask);
		StretchBlt(newMaskDC, 0, 0, nWidthDest, nHeightDest, maskDC, 0, 0,
			nWidthSrc, nHeightSrc, SRCCOPY);
		SelectObject(maskDC, oldMask);
		DeleteDC(maskDC);
		DeleteObject(maskBitmap);
		HDC newImageDC = CreateCompatibleDC(NULL);
		HBITMAP newImage = CreateBitmap(nWidthDest, nHeightDest, 1,
			GetDeviceCaps(newMaskDC, BITSPIXEL), NULL);
		if (newImage == NULL)
		{
			SelectObject(dc, oldBitmap);
			DeleteDC(dc);
			DeleteDC(newMaskDC);
			DeleteObject(bitmap);     
			break;
		}

		HBITMAP oldNewImage = (HBITMAP)SelectObject(newImageDC, newImage);
		StretchBlt(newImageDC, 0, 0, nWidthDest, nHeightDest, dc, 0, 0, nWidthSrc,
			nHeightSrc, SRCCOPY);
		SelectObject(dc, oldBitmap);
		DeleteDC(dc);
		DeleteObject(bitmap);     
		BitBlt( hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
			newMaskDC, 0, 0, SRCAND);
		BitBlt( hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
			newImageDC, 0, 0, SRCPAINT);
		SelectObject(newImageDC, oldNewImage);
		DeleteDC(newImageDC);
		SelectObject(newMaskDC, oldNewMask);
		DeleteDC(newMaskDC);
		DeleteObject(newImage);   
		DeleteObject(newMask);   
		break ;
	} while ( FALSE);

	return;
}
void CVideoBS::SetDelayTime(int delay, DWORD dwTmCur,  int iflag)
{
	m_iDwTmCur = dwTmCur;
	m_iDelayTime = delay;
	m_bRefresh = TRUE;
	m_iDelayflag = iflag;
}
void CVideoBS::WaitDelay(int iDecodeTime)
{
	if (m_bRefresh)
	{
		int iTm = 0;
		DWORD dwNow = ::timeGetTime();
		if (0 == m_iDelayflag)
		{
			// 减去解码时间
			iTm = m_iDelayTime - iDecodeTime;
			// 减去线程轮转到这里的时间
			iTm -= (dwNow - m_iDwTmCur /*-4*/);
		}
		else
		{
			iTm = m_iDelayTime;
		}
		/*CString str;
		str.Format("WaitTime Delay :%d , m_iDelayTime:%d, iDecodeTime:%d moreTime:%d\n", iTm, m_iDelayTime, iDecodeTime, dwNow - m_iDwTmCur);
		OutputDebugString(str);*/

		if (iTm > 0)
		{
			::Sleep(iTm);
		}
		else
		{
			//::Sleep(0);
		}
		m_bRefresh = FALSE;
	}
}