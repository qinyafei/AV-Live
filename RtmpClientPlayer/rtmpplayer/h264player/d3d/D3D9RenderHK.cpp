#include "StdAfx.h"
#include "D3D9RenderHK.h"


#define MAX_IABUF 2*1024 //定义智能帧最大缓冲区

//#pragma comment ( lib, "d3d9.lib" )
//#pragma comment ( lib, "d3dx9.lib" )

extern CDC g_dcStatus;
extern UINT g_transColor;

extern CbsrMediaApp theApp;

//DWORD WINAPI RenderThread( LPVOID lpvoid )
unsigned int __stdcall RenderThreadHK( LPVOID lpvoid ) 
{
	D3D9RenderHK *pRender = (D3D9RenderHK*)lpvoid;
	if( pRender != NULL )
	{
		pRender->Render( );
	}

	 _endthreadex( 0 );

	return 0;
}

D3D9RenderHK::D3D9RenderHK()
    : m_pVertexShader(0), m_pPixelShader(0), m_pD3D9(0), m_format(D3DFMT_UNKNOWN), m_pVertexConstantTable(0), m_pPixelConstantTable(0), 
      m_fillMode(KeepAspectRatio)
{
	
	m_pDevice = 0;
	m_pOffsceenSurface     = 0;
	m_pTextureSurface      = 0;
	m_pRGBTextureSurface   = 0;
	m_pTexture             = 0;
	m_pVertexBuffer        = 0; 
	m_pVertexShader        = 0; 
	m_pVertexConstantTable = 0; 
	m_pPixelConstantTable  = 0; 
	m_pPixelShader         = 0;
	m_pTextTexture         = 0; //定义字符叠加纹理

    m_pRGBTextureSurface = NULL;
	

	m_wndWidth           = 0;
	m_wndHeight          = 0;

	m_videoWidth         = 0;
	m_videoHeight        = 0;

	m_bShowCaption       = false;
	
	for( int i = 0; i < MAX_TITLE_POS; i++ )
	{
		m_dwTitleColor[i] = 0xff0000ff; //默认为蓝色
		for( int j = 0; j < MAX_TITLE_LINE; j++ )
		{
			m_titleSize[i][j].cx = 0;
			m_titleSize[i][j].cy = 0;
		}
	}

	

	memset( &m_rcDes, 0, sizeof(m_rcDes) );

	m_deviceIndex        = D3DADAPTER_DEFAULT;

	m_bShelter    = FALSE;								// 遮挡是否开启
	m_pDCShelter  = NULL;								// 遮挡图片
	memset( &m_rcShelter, 0, sizeof(m_rcShelter) );     // 遮挡区域

	m_pIAFrame  = NULL;
	m_pPenGreen = NULL;
	m_pPenRed   = NULL;

	m_xscale = 1.0;
	m_yscale = 1.0;

	m_bShowIAInfo = false;

	m_pOwner = NULL;

    m_bExit			= false;
	m_bBusy			= false;
	m_bVideoChanged = false;

	m_hRender	    = NULL;

	m_hEvent = CreateEvent( NULL, false, false, NULL );

	memset( &m_srcRect, 0, sizeof(m_srcRect) );
	
    m_pIABuf = new char[MAX_IABUF];
	
	memset( m_pIABuf, 0, MAX_IABUF );

	m_pIAFrame = (IAFrame*)m_pIABuf;

	InitializeCriticalSection( &m_section );

}

HRESULT D3D9RenderHK::Initialize(HWND hDisplayWindow, int index, void *pOwner )
{
	m_pOwner = pOwner;

    m_hDisplayWindow = hDisplayWindow;

	
	//theApp.D3DLock();
    m_pD3D9 = Direct3DCreate9( D3D_SDK_VERSION ); 
	//theApp.D3DUnlock();

    if(!m_pD3D9)
    {
		OutputDebugString( "====Create D3D object failed =====\r\n");
        return E_FAIL;
    }
	
	m_deviceIndex = GetCurMonitor( hDisplayWindow );

	OutputDebugString( "============create d3d object success===========\r\n" );
    HR(m_pD3D9->GetAdapterDisplayMode(m_deviceIndex, &m_displayMode));

	//Reset( );

	

	/*

    D3DCAPS9 deviceCaps;
    HR(m_pD3D9->GetDeviceCaps(m_deviceIndex, D3DDEVTYPE_HAL, &deviceCaps));

    DWORD dwBehaviorFlags = D3DCREATE_MULTITHREADED;

    if( deviceCaps.VertexProcessingCaps != 0 )
        dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else
        dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    
    HR(GetPresentParams(&m_presentParams, TRUE));
    
    HR(m_pD3D9->CreateDevice(m_deviceIndex, D3DDEVTYPE_HAL, m_hDisplayWindow, dwBehaviorFlags, &m_presentParams, &m_pDevice));
    
    return CreateResources();
	*/

	m_bExit = false;
	m_hRender = NULL;



	//m_hRender = CreateThread( NULL, 0, RenderThread, this, 0, 0 );
	m_hRender = (HANDLE)_beginthreadex( NULL, 0, RenderThreadHK, this, 0, NULL );

	return S_OK;
}

D3D9RenderHK::~D3D9RenderHK(void)
{

	m_bExit = true;
	if( m_hRender != NULL )
	{
		SetEvent( m_hEvent );
		WaitForSingleObject( m_hRender, INFINITE );
		CloseHandle( m_hRender );
		m_hRender = NULL;
	}

   
    SafeReleaseEx(m_pOffsceenSurface);
    SafeReleaseEx(m_pTextureSurface);
    SafeReleaseEx(m_pTexture);
    SafeReleaseEx(m_pVertexBuffer);
    SafeReleaseEx(m_pVertexShader); 
    SafeReleaseEx(m_pVertexConstantTable); 
    SafeReleaseEx(m_pPixelConstantTable); 
    SafeReleaseEx(m_pPixelShader);
	SafeReleaseEx(m_pRGBTextureSurface);
	SafeReleaseEx(m_pTextTexture);

	m_overlays.RemoveAll();

    SafeReleaseEx(m_pDevice);
    SafeReleaseEx(m_pD3D9);
  
	if( m_pDCShelter != NULL )
	{
		m_pDCShelter->DeleteDC();
		delete m_pDCShelter;
		m_pDCShelter = NULL;
	}

	if( m_pPenGreen != NULL )
	{
		m_pPenGreen->DeleteObject();
		delete m_pPenGreen;
		m_pPenGreen = NULL;
	}

	if( m_pPenRed != NULL )
	{
		m_pPenRed->DeleteObject();
		delete m_pPenRed;
		m_pPenRed   = NULL;
	}

	if( m_hEvent != NULL )
	{
		CloseHandle( m_hEvent );
		m_hEvent = NULL;
	}

	if( m_pIABuf != NULL )
	{
		delete[] m_pIABuf;
		m_pIABuf = NULL;
	}

	DeleteCriticalSection( &m_section );
}


HRESULT D3D9RenderHK::Reset( )
{
	HRESULT hr = S_OK;

	DiscardResources();

	 D3DCAPS9 deviceCaps;
    HR(m_pD3D9->GetDeviceCaps(m_deviceIndex, D3DDEVTYPE_HAL, &deviceCaps));

    DWORD dwBehaviorFlags = D3DCREATE_MULTITHREADED;

    if( deviceCaps.VertexProcessingCaps != 0 )
        dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else
        dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	dwBehaviorFlags |= D3DCREATE_DISABLE_DRIVER_MANAGEMENT;
    
    HR(GetPresentParams(&m_presentParams, TRUE));
    
	//theApp.D3DLock();
    hr = m_pD3D9->CreateDevice(m_deviceIndex, D3DDEVTYPE_HAL, m_hDisplayWindow, dwBehaviorFlags, &m_presentParams, &m_pDevice);
	//theApp.D3DUnlock();

	if( hr != S_OK )
	{
		OutputDebugString( "====Create D3D object failed ===123==\r\n");
		return hr;
	}

#ifndef USE_SURFACE
    return CreateResources();
#else
#if 1
	HR(m_pDevice->CreateRenderTarget( m_presentParams.BackBufferWidth,
									  m_presentParams.BackBufferHeight,
									  m_displayMode.Format,
									  D3DMULTISAMPLE_NONE,
									  0,
									  TRUE,
									  &m_pRGBTextureSurface, NULL ));
#endif
#endif

	return hr;
}

HRESULT D3D9RenderHK::CheckFormatConversion(D3DFORMAT format)
{
    HR(m_pD3D9->CheckDeviceFormat(m_deviceIndex, D3DDEVTYPE_HAL, m_displayMode.Format, 0, D3DRTYPE_SURFACE, format));
    
    return m_pD3D9->CheckDeviceFormatConversion(m_deviceIndex, D3DDEVTYPE_HAL, format, m_displayMode.Format);
}


HRESULT D3D9RenderHK::CreateRenderTarget(int width, int height)
{
	HRESULT hr = S_OK;
	HR(m_pDevice->CreateRenderTarget( width, height, m_displayMode.Format,  D3DMULTISAMPLE_NONE, 0, TRUE, &m_pRGBTextureSurface, NULL ));
    HR(m_pDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, m_displayMode.Format, D3DPOOL_DEFAULT, &m_pTexture, NULL));
	if( m_pTexture == NULL )
	{
		TRACE("cann't create texture");
	}
    HR(m_pTexture->GetSurfaceLevel(0, &m_pTextureSurface));
    HR(m_pDevice->CreateVertexBuffer(sizeof(VERTEX) * 4, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL));
    
    VERTEX vertexArray[] =
    {
        { D3DXVECTOR3(0, 0, 0),          D3DCOLOR_ARGB(255, 255, 255, 255), D3DXVECTOR2(0, 0) },  // top left
        { D3DXVECTOR3(width, 0, 0),      D3DCOLOR_ARGB(255, 255, 255, 255), D3DXVECTOR2(1, 0) },  // top right
        { D3DXVECTOR3(width, height, 0), D3DCOLOR_ARGB(255, 255, 255, 255), D3DXVECTOR2(1, 1) },  // bottom right
        { D3DXVECTOR3(0, height, 0),     D3DCOLOR_ARGB(255, 255, 255, 255), D3DXVECTOR2(0, 1) },  // bottom left
    };

    VERTEX *vertices;
    HR(m_pVertexBuffer->Lock(0, 0, (void**)&vertices, D3DLOCK_DISCARD));

    memcpy(vertices, vertexArray, sizeof(vertexArray));

    return m_pVertexBuffer->Unlock();
}

