#pragma once

#include <stdlib.h>
#include "d3d9.h"
#include "Vertex.h"
#include "atlbase.h"
#include "Macros.h"
#include <windows.h>
#include "Overlay.h"
#include "Format.h"
//#include "IRenderable.h"



#define MAX_TITLE_POS    5   //定义字符叠加位置
#define MAX_TITLE_LINE   10  //定义字符叠加行数

#ifndef USE_SURFACE
#define USE_SURFACE
#endif

//#undef USE_SURFACE

class D3D9RenderImpl
{
public:
	D3D9RenderImpl();
	virtual ~D3D9RenderImpl(void);

	virtual HRESULT Initialize(HWND hDisplayWindow, int index, void *pOwner );
	virtual HRESULT CheckFormatConversion(D3DFORMAT format);
	virtual HRESULT CreateVideoSurface(int width, int height, D3DFORMAT format = D3DFMT_YV12);
	virtual HRESULT Display(BYTE* pYplane, BYTE* pVplane, BYTE* pUplane, int width, int height, int strideY, int strideU, int strideV );

	//获取视频缓冲指针，由外部填充
	virtual HRESULT GetVideoPt( int width, int height, BYTE **dst, int &pitch );

	//释放并显示
	virtual HRESULT ReleaseVideoPt( );
	virtual HRESULT ReleaseVideoPt( RECT srcRect, bool bShow );

	virtual HRESULT FillData(BYTE* pYplane, BYTE* pVplane, BYTE* pUplane, int width, int height, int strideY, int strideU, int strideV );


	virtual void SetDisplayMode(FillMode mode);
    virtual FillMode GetDisplayMode();

	HRESULT SetPixelShader(LPCSTR pPixelShaderName, LPCSTR entryPoint, LPCSTR shaderModel, LPSTR* ppError);
	HRESULT SetPixelShader(DWORD* buffer);
	HRESULT SetPixelShaderConstant(LPCSTR name, LPVOID value, UINT size);

	HRESULT SetVertexShader(LPCSTR pVertexShaderName, LPCSTR entryPoint, LPCSTR shaderModel, LPSTR* ppError);
	HRESULT SetVertexShader(DWORD* buffer);
	HRESULT SetVertexShaderConstant(LPCSTR name, LPVOID value, UINT size);
	HRESULT ApplyWorldViewProj(LPCSTR matrixName);

	HRESULT SetVertexShaderMatrix(D3DXMATRIX* matrix, LPCSTR name);
	HRESULT SetPixelShaderMatrix(D3DXMATRIX* matrix, LPCSTR name);
	HRESULT SetVertexShaderVector(D3DXVECTOR4* vector, LPCSTR name);
	HRESULT SetPixelShaderVector(D3DXVECTOR4* vector, LPCSTR name);

	virtual HRESULT ClearPixelShader();
	virtual HRESULT ClearVertexShader();

	virtual HRESULT CaptureDisplayFrame(BYTE* pBuffer, INT* width, INT* height, INT* stride);
	virtual HRESULT CaptureVideoFrame(BYTE* pBuffer, INT* width, INT* height, INT* stride);

	//void SetTitle(TitlePos iPos, LPCSTR lpTitle, DWORD dwColor, int nLine);
	void ShowCaption( bool bShow );
	void SetStatus( char c, BOOL bFlag );
	//设置遮挡
	void SetShelter(BOOL bShelter, LPRECT lpRc, LPVOID lpReserved);
	BOOL CaptureEx( LPCSTR lpPath, DWORD dwWidth, DWORD dwHeight );

#if 0
	void ShowIAInfo( BOOL bShow );

	void SetIAFrame( IAFrame *pFrame, int len );
	void ResetIAInfo();
#endif

	HRESULT DiscardResources();

	void Render( );
	
private:
	IDirect3D9*             m_pD3D9;
	IDirect3DDevice9*       m_pDevice;
	IDirect3DSurface9*      m_pOffsceenSurface;
	IDirect3DSurface9*      m_pTextureSurface;
	IDirect3DSurface9*      m_pRGBTextureSurface;
	IDirect3DTexture9*      m_pTexture;
	IDirect3DVertexBuffer9*  m_pVertexBuffer;
	IDirect3DVertexShader9* m_pVertexShader; 
	ID3DXConstantTable*     m_pVertexConstantTable; 
	ID3DXConstantTable*     m_pPixelConstantTable; 
	IDirect3DPixelShader9*  m_pPixelShader;