HRESULT D3D9RenderHK::Display(BYTE* pYplane, BYTE* pVplane, BYTE* pUplane, int width, int height, int strideY, int strideU, int strideV)
{
	HRESULT hr = S_OK;

    if(!pYplane)
    {
        return E_POINTER;
    }

    if(m_format == D3DFMT_NV12 && !pVplane)
    {
        return E_POINTER;
    }

    if(m_format == D3DFMT_YV12 && (!pVplane || !pUplane))
    {
        return E_POINTER;
    }

	if( S_OK != CheckDevice( ) )
	{
		if( S_OK != Reset( ) )
		{
			return S_FALSE;
		}

		m_videoWidth  = width;
		m_videoHeight = height;

		hr = CreateVideoSurface( m_videoWidth,  m_videoHeight, D3DFMT_YV12 );
		if( hr != S_OK )
		{
			m_videoWidth = 0;
			m_videoHeight = 0;
			return hr;
		}
	}

	//判断窗口有无变化，若变化则重新初始化
	{
		RECT rc = {0, };

		::GetClientRect( m_hDisplayWindow, &rc );

		int wndWidth = rc.right - rc.left;
		int wndHeight = rc.bottom - rc.top;

		m_rcDes = rc;
		

		if( m_wndWidth != wndWidth || m_wndHeight != wndHeight )
		{
			if( S_OK != Reset( ) )
			{
				return S_FALSE;
			}

			m_wndWidth  = wndWidth;
			m_wndHeight = wndHeight;

			m_videoWidth  = width;
			m_videoHeight = height;

			hr = CreateVideoSurface( m_videoWidth,  m_videoHeight, D3DFMT_YV12 );
			if( hr != S_OK )
			{
				m_videoWidth = 0;
				m_videoHeight = 0;

				return hr;
			}
		}

	}

	//判断视频宽高是否一致
	{
		if( m_videoWidth != width || m_videoHeight != height )
		{
			if( S_OK != Reset( ) )
			{
				return S_FALSE;
			}

			//D3DFMT_YV12

			hr = CreateVideoSurface( width,  height, D3DFMT_YV12 );
			if( hr != S_OK )
			{
				return hr;	
			}

			m_videoWidth  = width;
			m_videoHeight = height;
		}

	}

    HR(CheckDevice());
    HR(FillBuffer(pYplane, pVplane, pUplane, strideY, strideU, strideV));

#ifndef USE_SURFACE
    HR(CreateScene());
#endif

    hr = Present();

	return hr;
}

HRESULT D3D9RenderHK::GetVideoPt( int width, int height, BYTE **dst, int &pitch )
{
	HRESULT hr = S_FALSE;


	do
	{
    SelfLock();

		if( S_OK != CheckDevice( ) )
		{
			if( S_OK != Reset( ) )
			{
				hr = S_FALSE;
				break;
			}

			m_videoWidth  = width;
			m_videoHeight = height;

			hr = CreateVideoSurface( m_videoWidth,  m_videoHeight, D3DFMT_YV12 );
			if( hr != S_OK )
			{
				m_videoWidth = 0;
				m_videoHeight = 0;
				break;
			}
		}

		//判断窗口有无变化，若变化则重新初始化
		{
			RECT rc = {0, };

			::GetClientRect( m_hDisplayWindow, &rc );

			int wndWidth = rc.right - rc.left;
			int wndHeight = rc.bottom - rc.top;
			m_rcDes = rc;

			if( m_wndWidth != wndWidth || m_wndHeight != wndHeight )
			{
				if( S_OK != Reset( ) )
				{
					hr = S_FALSE;
					break;
				}

				m_wndWidth  = wndWidth;
				m_wndHeight = wndHeight;

				m_videoWidth  = width;
				m_videoHeight = height;

				if( S_OK != CreateVideoSurface( m_videoWidth,  m_videoHeight, D3DFMT_YV12 ) )
				{
					m_videoWidth = 0;
					m_videoHeight = 0;
					hr = S_FALSE;
					break;
				}
			}

		}

		//判断视频宽高是否一致
		{
			if( m_videoWidth != width || m_videoHeight != height )
			{
				if( S_OK != Reset( ) )
				{
					hr = S_FALSE;
					break;
				}

				//D3DFMT_YV12

				if( S_OK != CreateVideoSurface( width,  height, D3DFMT_YV12 ) )
				{
					hr = S_FALSE;
					break;	
				}

				m_videoWidth  = width;
				m_videoHeight = height;
			}

		}

		if( CheckDevice() == S_FALSE )
		{
			hr = S_FALSE;
			break;
		}
		
		D3DSURFACE_DESC desc;
		hr = m_pOffsceenSurface->GetDesc( &desc );
		if( hr != S_OK )
		{
			break;
		}

		if( desc.Width < width || desc.Height < height )
		{
			hr = S_FALSE;
			break;
		}

		D3DLOCKED_RECT d3drect;
		hr = m_pOffsceenSurface->LockRect(&d3drect, NULL, D3DLOCK_NOSYSLOCK);

		if( hr!= S_OK )
		{
			break;
		}

		*dst  = (BYTE*)d3drect.pBits;
		pitch = d3drect.Pitch;

	}while( false );

	if( hr != S_OK )
	{
		SelfUnLock();
	}
	
	return hr;
}

HRESULT D3D9RenderHK::ReleaseVideoPt( )
{
	if( m_pOffsceenSurface != NULL )
	{
		m_pOffsceenSurface->UnlockRect();
	}

#ifndef USE_SURFACE
	CreateScene();
#endif

	SelfUnLock();

	SetEvent( m_hEvent );

	return S_OK;
   // return Present();
}

HRESULT D3D9RenderHK::ReleaseVideoPt( RECT srcRect, bool bShow )
{
	HRESULT hr = S_FALSE;

	do
	{
		if( m_pOffsceenSurface != NULL )
		{
			hr = m_pOffsceenSurface->UnlockRect();
		}


		if( srcRect.right - srcRect.left == 0 || srcRect.bottom - srcRect.top == 0 )
		{
			//
			break;
		}
		
		m_srcRect = srcRect;

#ifndef USE_SURFACE
		CreateScene();
#endif
		
	}while( false );

    //return Present( srcRect );

	SelfUnLock();

	SetEvent( m_hEvent );

	return hr;
}

HRESULT D3D9RenderHK::SetupMatrices(int width, int height)
{
    D3DXMATRIX matOrtho; 
    D3DXMATRIX matIdentity;

    D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, width, height, 0, 0.0f, 1.0f);
    D3DXMatrixIdentity(&matIdentity);

    HR(m_pDevice->SetTransform(D3DTS_PROJECTION, &matOrtho));
    HR(m_pDevice->SetTransform(D3DTS_WORLD, &matIdentity));
    return m_pDevice->SetTransform(D3DTS_VIEW, &matIdentity);
}

HRESULT D3D9RenderHK::CreateScene(void)
{
    HRESULT hr = m_pDevice->Clear(m_deviceIndex, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    HR(m_pDevice->BeginScene());	

    hr = m_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
    if(FAILED(hr))
    {
        m_pDevice->EndScene();
        return hr;
    }

    hr = m_pDevice->SetVertexShader(m_pVertexShader);
    if(FAILED(hr))
    {
        m_pDevice->EndScene();
        return hr;
    }
    
    hr = m_pDevice->SetPixelShader(m_pPixelShader);
    if(FAILED(hr))
    {
        m_pDevice->EndScene();
        return hr;
    }

    hr = m_pDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(VERTEX));
    if(FAILED(hr))
    {
        m_pDevice->EndScene();
        return hr;
    }

    hr = m_pDevice->SetTexture(0, m_pTexture);
    if(FAILED(hr))
    {
        m_pDevice->EndScene();
        return hr;
    }

    hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
    if(FAILED(hr))
    {
        m_pDevice->EndScene();
        return hr;
    }

    //m_overlays.Draw();

	DrawTitle();

    return m_pDevice->EndScene();
}


#ifndef USE_SURFACE

HRESULT D3D9RenderHK::CheckDevice(void)
{
	HRESULT hr = S_OK;

	if( m_pDevice == NULL )
	{
		return S_FALSE;
	}

    hr = m_pDevice->TestCooperativeLevel();
	if( hr == D3DERR_DEVICELOST )
	{
		OutputDebugString( "Device Lost!\r\n");
	}else if( hr == D3DERR_DEVICENOTRESET )
	{
		OutputDebugString( "Device reset!\r\n");
		/*
		m_pOffsceenSurface;
		CComPtr<IDirect3DSurface9>      m_pTextureSurface;
		CComPtr<IDirect3DSurface9>      m_pRGBTextureSurface;
		CComPtr<IDirect3DTexture9>      m_pTexture;
		*/
	}else if( m_pRGBTextureSurface == NULL || m_pTexture == NULL || m_pOffsceenSurface == NULL || m_pTextureSurface == NULL )
	{
		hr = D3DERR_DEVICELOST;
	}

	return hr;
	
}

#else

HRESULT D3D9RenderHK::CheckDevice(void)
{
	HRESULT hr = S_OK;

	if( m_pDevice == NULL )
	{
		return S_FALSE;
	}

    hr = m_pDevice->TestCooperativeLevel();
	if( hr == D3DERR_DEVICELOST )
	{
		OutputDebugString( "Device Lost!\r\n");
	}else if( hr == D3DERR_DEVICENOTRESET )
	{
		OutputDebugString( "Device reset!\r\n");
	
	}else if( m_pOffsceenSurface == NULL /*|| m_pRGBTextureSurface == NULL*/ )
	{
		hr = D3DERR_DEVICELOST;
	}

	return hr;
	
}

#endif


HRESULT D3D9RenderHK::DiscardResources()
{
    SafeReleaseEx(m_pOffsceenSurface);
    SafeReleaseEx(m_pTextureSurface);
    SafeReleaseEx(m_pTexture);
    SafeReleaseEx(m_pVertexBuffer);
    SafeReleaseEx(m_pVertexShader); 
    SafeReleaseEx(m_pVertexConstantTable); 
    SafeReleaseEx(m_pPixelConstantTable); 
    SafeReleaseEx(m_pPixelShader);
	SafeReleaseEx(m_pRGBTextureSurface);
	SafeReleaseEx(m_pTextTexture);

    m_overlays.RemoveAll();
	SafeReleaseEx(m_pDevice);

    return S_OK;
}

HRESULT D3D9RenderHK::CreateResources()
{
    m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
    m_pDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE);
    m_pDevice->SetRenderState( D3DRS_LIGHTING, FALSE);
    m_pDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE);

    m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_SPECULAR);

    m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	OutputDebugString( "====Create D3D object failed ==3333===\r\n");
    HR(CreateRenderTarget(m_presentParams.BackBufferWidth, m_presentParams.BackBufferHeight));
    OutputDebugString( "====Create D3D object failed ===4444==\r\n");

	//D3DXCreateFont( m_pDevice, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, CMediaEx::s_ftTitle.lfFaceName, &m_pTextTexture );

    return SetupMatrices(m_presentParams.BackBufferWidth, m_presentParams.BackBufferHeight);
}

static inline void ApplyLetterBoxing(RECT& rendertTargetArea, FLOAT frameWidth, FLOAT frameHeight)
{
    const float aspectRatio = frameWidth / frameHeight;

    const float targetW = fabs((FLOAT)(rendertTargetArea.right - rendertTargetArea.left));
    const float targetH = fabs((FLOAT)(rendertTargetArea.bottom - rendertTargetArea.top));

    float tempH = targetW / aspectRatio;    
            
    if(tempH <= targetH)
    {               
        float deltaH = fabs(tempH - targetH) / 2;
        rendertTargetArea.top += deltaH;
        rendertTargetArea.bottom -= deltaH;
    }
    else
    {
        float tempW = targetH * aspectRatio;    
        float deltaW = fabs(tempW - targetW) / 2;

        rendertTargetArea.left += deltaW;
        rendertTargetArea.right -= deltaW;
    }
}

HRESULT D3D9RenderHK::FillBuffer(BYTE* pY, BYTE* pV, BYTE* pU, int strideY, int strideU, int strideV)
{
	HRESULT hr = S_OK;

    D3DLOCKED_RECT d3drect;
    hr = m_pOffsceenSurface->LockRect(&d3drect, NULL, D3DLOCK_NOSYSLOCK);
	if( hr != S_OK )
	{
		hr = m_pOffsceenSurface->UnlockRect();
		DiscardResources();
		return hr;
	}

    int newHeight  = m_videoHeight;
    int newWidth  = m_videoWidth;

    BYTE* pict = (BYTE*)d3drect.pBits;
    BYTE* Y = pY;
    BYTE* V = pV;
    BYTE* U = pU;

    switch(m_format)
    {
        case D3DFMT_YV12:

            for (int y = 0 ; y < newHeight ; y++)
            {
                memcpy(pict, Y, newWidth);
                pict += d3drect.Pitch;
                Y += strideY;
            }

            for (int y = 0 ; y < newHeight / 2 ; y++)
            {
                memcpy(pict, V, newWidth / 2);
                pict += d3drect.Pitch / 2;
                V += strideV;
            }

            for (int y = 0 ; y < newHeight / 2; y++)
            {
                memcpy(pict, U, newWidth / 2);
                pict += d3drect.Pitch / 2;
                U += strideU;
            }	

            break;

        case D3DFMT_NV12:
            
            for (int y = 0 ; y < newHeight ; y++)
            {
                memcpy(pict, Y, newWidth);
                pict += d3drect.Pitch;
                Y += strideY;
            }
            for (int y = 0 ; y < newHeight / 2 ; y++)
            {
                memcpy(pict, V, newWidth);
                pict += d3drect.Pitch;
                V += strideV;
            }
            break;

        case D3DFMT_YUY2:
        case D3DFMT_UYVY:
        case D3DFMT_R5G6B5:
        case D3DFMT_X1R5G5B5:
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:

            memcpy(pict, Y, d3drect.Pitch * newHeight);

            break;
    }
     
    return  m_pOffsceenSurface->UnlockRect();
}

#ifndef USE_SURFACE

HRESULT D3D9RenderHK::Present(void)
{
	HDC hdc = NULL;
	HRESULT hr = S_OK;
    RECT rect;
	
    ::GetClientRect(m_hDisplayWindow, &rect);

    if(m_fillMode == KeepAspectRatio)
    {	
        //ApplyLetterBoxing(rect, (FLOAT)m_videoWidth, (FLOAT)m_videoHeight);
    }

    //HR(m_pDevice->ColorFill(m_pTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0)));
    //HR(m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &rect, D3DTEXF_LINEAR));

#if 1
	//hr = m_pDevice->ColorFill(m_pRGBTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0) );

	hr = m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pRGBTextureSurface, NULL, D3DTEXF_LINEAR);

	/*
	RECT imageRect;
	imageRect.left = m_videoWidth/2;
	imageRect.right = m_videoWidth;
	imageRect.top  = m_videoHeight/2;
	imageRect.bottom = m_videoHeight;

	RECT dest;
	dest.left = rect.right - 100;
	dest.right = rect.right;
	dest.top   = rect.bottom - 80;
	dest.bottom = rect.bottom;

	hr = m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pRGBTextureSurface, &dest,D3DTEXF_LINEAR );
	*/
	m_pRGBTextureSurface->GetDC( &hdc );
	if( hdc != NULL )
	{
		//绘制遮挡
		if (m_bShelter && m_pDCShelter != NULL)
		{
			RECT rc;
			rc.left   = m_rcShelter.left   * m_rcDes.right  / 100;
			rc.right  = m_rcShelter.right  * m_rcDes.right  / 100;
			rc.top    = m_rcShelter.top    * m_rcDes.bottom / 100;
			rc.bottom = m_rcShelter.bottom * m_rcDes.bottom / 100;
			StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top
				, m_pDCShelter->GetSafeHdc(), 0, 0, CIFF_XDIM, CIFF_YDIM, SRCCOPY);
		}

		//绘制状态
		DrawStatusFlag( hdc, &m_rcDes );

		if( m_bShowIAInfo && m_pIAFrame != NULL )
		{
			CDC cdc;
			if( cdc.Attach( hdc ) )
			{
				DrawIAInfo( &cdc, m_pIAFrame, &m_rcDes );
				cdc.Detach();
			}
		}

		m_pRGBTextureSurface->ReleaseDC( hdc );
	}
	

	hr = m_pDevice->StretchRect(m_pRGBTextureSurface, NULL, m_pTextureSurface, &rect, D3DTEXF_LINEAR);


#endif

    return m_pDevice->Present(NULL, NULL, NULL, NULL);
}


HRESULT D3D9RenderHK::Present( RECT srcRect )
{
	HDC hdc = NULL;
	HRESULT hr = S_OK;
    RECT rect;
	
    ::GetClientRect(m_hDisplayWindow, &rect);

    if(m_fillMode == KeepAspectRatio)
    {	
        //ApplyLetterBoxing(rect, (FLOAT)m_videoWidth, (FLOAT)m_videoHeight);
    }

    //HR(m_pDevice->ColorFill(m_pTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0)));
    //HR(m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &rect, D3DTEXF_LINEAR));

#if 1
	//hr = m_pDevice->ColorFill(m_pRGBTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0) );

	hr = m_pDevice->StretchRect(m_pOffsceenSurface, &srcRect, m_pRGBTextureSurface, NULL, D3DTEXF_LINEAR);

#if 0
	RECT imageRect;
	imageRect.left = m_videoWidth/2;
	imageRect.right = m_videoWidth;
	imageRect.top  = m_videoHeight/2;
	imageRect.bottom = m_videoHeight;

	RECT dest;
	dest.left = rect.right - 100;
	dest.right = rect.right;
	dest.top   = rect.bottom - 80;
	dest.bottom = rect.bottom;

	hr = m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pRGBTextureSurface, &dest,D3DTEXF_LINEAR );
#endif

	
#if 1
	m_pRGBTextureSurface->GetDC( &hdc );
	if( hdc != NULL )
	{
		//绘制遮挡
		if (m_bShelter && m_pDCShelter != NULL)
		{
			RECT rc;
			rc.left   = m_rcShelter.left   * m_rcDes.right  / 100;
			rc.right  = m_rcShelter.right  * m_rcDes.right  / 100;
			rc.top    = m_rcShelter.top    * m_rcDes.bottom / 100;
			rc.bottom = m_rcShelter.bottom * m_rcDes.bottom / 100;
			StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top
				, m_pDCShelter->GetSafeHdc(), 0, 0, CIFF_XDIM, CIFF_YDIM, SRCCOPY);
		}

		//绘制状态
		DrawStatusFlag( hdc, &m_rcDes );

		if( m_bShowIAInfo && m_pIAFrame != NULL )
		{
			CDC cdc;
			if( cdc.Attach( hdc ) )
			{
				DrawIAInfo( &cdc, m_pIAFrame, &m_rcDes );
				cdc.Detach();
			}
		}

		m_pRGBTextureSurface->ReleaseDC( hdc );
	}
#endif
	

	hr = m_pDevice->StretchRect(m_pRGBTextureSurface, NULL, m_pTextureSurface, &rect, D3DTEXF_LINEAR);


#endif

    return m_pDevice->Present(NULL, NULL, NULL, NULL);
}

#else


HRESULT D3D9RenderHK::Present(void)
{
	HDC hdc = NULL;
	HRESULT hr = S_OK;
    RECT rect;
	
    ::GetClientRect(m_hDisplayWindow, &rect);

    if(m_fillMode == KeepAspectRatio)
    {	
        //ApplyLetterBoxing(rect, (FLOAT)m_videoWidth, (FLOAT)m_videoHeight);
    }

	IDirect3DSurface9 * pBackBuffer = NULL;
	

	do
	{
		if( m_pDevice == NULL ||m_pOffsceenSurface == NULL ||m_pRGBTextureSurface == NULL)
		{
			break;
		}

		hr = m_pDevice->BeginScene();
		if( hr != S_OK )
		{
			break;
		}
		
		hr = m_pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer);
		if( hr != S_OK )
		{
			m_pDevice->EndScene();
			break;
		}

		if( m_srcRect.right - m_srcRect.left != 0 && m_srcRect.bottom - m_srcRect.top != 0 )
		{
			hr = m_pDevice->StretchRect(m_pOffsceenSurface, &m_srcRect, m_pRGBTextureSurface, NULL,D3DTEXF_LINEAR);
		}else
		{
			hr = m_pDevice->StretchRect(m_pOffsceenSurface,NULL, m_pRGBTextureSurface, NULL,D3DTEXF_LINEAR);
		}
			
		m_pRGBTextureSurface->GetDC( &hdc );

		if( hdc != NULL )
		{
			//绘制遮挡
			if (m_bShelter && m_pDCShelter != NULL)
			{
				RECT rc;
				rc.left   = m_rcShelter.left   * m_rcDes.right  / 100;
				rc.right  = m_rcShelter.right  * m_rcDes.right  / 100;
				rc.top    = m_rcShelter.top    * m_rcDes.bottom / 100;
				rc.bottom = m_rcShelter.bottom * m_rcDes.bottom / 100;
				StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top
					, m_pDCShelter->GetSafeHdc(), 0, 0, CIFF_XDIM, CIFF_YDIM, SRCCOPY);
			}

			//绘制状态
			DrawStatusFlag( hdc, &m_rcDes );

			if( m_bShowIAInfo && m_pIAFrame != NULL )
			{
				CDC cdc;
				if( cdc.Attach( hdc ) )
				{
					DrawIAInfo( &cdc, m_pIAFrame, &m_rcDes );
					cdc.Detach();
				}
			}

			if( m_pOwner != NULL )
			{
				//对于hk设备，依然采用原来的方式显示字符
				CMediaEx *pMedia = (CMediaEx*)m_pOwner;
				pMedia->DrawCustomDC(hdc);
			}

			m_pRGBTextureSurface->ReleaseDC( hdc );
		}

		//hr = m_pDevice->StretchRect(m_pOffsceenSurface,NULL, pBackBuffer, NULL,D3DTEXF_LINEAR);
		hr = m_pDevice->StretchRect(m_pRGBTextureSurface,NULL, pBackBuffer, NULL,D3DTEXF_LINEAR);

		pBackBuffer->Release();

		if( m_pOwner == NULL )
		{
			DrawTitle();
		}

		m_pDevice->EndScene();

		hr = m_pDevice->Present(NULL, NULL, NULL, NULL);

	}while( false );

	return hr;
}