	ID3DXFont*              m_pTextTexture; //定义字符叠加纹理

	D3DDISPLAYMODE m_displayMode;
	HWND m_hDisplayWindow;

	int m_wndWidth;
	int m_wndHeight;

	int m_videoWidth;
	int m_videoHeight;
	D3DFORMAT m_format;
	OverlayStore m_overlays;
	FillMode m_fillMode;
	D3DPRESENT_PARAMETERS m_presentParams;

	int m_deviceIndex;

	//字符信息
	
	DWORD   m_dwTitleColor[MAX_TITLE_POS];
	CSize   m_titleSize[MAX_TITLE_POS][MAX_TITLE_LINE];
	
	bool m_bShowCaption;

	RECT m_rcDes; //绘制的目标区域

	// 遮挡配置(临时)
	BOOL           m_bShelter;      // 遮挡是否开启
	RECT           m_rcShelter;     // 遮挡区域
	CDC*           m_pDCShelter;    // 遮挡图片

	//智能相关
	BOOL           m_bShowIAInfo;
	//IAFrame        *m_pIAFrame;

	CPen           *m_pPenGreen;
	CPen           *m_pPenRed;

	float          m_xscale;
	float          m_yscale;
	
	void *m_pOwner;

	bool           m_bExit;
	bool           m_bBusy;
	bool           m_bVideoChanged;

	HANDLE          m_hRender;

	CRITICAL_SECTION m_section;

	//记录数据源区域
	RECT             m_srcRect;
	HANDLE           m_hEvent;
	
	char *m_pIABuf;

	bool m_bDataReady;

public:
	//字符信息
	CString m_sTitle[MAX_TITLE_POS][MAX_TITLE_LINE];


private:
	HRESULT SetupMatrices(int width, int height);
	HRESULT CreateScene(void);
	HRESULT CheckDevice(void);
	HRESULT FillBuffer(BYTE* pY, BYTE* pV, BYTE* pU, int strideY, int strideU, int strideV);
	HRESULT CreateRenderTarget(int width, int height);
	HRESULT Present(void);	
	
	HRESULT Present( RECT srcRect );

	HRESULT GetPresentParams(D3DPRESENT_PARAMETERS* params, BOOL bWindowed);

	
	HRESULT CreateResources();

	HRESULT Reset( );

	void OverlayDrawText( LPCSTR pText, RECT pos, DWORD color );
	void DrawTitle( );
	//绘制状态
	void DrawStatusFlag( HDC hdc, RECT *rc );

	//
	void DrawTitleForCapture( HDC hdc, RECT *pRect );

	void PreTextOut(HDC hdc, CString strTitle, int &iWidth, int &iHeigth);

	int SaveDC2BmpFile(const HDC hDC, const unsigned long dwWidth, const unsigned long dwHeight, 
						 const unsigned long dwVFWidth, const unsigned long dwVFHeight, const char* szFileName);

	DWORD D3DColorToRGB( DWORD color );

	/*
	绘制智能信息
	*/
#if 0
	 // 绘制IA信息
	void DrawIAInfo(CDC* pDC, IAFrame* pFrame, RECT *pRect );
	// 计算当前IA Frame
	IAFrame* GetIAFrame(int sequence);
	// 绘制点连线
	void DrawIARgn(CDC* pDC, int count, point_t* pts);
	// 绘制矩形
	void DrawIARect(CDC* pDC, quad_t* figure);

	// 计算2点间距离
	double GetDistance(POINT pt1, POINT pt2);
	// 计算2点垂线
	void GetPerpendicularLine(POINT ptIn1, POINT ptIn2, POINT& ptOut1, POINT& ptOut2);
	// 绘制箭头
	void DrawArrow(CDC* pDC, POINT ptFrom, POINT ptTo, double radian, double len);
	// IA是否报警
	BOOL IAIsAlarmEvent(IAEvent* pEvent, int nIndex);
	// IA信息有效
	BOOL IAFrameIsValide(IAFrame* pFrame);
#endif

	//获取监视器
	int GetCurMonitor( HWND hWnd );


	void ReleaseDebug( IUnknown **pObj );
	  			
};