HRESULT D3D9RenderHK::Present( RECT srcRect )
{
	HDC hdc = NULL;
	HRESULT hr = S_OK;
    RECT rect;
	
    ::GetClientRect(m_hDisplayWindow, &rect);

    if(m_fillMode == KeepAspectRatio)
    {	
        //ApplyLetterBoxing(rect, (FLOAT)m_videoWidth, (FLOAT)m_videoHeight);
    }

   	IDirect3DSurface9 * pBackBuffer = NULL;
	hr = m_pDevice->BeginScene();

	do
	{
		if( hr != S_OK )
		{
			break;
		}
		
		hr = m_pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer);
		if( hr != S_OK )
		{
			m_pDevice->EndScene();
			break;
		}

		hr = m_pDevice->StretchRect(m_pOffsceenSurface, &srcRect, m_pRGBTextureSurface, &rect,D3DTEXF_LINEAR);
				
		
		m_pRGBTextureSurface->GetDC( &hdc );

		if( hdc != NULL )
		{
			//绘制遮挡
			if (m_bShelter && m_pDCShelter != NULL)
			{
				RECT rc;
				rc.left   = m_rcShelter.left   * m_rcDes.right  / 100;
				rc.right  = m_rcShelter.right  * m_rcDes.right  / 100;
				rc.top    = m_rcShelter.top    * m_rcDes.bottom / 100;
				rc.bottom = m_rcShelter.bottom * m_rcDes.bottom / 100;
				StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top
					, m_pDCShelter->GetSafeHdc(), 0, 0, CIFF_XDIM, CIFF_YDIM, SRCCOPY);
			}

			//绘制状态
			DrawStatusFlag( hdc, &m_rcDes );

			if( m_bShowIAInfo && m_pIAFrame != NULL )
			{
				CDC cdc;
				if( cdc.Attach( hdc ) )
				{
					DrawIAInfo( &cdc, m_pIAFrame, &m_rcDes );
					cdc.Detach();
				}
			}

			if( m_pOwner != NULL )
			{
				//对于hk设备，依然采用原来的方式显示字符
				CMediaEx *pMedia = (CMediaEx*)m_pOwner;
				pMedia->DrawCustomDC(hdc);
			}


			m_pRGBTextureSurface->ReleaseDC( hdc );
		}

		hr = m_pDevice->StretchRect(m_pRGBTextureSurface,NULL, pBackBuffer, NULL,D3DTEXF_LINEAR);

		pBackBuffer->Release();

		if( m_pOwner == NULL )
		{
			//蓝星设备
			DrawTitle();
		}

		m_pDevice->EndScene();

		

		hr = m_pDevice->Present(NULL, NULL, NULL, NULL);

	}while( false );

	return hr;
}


#endif


HRESULT D3D9RenderHK::CreateVideoSurface(int width, int height, D3DFORMAT format)
{
    m_videoWidth = width;
    m_videoHeight = height;
    m_format = format;

	if( m_pDevice == NULL )
	{
		return S_FALSE;
	}

	if( m_pOffsceenSurface != NULL )
	{
		m_pOffsceenSurface->Release();
		m_pOffsceenSurface = NULL;
	}
	
    HR(m_pDevice->CreateOffscreenPlainSurface(width, height, format , D3DPOOL_DEFAULT, &m_pOffsceenSurface, NULL));

    return m_pDevice->ColorFill(m_pOffsceenSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0));
}


HRESULT D3D9RenderHK::GetPresentParams(D3DPRESENT_PARAMETERS* params, BOOL bWindowed)
{
    UINT height, width;

    if(bWindowed) // windowed mode
    {
        RECT rect;
        ::GetClientRect(m_hDisplayWindow, &rect);
        height = rect.bottom - rect.top;
        width = rect.right - rect.left;
    }
    else   // fullscreen mode
    {
        width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }

    D3DPRESENT_PARAMETERS presentParams = {0};
    presentParams.Flags                  = D3DPRESENTFLAG_DEVICECLIP;
    presentParams.Windowed               = bWindowed;
    presentParams.hDeviceWindow          = m_hDisplayWindow;
    presentParams.BackBufferWidth        = width;
    presentParams.BackBufferHeight       = height;
    presentParams.SwapEffect             = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;
    presentParams.MultiSampleType        = D3DMULTISAMPLE_NONE;
    presentParams.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;//D3DPRESENT_INTERVAL_DEFAULT;
    presentParams.BackBufferFormat       = m_displayMode.Format;
    presentParams.BackBufferCount        = 1;
    presentParams.EnableAutoDepthStencil = FALSE;

    memcpy(params, &presentParams, sizeof(D3DPRESENT_PARAMETERS));

    return S_OK;
}

HRESULT D3D9RenderHK::SetVertexShader(LPCSTR pVertexShaderName, LPCSTR entryPoint, LPCSTR shaderModel, LPSTR* ppError)
{
    //CComPtr<ID3DXBuffer> code;
    //CComPtr<ID3DXBuffer> errMsg;
	ID3DXBuffer*         code;
	ID3DXBuffer*		 errMsg;

    HRESULT hr = D3DXCompileShaderFromFile(pVertexShaderName, NULL, NULL, entryPoint, shaderModel, 0, &code, &errMsg, &m_pVertexConstantTable);
    if (FAILED(hr))
    {	
        if(errMsg != NULL)
        {
            size_t len = errMsg->GetBufferSize() + 1;
            *ppError = new CHAR[len];		
            memcpy(*ppError, errMsg->GetBufferPointer(), len);			
        }
        return hr;
    }

    return m_pDevice->CreateVertexShader((DWORD*)code->GetBufferPointer(), &m_pVertexShader);
}

HRESULT D3D9RenderHK::ApplyWorldViewProj(LPCSTR matrixName)
{
    D3DXMATRIX matOrtho;
    HR(m_pDevice->GetTransform(D3DTS_PROJECTION, &matOrtho));

    return m_pVertexConstantTable->SetMatrix(m_pDevice, matrixName, &matOrtho);
}

HRESULT D3D9RenderHK::SetVertexShader(DWORD* buffer)
{
    HR(D3DXGetShaderConstantTable(buffer, &m_pVertexConstantTable));

    return m_pDevice->CreateVertexShader(buffer, &m_pVertexShader);
}

HRESULT D3D9RenderHK::SetVertexShaderConstant(LPCSTR name, LPVOID value, UINT size)
{
    return m_pVertexConstantTable->SetValue(m_pDevice, name, value, size);
}

HRESULT D3D9RenderHK::SetPixelShader(LPCSTR pPixelShaderName, LPCSTR entryPoint, LPCSTR shaderModel, LPSTR* ppError)
{
   // CComPtr<ID3DXBuffer> code;
    //CComPtr<ID3DXBuffer> errMsg;
	ID3DXBuffer*          code;
	ID3DXBuffer*		  errMsg;

    HRESULT hr = D3DXCompileShaderFromFile(pPixelShaderName, NULL, NULL, entryPoint, shaderModel, 0, &code, &errMsg, &m_pPixelConstantTable);
    if (FAILED(hr))
    {	
        if(errMsg != NULL)
        {
            size_t len = errMsg->GetBufferSize() + 1;
            *ppError = new CHAR[len];		
            memcpy(*ppError, errMsg->GetBufferPointer(), len);	
        }
        return hr;
    }

    return m_pDevice->CreatePixelShader((DWORD*)code->GetBufferPointer(), &m_pPixelShader);
}

HRESULT D3D9RenderHK::SetPixelShader(DWORD* buffer)
{
    HR(D3DXGetShaderConstantTable(buffer, &m_pPixelConstantTable));

    return m_pDevice->CreatePixelShader(buffer, &m_pPixelShader);
}

HRESULT D3D9RenderHK::SetPixelShaderConstant(LPCSTR name, LPVOID value, UINT size)
{
    return m_pPixelConstantTable->SetValue(m_pDevice, name, value, size);
}

HRESULT D3D9RenderHK::SetVertexShaderMatrix(D3DXMATRIX* matrix, LPCSTR name)
{
    return m_pVertexConstantTable->SetMatrix(m_pDevice, name, matrix);
}

HRESULT D3D9RenderHK::SetPixelShaderMatrix(D3DXMATRIX* matrix, LPCSTR name)
{
    return m_pPixelConstantTable->SetMatrix(m_pDevice, name, matrix);
}
    
HRESULT D3D9RenderHK::SetVertexShaderVector(D3DXVECTOR4* vector, LPCSTR name)
{
    return m_pVertexConstantTable->SetVector(m_pDevice, name, vector);
}
    
HRESULT D3D9RenderHK::SetPixelShaderVector(D3DXVECTOR4* vector, LPCSTR name)
{
    return m_pPixelConstantTable->SetVector(m_pDevice, name, vector);
}


HRESULT D3D9RenderHK::CaptureDisplayFrame(BYTE* pBuffer, INT* width, INT* height, INT* stride)
{
    //CComPtr<IDirect3DSurface9> pTargetSurface;	
    //CComPtr<IDirect3DSurface9> pTempSurface;
	IDirect3DSurface9* pTargetSurface;	
	IDirect3DSurface9* pTempSurface;

    HR(m_pDevice->GetRenderTarget(0, &pTargetSurface));	

    D3DSURFACE_DESC desc;		
    HR(pTargetSurface->GetDesc(&desc));	

    if(!pBuffer)
    {
        *width = desc.Width;
        *height = desc.Height;
        *stride = desc.Width * 4; // Always ARGB32
        return S_OK;
    }

    HR(m_pDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pTempSurface, NULL));				
    HR(m_pDevice->GetRenderTargetData(pTargetSurface, pTempSurface));					
    
    D3DLOCKED_RECT d3drect;
    HR(pTempSurface->LockRect(&d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));		
    
    BYTE* pFrame = (BYTE*)d3drect.pBits;
    BYTE* pBuf = pBuffer;
    
    memcpy(pBuf, pFrame, desc.Height * d3drect.Pitch);

    return pTempSurface->UnlockRect();
}

HRESULT D3D9RenderHK::CaptureVideoFrame(BYTE* pBuffer, INT* width, INT* height, INT* stride)
{
    if(!pBuffer)
    {
        *width = m_videoWidth;
        *height = m_videoHeight;
        *stride = m_videoWidth * 4; // Always ARGB32
        return S_OK;
    }

    //CComPtr<IDirect3DSurface9> pTempSurface;
	IDirect3DSurface9*     pTempSurface;
    HR(m_pDevice->CreateOffscreenPlainSurface(m_videoWidth, m_videoHeight, m_displayMode.Format, D3DPOOL_DEFAULT, &pTempSurface, NULL));
    HR(m_pDevice->StretchRect(m_pOffsceenSurface, NULL, pTempSurface, NULL, D3DTEXF_LINEAR))	;
    
    D3DLOCKED_RECT d3drect;
    HR(pTempSurface->LockRect(&d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));		
    
    BYTE* pFrame = (BYTE*)d3drect.pBits;
    BYTE* pBuf = pBuffer;

    memcpy(pBuf, pFrame, m_videoHeight * d3drect.Pitch);
    
    return pTempSurface->UnlockRect();
}

HRESULT D3D9RenderHK::ClearPixelShader()
{
    SafeReleaseEx(m_pPixelConstantTable); 
    SafeReleaseEx(m_pPixelShader);

    return S_OK;
}

HRESULT D3D9RenderHK::ClearVertexShader()
{
    SafeReleaseEx(m_pVertexConstantTable); 
    SafeReleaseEx(m_pVertexShader); 
    
    return S_OK;
}

void D3D9RenderHK::SetDisplayMode(FillMode mode)
{
    m_fillMode = mode;
}

FillMode D3D9RenderHK::GetDisplayMode()
{
    return m_fillMode;
}

void D3D9RenderHK::SetTitle(TitlePos iPos, LPCSTR lpTitle, DWORD dwColor, int nLine)
{
	SelfLock();

	do
	{
		int i = (int)iPos;

		if( i < 0 || i >= MAX_TITLE_POS )
		{
			break;
		}

		if( nLine < 0 || nLine >= MAX_TITLE_LINE )
		{
			break;
		}
		CString strTitle = lpTitle;
		if (m_sTitle[i][nLine] != "" && strTitle !="")
		{
			m_sTitle[i][nLine] += ",";
			m_sTitle[i][nLine] += strTitle;
		}else 
		{
			m_sTitle[i][nLine] = strTitle;
		}
		
		m_titleSize[i][nLine].cx = 0;
		m_titleSize[i][nLine].cy = 0;

		//对颜色进行转换

		DWORD val = 0xff;
		DWORD temp = val << 24;

		//取R
		val = dwColor&0xff;

		temp |= (val<<16);

		val = (dwColor >> 8)&0xff;

		temp |= (val<<8);

		val = (dwColor >> 16)&0xff;

		temp |= val;

		m_dwTitleColor[i]  = temp;


		m_bShowCaption = true;

	}while( false );

	SelfUnLock();

}

void D3D9RenderHK::ShowCaption( bool bShow )
{
	m_bShowCaption = bShow;
}


void
D3D9RenderHK::SetStatus(char c, BOOL bFlag)
{
	SelfLock();

	int iPos = m_sTitle[2][0].Find(c);
	if (bFlag)
	{
		if (iPos < 0)
		{
			m_sTitle[2][0].Insert(0, c);
		}
	}
	else
	{
		if (iPos >= 0)
		{
			m_sTitle[2][0].Delete(iPos);
		}
	}

	SelfUnLock();
}


void
D3D9RenderHK::PreTextOut(HDC hdc, CString strTitle, int &iWidth, int &iHeigth)
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

//绘制字符叠加函数
void D3D9RenderHK::OverlayDrawText(LPCSTR pText, RECT pos, DWORD color  )
{
	HRESULT hr = S_OK;

	if( (m_pTextTexture != NULL) && (pText != NULL) )
	{
		hr = m_pTextTexture->DrawText(NULL, pText, -1, &pos, 0, color );

		TEXTMETRICA text;
		m_pTextTexture->GetTextMetricsA(&text);

		int width = 0;
		int height = 0;

		HDC hdc = m_pTextTexture->GetDC();
		if( hdc != NULL )
		{
			PreTextOut( hdc, pText, width, height );

			width = 0;
		}
	}
}

void D3D9RenderHK::DrawTitle( )
{
	HRESULT hr = S_OK;

	HDC hdc = NULL; //此处需要修改

	do
	{

		if (!m_bShowCaption)
		{
			break;
		}

		if( m_pTextTexture == NULL )
		{
			D3DXCreateFont( m_pDevice, 
								 CMediaEx::s_ftTitle.lfHeight,  //height
									CMediaEx::s_ftTitle.lfWidth,//width
									FW_MEDIUM,//CMediaEx::s_ftTitle.lfWeight,                  //weight
									0,                          //Mip
									FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
									CMediaEx::s_ftTitle.lfPitchAndFamily,//DEFAULT_PITCH | FF_DONTCARE, 
									CMediaEx::s_ftTitle.lfFaceName,//font,
									&m_pTextTexture );
		}

		if( m_pTextTexture == NULL )
		{
			break;
		}
		
		int iX = 0, iY = 0;
		for (int i = 0; i < MAX_TITLE_POS; i++)
		{
			for (int j = 0;j < MAX_TITLE_LINE -1;j++)
			{
				int iLen = m_sTitle[i][j].GetLength();
			if (iLen > 0)
			{
				int iwidth = 0;
				int iheigth = 0;
					
				iwidth = m_titleSize[i][j].cx;
				iheigth = m_titleSize[i][j].cy;

				if( iwidth == 0 || iheigth == 0 )
				{
					hdc = m_pTextTexture->GetDC();
					if( hdc == NULL )
					{
						break;
					}

					PreTextOut(hdc, m_sTitle[i][j], iwidth, iheigth);
					m_titleSize[i][j].cx = iwidth;
					m_titleSize[i][j].cy = iheigth;
				}

				switch (i)
				{
				case TP_CENTER:
					{
						RECT rect;
						rect.left = (m_rcDes.right - iwidth) >> 1;
						rect.top = ((m_rcDes.bottom - iheigth) >> 1)+(j*iheigth);
						rect.right = m_rcDes.right;
						rect.bottom = m_rcDes.bottom + (j*iheigth);
						OverlayDrawText( m_sTitle[i][j], rect, m_dwTitleColor[i] );
						break;
					}
				case TP_LEFT_TOP:
					{
						RECT rect;
						rect.left = 10;
						rect.top = 10 + (j*iheigth);
						rect.right = m_rcDes.right;
						rect.bottom = m_rcDes.bottom+ (j*iheigth);
						OverlayDrawText( m_sTitle[i][j], rect, m_dwTitleColor[i] );
						break;
					}
				case TP_RIGHT_TOP:
					{/*
					 void CMediaEx::DrawTransBitmap( HDC hdcDest,     
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
					 */
						iX = m_rcDes.right - 10 - 16;
						iY = 10;
						
						if (m_sTitle[2][0].Find('R') >= 0)
						{
							//BitBlt(hdc, iX, iY, 16, 16, m_dcStatus.GetSafeHdc(), 0, 0, SRCCOPY);
						
							iX -= 20;
						}
						if (m_sTitle[2][0].Find('A') >= 0)
						{
							//BitBlt(hdc, iX, iY, 16, 16, m_dcStatus.GetSafeHdc(), 16, 0, SRCCOPY);
							
							iX -= 20;
						}

						if (m_sTitle[2][0].Find('S') >= 0)
						{
							//BitBlt(hdc, iX, iY, 16, 16, m_dcStatus.GetSafeHdc(), 32, 0, SRCCOPY);
							
							iX -= 20;
						}
						if (m_sTitle[2][0].Find('C') >= 0)
						{
							//BitBlt(hdc, iX, iY, 16, 16, m_dcStatus.GetSafeHdc(), 48, 0, SRCCOPY);
							
							iX -= 20;
						}


						if (iX == m_rcDes.right - 10 - 16)
						{
							RECT rect; 
							rect.left = m_rcDes.right - 10 - 16 - iwidth;
							rect.top = 10 + (j*iheigth);
							rect.right = m_rcDes.right;
							rect.bottom = m_rcDes.bottom + (j*iheigth);
							OverlayDrawText( m_sTitle[i][j], rect, m_dwTitleColor[i] );
						}

						break;
					}
				case TP_LEFT_BOTTOM:
					{
						RECT rect; 
						rect.left = 10;
						rect.top = (m_rcDes.bottom - iheigth - 10) - (j*iheigth);
						rect.right = m_rcDes.right;
						rect.bottom = m_rcDes.bottom - (j*iheigth);
						OverlayDrawText( m_sTitle[i][j], rect, m_dwTitleColor[i] );
						break;
					}
				case TP_RIGHT_BOTTOM:
					{
						RECT rect; 
						rect.left = m_rcDes.right - iwidth-10 ;
						rect.top = (m_rcDes.bottom - iheigth - 20) - (j*iheigth);
						rect.right = m_rcDes.right-10;
						rect.bottom = (m_rcDes.bottom-10) - (j*iheigth);
						OverlayDrawText( m_sTitle[i][j], rect, m_dwTitleColor[i] );
						break;
					}
				default:
					{
						RECT rect;
						rect.left = 10;
						rect.top = 10 + (j*iheigth);
						rect.right = m_rcDes.right;
						rect.bottom = m_rcDes.bottom + (j*iheigth);
						OverlayDrawText( m_sTitle[i][j], rect, m_dwTitleColor[i] );
					}
					break;
				}
			}
			}
			
		}
		//DrawDomeDc(hdc, m_sDomePan, m_sDomeTilt);

	}while( false );
}



void
D3D9RenderHK::DrawTitleForCapture( HDC hdc, RECT *pRect )
{
	do
	{
		if (hdc == NULL || pRect == NULL )
		{
			break;
		}

		if (!m_bShowCaption)
		{
			break;
		}

		SetBkMode(hdc, TRANSPARENT);
		
		CFont ftCaption;

		ftCaption.CreateFontIndirectA( (const LOGFONT*)&CMediaEx::s_ftTitle );

		HGDIOBJ hFont = ftCaption.GetSafeHandle();
		HGDIOBJ hObj = ::SelectObject(hdc, ftCaption.GetSafeHandle());
		if (m_bShelter && m_pDCShelter != NULL)
		{
			RECT rc;
			rc.left   = m_rcShelter.left   * pRect->right  / 100;
			rc.right  = m_rcShelter.right  * pRect->right  / 100;
			rc.top    = m_rcShelter.top    * pRect->bottom / 100;
			rc.bottom = m_rcShelter.bottom * pRect->bottom / 100;
			StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top
				, m_pDCShelter->GetSafeHdc(), 0, 0, CIFF_XDIM, CIFF_YDIM, SRCCOPY);
		}

		//DrawTextEx(hdc, m_sTitle[i], -1, &rect,  DT_CENTER|DT_VCENTER|DT_NOCLIP, m_colorCaption);
		int iX = 0, iY = 0;
		for (int i = 0; i < MAX_TITLE_POS; i++)
		{
			DWORD color = D3DColorToRGB( m_dwTitleColor[i] );
			SetTextColor(hdc, color );

			for (int j = 0;j < MAX_TITLE_LINE -1;j++)
			{
				int iLen = m_sTitle[i][j].GetLength();
				if (iLen > 0)
				{
					int iwidth = 0;
					int iheigth = 0;
						
					PreTextOut(hdc, m_sTitle[i][j], iwidth, iheigth);

					switch (i)
					{
					case TP_CENTER:
						{
							RECT rect;
							rect.left = (pRect->right - iwidth) >> 1;
							rect.top = ((pRect->bottom - iheigth) >> 1)+(j*iheigth);
							rect.right = pRect->right;
							rect.bottom = pRect->bottom + (j*iheigth);
							DrawTextEx(hdc, m_sTitle[i][j], -1, &rect,  DT_CENTER|DT_VCENTER|DT_NOCLIP,color);
							break;
						}
					case TP_LEFT_TOP:
						{
							RECT rect;
							rect.left	= 10;
							rect.top	= 10 + (j*iheigth);
							rect.right	= pRect->right;
							rect.bottom = pRect->bottom+ (j*iheigth);
							DrawTextEx(hdc, m_sTitle[i][j], -1, &rect,  DT_LEFT|DT_VCENTER|DT_NOCLIP, color);
							break;
						}
					case TP_RIGHT_TOP:
						{/*
						 void CMediaEx::DrawTransBitmap( HDC hdcDest,     
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
						 */
							iX = pRect->right - 10 - 16;
							iY = 10;
							
							if (m_sTitle[2][0].Find('R') >= 0)
							{
								//BitBlt(hdc, iX, iY, 16, 16, m_dcStatus.GetSafeHdc(), 0, 0, SRCCOPY);
							
								iX -= 20;
							}
							if (m_sTitle[2][0].Find('A') >= 0)
							{
								//BitBlt(hdc, iX, iY, 16, 16, m_dcStatus.GetSafeHdc(), 16, 0, SRCCOPY);
								
								iX -= 20;
							}

							if (m_sTitle[2][0].Find('S') >= 0)
							{
								//BitBlt(hdc, iX, iY, 16, 16, m_dcStatus.GetSafeHdc(), 32, 0, SRCCOPY);
								
								iX -= 20;
							}
							if (m_sTitle[2][0].Find('C') >= 0)
							{
								//BitBlt(hdc, iX, iY, 16, 16, m_dcStatus.GetSafeHdc(), 48, 0, SRCCOPY);
								
								iX -= 20;
							}


							if (iX == pRect->right - 10 - 16)
							{
								RECT rect; 
								rect.left		= pRect->right - 10 - 16 - iwidth;
								rect.top		= 10 + (j*iheigth);
								rect.right		= pRect->right;
								rect.bottom		= pRect->bottom + (j*iheigth);
								DrawTextEx(hdc, m_sTitle[i][j], -1, &rect,  DT_LEFT|DT_VCENTER|DT_NOCLIP, color);
							}

							break;
						}
					case TP_LEFT_BOTTOM:
						{
							RECT rect; 
							rect.left	= 10;
							rect.top	= (pRect->bottom - iheigth - 10) - (j*iheigth);
							rect.right  = pRect->right;
							rect.bottom = pRect->bottom - (j*iheigth);
							DrawTextEx(hdc, m_sTitle[i][j], -1, &rect,  DT_LEFT|DT_VCENTER|DT_NOCLIP, color);
							break;
						}
					case TP_RIGHT_BOTTOM:
						{
							RECT rect; 
							rect.left = pRect->right - iwidth - 10 ;
							rect.top = (pRect->bottom - iheigth - 20) - (j*iheigth);
							rect.right = pRect->right-10;
							rect.bottom = (pRect->bottom-10) - (j*iheigth);
							DrawTextEx(hdc, m_sTitle[i][j], -1, &rect,  DT_LEFT|DT_VCENTER|DT_NOCLIP, color);
							break;
						}
					default:
						{
							RECT rect;
							rect.left	= 10;
							rect.top	= 10 + (j*iheigth);
							rect.right  = pRect->right;
							rect.bottom = pRect->bottom + (j*iheigth);
							DrawTextEx(hdc, m_sTitle[i][j], -1, &rect,  DT_LEFT|DT_VCENTER|DT_NOCLIP, color);
						}
						break;
					}
				}
			}
			
		}



		if( hObj != NULL )
		{
			::SelectObject( hdc, hObj );
		}

		ftCaption.DeleteObject();

	}while( false );
}


void D3D9RenderHK::DrawStatusFlag( HDC hdc, RECT *rc )
{
	if( rc == NULL )
	{
		return;
	}

	int iX = rc->right - 10 - 16;
	int iY = 10;

	HDC hsrc = g_dcStatus.GetSafeHdc();

	do
	{
		if( hsrc == NULL )
		{
			break;
		}

		for( int j = 0; j < 1; j++ )
		{
			if (m_sTitle[2][j].Find('R') >= 0)
			{
				::TransparentBlt( hdc, iX, iY, 16, 16, hsrc, 0, 0, 16, 16, g_transColor );
				iX -= 20;
			}
			if (m_sTitle[2][j].Find('A') >= 0)
			{	
				::TransparentBlt( hdc, iX, iY, 16, 16, hsrc, 16, 0, 16, 16, g_transColor );
				iX -= 20;
			}
			if (m_sTitle[2][j].Find('S') >= 0)
			{
				::TransparentBlt( hdc, iX, iY, 16, 16, hsrc, 32, 0, 16, 16, g_transColor );
				iX -= 20;
			}
			if (m_sTitle[2][j].Find('C') >= 0)
			{
				::TransparentBlt( hdc, iX, iY, 16, 16, hsrc, 48, 0, 16, 16, g_transColor );
				iX -= 20;
			}
		}

	}while( false );
}

void
D3D9RenderHK::SetShelter(BOOL bShelter, LPRECT lpRc, LPVOID lpReserved)
{
	if (bShelter)
	{
		if (lpReserved != NULL)
		{
			CBitmap bmShelter;
			bmShelter.m_hObject = (HBITMAP)::LoadImage(NULL, (LPCSTR)lpReserved, IMAGE_BITMAP, CIFF_XDIM, CIFF_YDIM, LR_LOADFROMFILE);
			SAFE_DELETE(m_pDCShelter);
			m_pDCShelter = new CDC;
			m_pDCShelter->CreateCompatibleDC(NULL);
			m_pDCShelter->SelectObject(&bmShelter);
			memcpy_s(&m_rcShelter, sizeof(RECT), lpRc, sizeof(RECT));
			m_bShelter = TRUE;
		}
	}
	else
	{
		m_bShelter = FALSE;
		memset(&m_rcShelter, 0, sizeof(RECT));
		SAFE_DELETE(m_pDCShelter);
	}
}


int 
D3D9RenderHK::SaveDC2BmpFile(const HDC hDC, const unsigned long dwWidth, const unsigned long dwHeight, 
						 const unsigned long dwVFWidth, const unsigned long dwVFHeight, const char* szFileName)
{
	int              iResult = 0;              // 本函数返回值
	int              iOldBltModeSta = 0;       // 旧缩放模式
	unsigned char   *lpBMPFileBuffer = NULL;   // 位图文件缓冲区
	unsigned char    nBitsPerPixel = 0;        // 每象素所占位数(bit)
	unsigned long    dwBMPByteSize = 0;        // 位图中像素字节大小
	unsigned long    dwWrittenSize = 0;        // 写入文件字节数
	unsigned int     nColorPlaneNum =0;        // 调色板颜色数
	HDC              hCompatibleDC = NULL;     // 与参数设备场景相兼容的设备场景
	HBITMAP          hCompatibleBitmap = NULL; // 与参数设备场景相兼容的位图
	HBITMAP          hOldBitmap = NULL;        // 设备场景中的旧位图
	HANDLE           hBmpFile = NULL;          // 位图文件句柄
	BITMAP           sBitmapInfo;              // 位图信息结构变量
	BITMAPINFOHEADER sBitmapHeader;            // 位图信息头结构
	BITMAPFILEHEADER sBMPFileHeader;           // 设置位图文件头 

	do 
	{
		/// 1.创建兼容设备场景
		hCompatibleDC = ::CreateCompatibleDC(hDC);
		if (NULL == hCompatibleDC)
		{
			iResult = -1;
			break;
		}

		// 2.创建兼容位图
		hCompatibleBitmap = ::CreateCompatibleBitmap(hDC, dwWidth, dwHeight);
		if (NULL == hCompatibleBitmap)
		{
			iResult = -2;
			break;
		}

		// 向兼容设备场景中置入兼容位图
		hOldBitmap = (HBITMAP)::SelectObject(hCompatibleDC, hCompatibleBitmap);
		if (NULL == hOldBitmap)
		{
			iResult = -3;
			break;
		}

		iOldBltModeSta = ::SetStretchBltMode(hCompatibleDC, HALFTONE);
		::SetBrushOrgEx(hCompatibleDC, 0, 0, NULL);

		// 将参数设备场景的画面缩放到兼容设备场景上去
		// StrethcBlt( 目标设备场景, 左, 上, 宽, 高, 来源设备场景, 左, 上, 宽, 高, 拷贝 )
		if (0 == ::StretchBlt(hCompatibleDC, 0, 0, dwWidth, dwHeight, hDC, 0, 0, dwVFWidth, dwVFHeight, SRCCOPY) )
		{
			iResult = -4;
			::SetStretchBltMode(hCompatibleDC, iOldBltModeSta);
			break;
		}

		// 恢复原拉伸模式
		::SetStretchBltMode(hCompatibleDC, iOldBltModeSta);

		// 将表示不同颜色的位放在不同内存平面上。
		nColorPlaneNum =::GetDeviceCaps(hCompatibleDC, PLANES);
		// 当前分辨率下每个调色板的每象素所占位数(bit)
		nBitsPerPixel = ::GetDeviceCaps(hCompatibleDC, BITSPIXEL);
		nBitsPerPixel *= nColorPlaneNum;

		// 如果每象素所占位数小于等于零的话，则
		if (0 >= nBitsPerPixel)
		{
			iResult = -6;
			break;
		} 
		else if (nBitsPerPixel <= 1) // 将每像素素位数4的整倍数化       
		{
			nBitsPerPixel = 1; 
		}
		else if (nBitsPerPixel <= 4)  
		{
			nBitsPerPixel = 4; 
		}
		else if (nBitsPerPixel <= 8)  
		{
			nBitsPerPixel = 8; 
		}
		else if (nBitsPerPixel <= 24)  
		{
			nBitsPerPixel = 24; 
		}
		else
		{
			nBitsPerPixel = 32; 
		}

		ZeroMemory(&sBitmapInfo, sizeof(sBitmapInfo));

		// 如果取得位图信息失败的话，则
		if (0 == ::GetObject(hCompatibleBitmap, sizeof(sBitmapInfo), (LPSTR)&sBitmapInfo))
		{
			iResult = -7;
			break;
		}

		// 算出四字节倍数宽度的位图数据区大小 = 四字节的像素 * 图像宽为四的倍数 * 图像的高度
		dwBMPByteSize = ((sBitmapInfo.bmWidth * nBitsPerPixel + 31) / 32) * 4 * sBitmapInfo.bmHeight;

		// 以下为sBitmapHeader结构变量的全部成员赋值
		sBitmapHeader.biSize = sizeof(BITMAPINFOHEADER); // 指定这个结构的长度
		sBitmapHeader.biWidth = sBitmapInfo.bmWidth;     // 指定图象的宽度，单位是象素
		sBitmapHeader.biHeight = sBitmapInfo.bmHeight;   // 指定图象的高度，单位是象素
		sBitmapHeader.biPlanes = 1;                      // 必须是1，不用考虑
		sBitmapHeader.biBitCount = nBitsPerPixel;        // 指定表示颜色时要用到的位数，常用的值为1(黑白二色图), 4(16色图), 8(256色), 24(真彩色图)
		sBitmapHeader.biCompression = BI_RGB;            // 指定位图是否压缩，BI_RGB为不压缩
		sBitmapHeader.biSizeImage = 0;                   // 指定实际的位图数据占用的字节数(针对压缩方式而言)，如果biCompression为BI_RGB，则该项可以为零
		sBitmapHeader.biXPelsPerMeter = 0;               // 指定目标设备的水平分辨率，单位是每米的象素个数
		sBitmapHeader.biYPelsPerMeter = 0;               // 指定目标设备的垂直分辨率，单位同上
		sBitmapHeader.biClrUsed = 0;                     // 调色板中实际使用的颜色数
		sBitmapHeader.biClrImportant = 0;                // 指定本图象中重要的颜色数，如果该值为零，则认为所有的颜色都是重要的

		// 以下为sBMPFileHeader结构变量的全部成员赋值
		sBMPFileHeader.bfType = 0x4D42;                              // "BM"，固定值
		sBMPFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBMPByteSize; 
		sBMPFileHeader.bfReserved1 = 0;
		sBMPFileHeader.bfReserved2 = 0;
		sBMPFileHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		// 为位图数据区分配堆内存
		lpBMPFileBuffer = new BYTE[dwBMPByteSize];
		if (NULL == lpBMPFileBuffer)
		{
			iResult = -8;
			break;
		}

		// 获取该设备场景的全部像素值失败
		// GetDIBits( 设备DC句柄, 图像句柄, 第一个扫描线, 扫描线数, 缓冲区指针, BITMAPINFO结构的地址, 颜色表);
		if (0 == ::GetDIBits(hCompatibleDC, hCompatibleBitmap, 0, (UINT)sBitmapInfo.bmHeight, 
			(void*)lpBMPFileBuffer, (BITMAPINFO*)&sBitmapHeader, DIB_RGB_COLORS))
		{
			iResult = -9;
			break;
		}
#if 1

		// 创建位图文件  
		hBmpFile = ::CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE == hBmpFile)  
		{
			iResult = -10;
			break;
		}

		// 写入位图文件头 
		::WriteFile(hBmpFile, (void*)&sBMPFileHeader, sizeof(sBMPFileHeader), &dwWrittenSize, NULL);
		if (sizeof(sBMPFileHeader) != dwWrittenSize)
		{
			iResult = -11;
			break;
		}

		// 写入位图信息头 
		::WriteFile(hBmpFile, (void*)&sBitmapHeader, sizeof(sBitmapHeader), &dwWrittenSize, NULL);         
		if (sizeof(sBitmapHeader) != dwWrittenSize)
		{
			iResult = -12;
			break;
		}

		// 写入位图数据 
		::WriteFile(hBmpFile, (void*)lpBMPFileBuffer, dwBMPByteSize, &dwWrittenSize, NULL);
		if (dwBMPByteSize != dwWrittenSize)
		{
			iResult = -13;
			break;
		}
#endif

	} while (0);

	// 如果位图数据缓冲区指针不为空的话，则
	if (NULL != lpBMPFileBuffer)
	{
		delete[] lpBMPFileBuffer;
	}

	// 如果位图文件句柄是有效的话，则
	if (INVALID_HANDLE_VALUE != hBmpFile)
	{
		::CloseHandle(hBmpFile);
	}

	// 恢复兼容设备场景原有的位图并取得置出的兼容位图
	if (NULL != hCompatibleDC)
	{		
		(HBITMAP)::SelectObject(hCompatibleDC, hOldBitmap);
		::DeleteDC(hCompatibleDC);
	}

	if (NULL != hCompatibleBitmap)
	{
		::DeleteObject(hCompatibleBitmap);
	}

	return iResult;
}

BOOL D3D9RenderHK::CaptureEx( LPCSTR lpPath, DWORD dwWidth, DWORD dwHeight )
{
	BOOL bResult = FALSE;
	HRESULT hr = S_OK;
	dwWidth = (dwWidth == 0) ? m_videoWidth : dwWidth;
	dwHeight = (dwHeight == 0) ? m_videoHeight : dwHeight;

	//CComPtr<IDirect3DSurface9>      pCaptureSurface; //抓图表面
	IDirect3DSurface9*  pCaptureSurface;

	HDC hdc = NULL;

	RECT dstRect;
	dstRect.left     = 0;
	dstRect.right    = dwWidth;
	dstRect.top      = 0;
	dstRect.bottom   = dwHeight;

	do
	{
		if( m_pDevice == NULL || m_pOffsceenSurface == NULL )
		{
			break;
		}

		hr = m_pDevice->CreateRenderTarget( dwWidth, dwHeight, m_displayMode.Format,  D3DMULTISAMPLE_NONE, 0, TRUE, &pCaptureSurface, NULL );
		if( pCaptureSurface == NULL )
		{
			break;
		}

		hr = m_pDevice->StretchRect(m_pOffsceenSurface, NULL, pCaptureSurface, NULL, D3DTEXF_LINEAR);
		if( hr != S_OK )
		{
			break;
		}

		pCaptureSurface->GetDC( &hdc );
		if( hdc == NULL )
		{
			break;
		}
		
		//绘制当前状态
		DrawStatusFlag( hdc, &dstRect );

		//绘制叠加的字符
		DrawTitleForCapture( hdc, &dstRect );

		if( m_bShowIAInfo && m_pIAFrame != NULL )
		{
			CDC cdc;
			if( cdc.Attach( hdc ) )
			{
				DrawIAInfo( &cdc, m_pIAFrame, &m_rcDes );
				cdc.Detach();
			}
		}
		
#if 1
		if( 0 != SaveDC2BmpFile( hdc, dwWidth, dwHeight, 
						 dwWidth, dwHeight, lpPath ) )
		{

			break;
		}
#endif
		
		
		bResult = TRUE;

	}while( false );

	if( hdc != NULL )
	{
		pCaptureSurface->ReleaseDC( hdc );
	}

	//SafeRelease( pCaptureSurface );
	SafeReleaseEx(pCaptureSurface)

	return bResult;
}

DWORD D3D9RenderHK::D3DColorToRGB( DWORD color )
{
	DWORD temp = 0;
	DWORD val  = 0;
	
	temp = color &0xff;

	val = temp << 16;
	temp = ( color >> 8 ) & 0xff;

	val |= (temp << 8);

	temp = ( color >> 16 ) & 0xff;

	val |= temp;

	return val;
	
}

void D3D9RenderHK::ShowIAInfo( BOOL bShow )
{
	m_bShowIAInfo = bShow;
	m_pIAFrame    = NULL;
}


void
D3D9RenderHK::DrawIAInfo(CDC* pDC, IAFrame* pFrame, RECT *pRect )
{
	if ( pFrame == NULL || pRect == NULL )
	{
		return;
	}
	if (pFrame->count > 20)
	{
		CString err;
		err.Format("IA error!! pFrame->count=%d(>20)!", pFrame->count);
		//OutputDebugString(err);
		return;
	}

	CPen *pOld = NULL;

	if (m_pPenGreen == NULL)
	{
		m_pPenGreen = new CPen;
		m_pPenGreen->CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	}

	if (m_pPenRed == NULL)
	{
		m_pPenRed = new CPen;
		m_pPenRed->CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	}

	pDC->SetBkMode(TRANSPARENT);

	// [绘制坐标系缩放]
	//ZoomDC(pDC,(float)m_rcDes.right/(float)CIFF_XDIM,(float)m_rcDes.bottom/(float)CIFF_YDIM);

	m_xscale = (float)pRect->right/(float)CIFF_XDIM;
	m_yscale = (float)pRect->bottom/(float)CIFF_YDIM;

	int pos = sizeof(IAFrame);
	for (int i = 0; i < pFrame->count; i++)
	{
		IAEvent* pEvent = (IAEvent*)((char *)pFrame + pos);
		if (pEvent->objCount > 20)
		{
			CString err;
			err.Format("IA error!! pEvent->objCount=%d(>20)!", pEvent->objCount);
			OutputDebugString(err);
			break;
		}
		if (pEvent->type >= 100)
		{
			CString err;
			err.Format("IA error!! pEvent->type=%d(>100)!", pEvent->type);
			OutputDebugString(err);
			break;
		}
		//     if (pEvent->spot.vertices <= 0 || pEvent->spot.vertices > 16)
		//     {
		//       CString err;
		//       err.Format("IA error!! pEvent->spot.vertices=%d(>16 || <=0)!", pEvent->spot.vertices);
		//       OutputDebugString(err);
		//       return;
		//     }

		if (IAIsAlarmEvent(pEvent, i))
		{
			if( pOld != NULL )
			{
				pDC->SelectObject( pOld );
			}

			pOld = pDC->SelectObject(m_pPenRed);
		}
		else
		{
			if( pOld != NULL )
			{
				pDC->SelectObject( pOld );
			}

			pOld = pDC->SelectObject(m_pPenGreen);
		}

		if (pEvent->spot.vertices > 0 && pEvent->spot.vertices <= 16)
		{
			DrawIARgn(pDC, pEvent->spot.vertices, pEvent->spot.points);
		}

		pos += sizeof(IAEvent);
		if (pEvent->objCount > 0)
		{
			if (pEvent->type != BSRALGEVENT_FACEDETECT)
			{
				if( pOld != NULL )
				{
					pDC->SelectObject( pOld );
				}

				pOld = pDC->SelectObject(m_pPenRed);
			}

			for (unsigned int j = 0; j < pEvent->objCount; j++)
			{
				IAObject* pIAObject = (IAObject*)((char *)pFrame + pos);
				DrawIARect(pDC, &pIAObject->figure);
				pos += sizeof(IAObject);
			}
		}

		if (pEvent->type == BSRALGEVENT_PEOPLECOUNT)
		{
			char szinfo[30];
			sprintf_s(szinfo, "人数统计%d\0", pEvent->objCount);
			pDC->TextOut(10, 268, szinfo);
		}

		if (pEvent->spot.vertices == 2 && pEvent->type == BSRALGEVENT_RESET
			|| (pEvent->type == BSRALGEVENT_TRAPWIRE))
		{
			POINT pt1, pt2, pt3, pt4;
			pt1.x = pEvent->spot.points[0].x*m_xscale;
			pt1.y = pEvent->spot.points[0].y*m_yscale;
			pt2.x = pEvent->spot.points[1].x*m_xscale;
			pt2.y = pEvent->spot.points[1].y*m_yscale;
			GetPerpendicularLine(pt1, pt2, pt3, pt4);
			pDC->MoveTo(pt3);
			pDC->LineTo(pt4);

			int direct = 0;
			if (pEvent->type == BSRALGEVENT_TRAPWIRE)
			{
				direct = pEvent->behavior;
				if( direct == 0 )
				{
					//为了兼容以前的协议
					direct = pEvent->concerns;
				}
			}
			else
			{
				direct = pEvent->concerns;
			}
			if (direct == ARROW_DIRECT_UP)
			{
				DrawArrow(pDC, pt3, pt4, PI/9, 20);
			}
			else if( direct == ARROW_DIRECT_DOWN )
			{
				DrawArrow(pDC, pt4, pt3, PI/9, 20);
			}else if( direct == ARROW_DIRECT_BOTH )
			{
				DrawArrow(pDC, pt3, pt4, PI/9, 20);
				DrawArrow(pDC, pt4, pt3, PI/9, 20);
			}
		}
	}

	if( pOld != NULL )
	{
		pDC->SelectObject( pOld );
	}
}


BOOL
D3D9RenderHK::IAIsAlarmEvent(IAEvent* pEvent, int nIndex)
{
	BOOL bRet = FALSE;
	if(pEvent->type == 0 || pEvent->behavior == 0)
		return bRet ;

	switch (pEvent->type)
	{
	case BSRALGEVENT_PEOPLECOUNT:
		if  (pEvent->objCount> 0 )
		{
			bRet = TRUE;
		}
		break;
	case BSRALGEVENT_SENTRYDETECT:
	case BSRALGEVENT_SENTRYGUARDER:
		//如果行为==重置 或者 在岗 则为非报警状态
		if (pEvent->behavior != SENTRY_OK)
		{
			bRet = TRUE;
		}
		break;
	default:
		bRet = TRUE;
		break;
	}

	return bRet;

#if 0
	BOOL bRet = FALSE;
	switch (pEvent->type)
	{
	case BSRALGEVENT_RESET:
		bRet = FALSE;
		break;
	case BSRALGEVENT_PERIMETER:
		bRet = TRUE;
		break;
	case BSRALGEVENT_ABANDONED:
	case BSRALGEVENT_STOLEN:
	case BSRALGEVENT_TRAPWIRE:
		bRet = TRUE;
		break;
	case BSRALGEVENT_FACEDETECT:
		break;
	case BSRALGEVENT_PEOPLECOUNT:
		if (2 == pEvent->behavior)
		{
			bRet = TRUE;
		}
		else if (0 == nIndex)
		{
			bRet = FALSE;
		}
		else
		{
			bRet = TRUE;
		}
		break;
	case BSRALGEVENT_SENTRYDETECT:
	case BSRALGEVENT_SENTRYGUARDER:
		//如果行为==重置 或者 在岗 则为非报警状态
		if (pEvent->behavior == SENTRY_OK || pEvent->behavior == SENTRY_RESET )
		{
			bRet = FALSE;
		}
		else
		{
			bRet = TRUE;
		}
		break;
	default:
		bRet = TRUE;
		break;
	}
	return bRet;
#endif
}

void
D3D9RenderHK::DrawIARgn(CDC* pDC, int count, point_t* pts)
{
	int x = 0;
	int y = 0;

	x = pts[0].x *m_xscale;
	y = pts[0].y *m_yscale;

	pDC->MoveTo(x, y);
	for (int i = 1; i < count; i++)
	{
		
		x = pts[i].x *m_xscale;
		y = pts[i].y *m_yscale;

		pDC->LineTo(x , y);
	}

	x = pts[0].x *m_xscale;
	y = pts[0].y *m_yscale;

	pDC->LineTo( x, y);
}

void
D3D9RenderHK::DrawIARect(CDC* pDC, quad_t* figure)
{
	pDC->MoveTo(figure->topleft.x*m_xscale,     figure->topleft.y*m_yscale);
	pDC->LineTo(figure->bottomright.x*m_xscale, figure->topleft.y*m_yscale);
	pDC->LineTo(figure->bottomright.x*m_xscale, figure->bottomright.y*m_yscale);
	pDC->LineTo(figure->topleft.x*m_xscale,     figure->bottomright.y*m_yscale);
	pDC->LineTo(figure->topleft.x*m_xscale,     figure->topleft.y*m_yscale);
}

BOOL
D3D9RenderHK::IAFrameIsValide(IAFrame* pFrame)
{
	if (pFrame == NULL
		|| pFrame->version != 1
		|| pFrame->profile != 1
		|| pFrame->count > 255 || pFrame->count < 1)
		return FALSE;

	return TRUE;
}


double
D3D9RenderHK::GetDistance(POINT pt1, POINT pt2)
{
	return sqrt(pow((double)(pt1.x - pt2.x), 2) + pow((double)(pt1.y - pt2.y), 2));
}

void
D3D9RenderHK::GetPerpendicularLine(POINT ptIn1, POINT ptIn2, POINT& ptOut1, POINT& ptOut2)
{
	if (ptIn1.x > ptIn2.x)
	{
		POINT ptTmp = ptIn1;
		ptIn1 = ptIn2;
		ptIn2 = ptTmp;
	}

	double dis = GetDistance(ptIn1, ptIn2);
	if (dis == 0)
	{
		return;
	}
	int len = dis / 6;
	double sin = (ptIn2.y - ptIn1.y) / dis;
	double cos = (ptIn2.x - ptIn1.x) / dis;
	POINT ptMid = {(ptIn1.x + ptIn2.x) / 2, (ptIn1.y + ptIn2.y) / 2};
	ptOut1.x = ptMid.x - len * sin;
	ptOut1.y = ptMid.y + len * cos;
	ptOut2.x = ptMid.x + len * sin;
	ptOut2.y = ptMid.y - len * cos;
}

void
D3D9RenderHK::DrawArrow(CDC* pDC, POINT ptFrom, POINT ptTo, double radian, double len)
{
	double dis = GetDistance(ptFrom, ptTo);
	if (dis == 0)
	{
		return;
	}

	POINT pts[3];
	pts[0] = ptTo;
	double tmpX = ptTo.x + (ptFrom.x - ptTo.x) * len / dis;   
	double tmpY = ptTo.y + (ptFrom.y - ptTo.y) * len / dis;   
	pts[1].x = (tmpX - ptTo.x) * cos(-radian/2) - (tmpY - ptTo.y) * sin(-radian/2) + ptTo.x;   
	pts[1].y = (tmpY - ptTo.y) * cos(-radian/2) + (tmpX - ptTo.x) * sin(-radian/2) + ptTo.y;   
	pts[2].x = (tmpX - ptTo.x) * cos( radian/2) - (tmpY - ptTo.y) * sin( radian/2) + ptTo.x;   
	pts[2].y = (tmpY - ptTo.y) * cos( radian/2) + (tmpX - ptTo.x) * sin( radian/2) + ptTo.y;   
	pDC->Polygon(pts, 3);
}

HRESULT 
D3D9RenderHK::FillData(BYTE* pY, BYTE* pV, BYTE* pU, int width, int height, int strideY, int strideU, int strideV )
{
	HRESULT hr = S_FALSE;
	D3DLOCKED_RECT d3drect;

	SelfLock();

	do
	{
		if( pY == NULL || pV == NULL || pU == NULL )
		{
			break;
		}

		if( m_videoWidth != width || m_videoHeight != height )
		{
			m_bVideoChanged = true;
			m_videoWidth = width;
			m_videoHeight = height;
			break;
		}

		if( m_pOffsceenSurface == NULL )
		{
			break;
		}

		D3DSURFACE_DESC desc;
		hr = m_pOffsceenSurface->GetDesc( &desc );
		if( hr != S_OK )
		{
			break;
		}

		if( desc.Width < width || desc.Height < height )
		{
			break;
		}

		hr = m_pOffsceenSurface->LockRect(&d3drect, NULL, D3DLOCK_NOSYSLOCK);
		if( hr != S_OK )
		{
			hr = m_pOffsceenSurface->UnlockRect();
			break;
		}

		int newHeight  = m_videoHeight;
		int newWidth  = m_videoWidth;

		BYTE* pict = (BYTE*)d3drect.pBits;
		BYTE* Y = pY;
		BYTE* V = pV;
		BYTE* U = pU;

		switch(m_format)
		{
			case D3DFMT_YV12:

				for (int y = 0 ; y < newHeight ; y++)
				{
					memcpy(pict, Y, newWidth);
					pict += d3drect.Pitch;
					Y += strideY;
				}

				for (int y = 0 ; y < newHeight / 2 ; y++)
				{
					memcpy(pict, V, newWidth / 2);
					pict += d3drect.Pitch / 2;
					V += strideV;
				}

				for (int y = 0 ; y < newHeight / 2; y++)
				{
					memcpy(pict, U, newWidth / 2);
					pict += d3drect.Pitch / 2;
					U += strideU;
				}	

				break;

			case D3DFMT_NV12:
	            
				for (int y = 0 ; y < newHeight ; y++)
				{
					memcpy(pict, Y, newWidth);
					pict += d3drect.Pitch;
					Y += strideY;
				}
				for (int y = 0 ; y < newHeight / 2 ; y++)
				{
					memcpy(pict, V, newWidth);
					pict += d3drect.Pitch;
					V += strideV;
				}
				break;

			case D3DFMT_YUY2:
			case D3DFMT_UYVY:
			case D3DFMT_R5G6B5:
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:

				memcpy(pict, Y, d3drect.Pitch * newHeight);

				break;
		}
	    
		// Add by lwl 20161102
		if (NULL != m_pOffsceenSurface)
		{
		  m_pOffsceenSurface->UnlockRect();
		}

	}while( false );

	SelfUnLock();

	SetEvent( m_hEvent );

	return hr;
}


void 
D3D9RenderHK::Render()
{
	HRESULT hr = S_OK;

	do
	{	
		 SelfLock();
		do
		{
     	if( S_OK != CheckDevice( ) )
			{
				if( m_videoWidth == 0 || m_videoHeight == 0 )
				{
					break;
				}

				if( S_OK != Reset( ) )
				{
					break;
				}

				hr = CreateVideoSurface( m_videoWidth,  m_videoHeight, D3DFMT_YV12 );
				if( hr != S_OK )
				{
					m_videoWidth = 0;
					m_videoHeight = 0;
					break;
				}

				break; //没有数据，不需要显示
			}

			//判断窗口有无变化，若变化则重新初始化
			{
				RECT rc = {0, };

				::GetClientRect( m_hDisplayWindow, &rc );

				int wndWidth = rc.right - rc.left;
				int wndHeight = rc.bottom - rc.top;

				m_rcDes = rc;
				
				if( m_wndWidth != wndWidth || m_wndHeight != wndHeight )
				{
					if( S_OK != Reset( ) )
					{
						break;
					}

					m_wndWidth  = wndWidth;
					m_wndHeight = wndHeight;

					hr = CreateVideoSurface( m_videoWidth,  m_videoHeight, D3DFMT_YV12 );
					if( hr != S_OK )
					{
						m_videoWidth  = 0;
						m_videoHeight = 0;
						break;						
					}

					break; //没有数据，不需要显示
				}

			}

			//判断视频宽高是否一致
			{
				if( m_bVideoChanged )
				{
					hr = CreateVideoSurface( m_videoWidth,  m_videoHeight, D3DFMT_YV12 );
					if( hr != S_OK )
					{
						break;	
					}

					m_bVideoChanged = false;
					break; //没有数据，不需要显示
				}
			}

			if( CheckDevice() != S_OK )
			{
				break;
			}

#ifndef USE_SURFACE
			hr = CreateScene();
#endif
			hr = Present();


		}while( false );

		SelfUnLock();

		//若没有数据则500ms刷新一次
		WaitForSingleObject( m_hEvent, 500 );

	}while( !m_bExit );
}

void D3D9RenderHK::SetIAFrame( IAFrame *pFrame, int size ) 
{
	if( pFrame == NULL || size < 1 )
	{
		return;
	}

	SelfLock();
	//清除已有信息
	memset( m_pIABuf, 0, MAX_IABUF );
	//m_pIAFrame = pFrame;

	memcpy( m_pIABuf, pFrame, size );

	SelfUnLock();
}

void D3D9RenderHK::ResetIAInfo() 
{
	SelfLock();
	memset( m_pIABuf, 0, MAX_IABUF );
	SelfUnLock();
}


bool D3D9RenderHK::SelfLock( )
{
  /*
  while( m_bBusy )
  {
    Sleep( 0 );
  }

  m_bBusy = true;
  */
  EnterCriticalSection( &m_section );

  return true;
}

void D3D9RenderHK::SelfUnLock( )
{
  //m_bBusy = false;
	LeaveCriticalSection( &m_section );
}

int D3D9RenderHK::GetCurMonitor( HWND hWnd )
{
	UINT iCount = 0;
	int iMon = 0;

	MONITORINFOEX *pInfo = new MONITORINFOEX[16];
	RECT rc;
    POINT pt;

	char name[64] = {0, };

	do
	{
		if( m_pD3D9 == NULL || pInfo == NULL || hWnd == NULL )
		{
			break;
		}

		memset( pInfo, 0, sizeof(MONITORINFOEX)*10 );

		iCount = m_pD3D9->GetAdapterCount();

		if( iCount > 16 )
		{
			iCount = 16;
		}

		for( unsigned int i = 0; i < iCount; i++ )
		{
			HMONITOR hMonitor= m_pD3D9->GetAdapterMonitor( i );
			if( hMonitor == NULL )
			{
				break;
			}
			
			pInfo[i].cbSize = sizeof(MONITORINFOEX);
			GetMonitorInfo( hMonitor, &pInfo[i] );
		}

		GetWindowRect(hWnd, &rc);
		pt.x = (rc.left + rc.right) / 2;
		pt.y = (rc.top + rc.bottom) / 2;

		for( unsigned int i = 0; i < iCount; i++ )
		{
			if( PtInRect( &pInfo[i].rcMonitor, pt ) )
			{
				memcpy( name, pInfo[i].szDevice, sizeof(pInfo[i].szDevice));
				break;
			}
		}

		if( strlen( name ) != 0 )
		{
			D3DADAPTER_IDENTIFIER9 id = {0, };

			for( unsigned int i = 0; i < iCount; i++ )
			{
				m_pD3D9->GetAdapterIdentifier( i, 0, &id );
				if( strcmp( id.DeviceName, name ) == 0 )
				{
					iMon = i;
					break;
				}
			}
		}

	}while( false );

	if( pInfo != NULL )
	{
		delete []pInfo;
	}

	return iMon;
}