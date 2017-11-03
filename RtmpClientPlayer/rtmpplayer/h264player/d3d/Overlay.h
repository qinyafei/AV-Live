#pragma once

#include "d3d9.h"
#include <windows.h>
#include <map>
#include "Macros.h"

using namespace std;

class Overlay
{
public:
	Overlay(IDirect3DDevice9* device, D3DCOLOR color, BYTE opacity)
	{
		m_device = device;
		m_opacity = opacity;
		m_color = (color & 0x00FFFFFF) | (opacity << 24);
	}

	virtual ~Overlay(void)
	{

	}

	virtual HRESULT Draw(void) = 0;

protected:
	IDirect3DDevice9* m_device;
	D3DCOLOR m_color;
	BYTE m_opacity;
};

class LineOverlay : public Overlay
{
public:
	LineOverlay(IDirect3DDevice9* device, POINT p1, POINT p2, INT width, D3DCOLOR color, BYTE opacity)
		: Overlay(device, color, opacity)
	{
		HRESULT hr = D3DXCreateLine(m_device, &m_line);
		m_vectors[0].x = p1.x;
		m_vectors[0].y = p1.y;
		m_vectors[1].x = p2.x;
		m_vectors[1].y = p2.y;
		m_line->SetWidth(width);
	}

	virtual ~LineOverlay()
	{
		SafeReleaseEx(m_line);
	}

	virtual HRESULT Draw(void)
	{
		HR(m_line->Begin());
		HR(m_line->Draw(m_vectors, 2, m_color));
		return m_line->End();
	}

private:
	ID3DXLine* m_line;
	D3DXVECTOR2 m_vectors[2];
};

class RectangleOverlay : public Overlay
{
public:
	RectangleOverlay(IDirect3DDevice9* device, RECT rectangle, INT width, D3DCOLOR color, BYTE opacity)
		: Overlay(device, color, opacity)
	{
		D3DXCreateLine(m_device, &m_line);
		m_line->SetWidth(width);

		m_vectors[0].x = rectangle.left;
		m_vectors[0].y = rectangle.top;
		m_vectors[1].x = rectangle.right;
		m_vectors[1].y = rectangle.top;
		m_vectors[2].x = rectangle.right;
		m_vectors[2].y = rectangle.bottom;
		m_vectors[3].x = rectangle.left;
		m_vectors[3].y = rectangle.bottom;
		m_vectors[4].x = rectangle.left;
		m_vectors[4].y = rectangle.top;
	}

	virtual ~RectangleOverlay()
	{
		SafeReleaseEx(m_line);
	}

	virtual HRESULT Draw(void)
	{
		HR(m_line->Begin());
		HR(m_line->Draw(m_vectors, 5, m_color));
		return m_line->End();
	}

private:
	ID3DXLine* m_line;
	D3DXVECTOR2 m_vectors[5];
};

class PolygonOverlay : public Overlay
{
public:
	PolygonOverlay(IDirect3DDevice9* device, POINT* points, INT pointsLen, INT width, D3DCOLOR color, BYTE opacity)
		: Overlay(device, color, opacity) 
	{
		HRESULT hr = D3DXCreateLine(m_device, &m_line);
		m_vectors = new D3DXVECTOR2[pointsLen + 1];
		for(int i = 0 ; i < pointsLen; i++)
		{
			m_vectors[i].x = points[i].x;
			m_vectors[i].y = points[i].y;
		}

		m_vectors[pointsLen].x = points[0].x;
		m_vectors[pointsLen].y = points[0].y;

		m_numOfVectors = pointsLen + 1;
		m_line->SetWidth(width);
	}

	virtual ~PolygonOverlay()
	{
		delete []m_vectors;
		SafeReleaseEx(m_line);
	}

	virtual HRESULT Draw(void)
	{
		HR(m_line->Begin());
		HR(m_line->Draw(m_vectors, m_numOfVectors, m_color));
		return m_line->End();
	}

private:
  ID3DXLine* m_line;
	D3DXVECTOR2* m_vectors;
	INT m_numOfVectors;
};

class TextOverlay : public Overlay
{
public:
	TextOverlay(IDirect3DDevice9* device, LPCSTR text, RECT pos, INT size, D3DCOLOR color, LPCSTR font, BYTE opacity)
		: Overlay(device, color, opacity)
	{
		
		m_pos = pos;
		m_text = text;
		HRESULT hr = D3DXCreateFont( m_device, size, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font, &m_font );
		
	}

	virtual HRESULT Draw(void)
	{
		return m_font->DrawText(NULL, m_text, -1, &m_pos, 0, m_color );
		//return S_OK;
	}

  virtual ~TextOverlay()
  {
    SafeReleaseEx(m_font);
  }

private:
	ID3DXFont* m_font;
	CString m_text;
	RECT m_pos;
};

class BitmapOverlay : public Overlay
{
public:
	BitmapOverlay(IDirect3DDevice9* device, POINT destPosition, INT destWidth, INT destHeight, POINT srcStartPos, POINT srcEndPos, BYTE* pPixelData, D3DCOLOR color, BYTE opacity)
		: Overlay(device, D3DCOLOR_ARGB(0xff, 0, 0, 0), opacity)
	{
		VERTEX vertexArray[] =
		{
			{ D3DXVECTOR3(destPosition.x-0.5f, destPosition.y-0.5f, 0),                  D3DCOLOR_ARGB(opacity, 255, 255, 255), D3DXVECTOR2(0, 1) },  // top left
			{ D3DXVECTOR3(destPosition.x + destWidth-0.5f, destPosition.y-0.5f, 0),          D3DCOLOR_ARGB(opacity, 255, 255, 255), D3DXVECTOR2(1, 1) },  // top right
			{ D3DXVECTOR3(destPosition.x + destWidth-0.5f, destPosition.y + destHeight-0.5f, 0), D3DCOLOR_ARGB(opacity, 255, 255, 255), D3DXVECTOR2(1, 0) },  // bottom right
			{ D3DXVECTOR3(destPosition.x-0.5f, destPosition.y + destHeight-0.5f, 0),         D3DCOLOR_ARGB(opacity, 255, 255, 255), D3DXVECTOR2(0, 0) },  // bottom left
		};

		HRESULT hr = m_device->CreateTexture(destWidth, destHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexture, NULL);
		


#if 1
		hr = m_device->CreateVertexBuffer(sizeof(VERTEX) * 4, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL);

		D3DLOCKED_RECT lock;
		hr = m_pTexture->LockRect(0, &lock, NULL,D3DLOCK_DISCARD);
		BYTE* target = (BYTE*)lock.pBits;
		BYTE red = GetRValue(color);
		BYTE green = GetGValue(color);
		BYTE blue = GetBValue(color);

		for (int i=0;i<destHeight;i++ )
		{
			for (int j =0;j<destWidth;j++)
			{

				int linebyte = (destWidth*24/8+3)/4*4;
				int index= i*linebyte+j*3;
				target[0]=pPixelData[index+0];//B
				target[1]=pPixelData[index+1];//G
				target[2]=pPixelData[index+2];//R
				if ((target[2] ==red &&target[1]==green && target[0] == blue)||(j<srcStartPos.x ||i<srcStartPos.y)||(j>=srcEndPos.x|| i>=srcEndPos.y))
				{
					target[3]=0;
				}
				else 
				{
					target[3]=255;
				}
				target += 4;
			}
			target += (lock.Pitch-4*destWidth);

		}

		hr = m_pTexture->UnlockRect(0);

		VERTEX *vertices;
		hr = m_pVertexBuffer->Lock(0, 0, (void**)&vertices, D3DLOCK_DISCARD);
		memcpy(vertices, vertexArray, sizeof(vertexArray));
		hr = m_pVertexBuffer->Unlock();	

#endif


#if 0
		IDirect3DSurface9 *pSurface = NULL;
		m_pTexture->GetSurfaceLevel( 0, &pSurface );
		if( pSurface != NULL )
		{
			HDC hdc = NULL;
			pSurface->GetDC( &hdc );
			if( hdc != NULL )
			{
				HPEN hPen = CreatePen( PS_SOLID, 5, RGB(255, 255, 255));
				HGDIOBJ  hOld = NULL;
				if( hPen != NULL )
				{
					hOld = ::SelectObject(hdc, hPen );
				}
				::SetBkColor( hdc, RGB(255, 255, 255 ));
				::TextOut(hdc, 0, 0, "Fuck", strlen("Fuck"));

				::MoveToEx( hdc, 10, 10, NULL );
				::LineTo( hdc, 20, 20 );

				if( hOld != NULL )
				{
					::SelectObject( hdc, hOld );
				}

				DeleteObject( hPen );
				
				pSurface->ReleaseDC( hdc );
			}

		}
#endif

	}

	virtual ~BitmapOverlay()
	{
		SafeReleaseEx(m_pVertexBuffer);
    SafeReleaseEx(m_pTexture);
	}

	virtual HRESULT Draw(void)
	{
		HRESULT hr = S_OK;

		hr = m_device->SetPixelShader(NULL);
		hr = m_device->SetVertexShader(NULL);
		hr = m_device->SetTexture(0, m_pTexture);
		hr = m_device->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(VERTEX));
		hr = m_device->SetFVF(D3DFVF_CUSTOMVERTEX);
		hr = m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

		return hr;
	}

private:
	IDirect3DTexture9 *m_pTexture;
	IDirect3DVertexBuffer9 *m_pVertexBuffer;
};
class ArrowOverlay: public Overlay
{
public:
	ArrowOverlay(IDirect3DDevice9* device, POINT ptFrom, POINT ptTo, double radian, double len,FLOAT width, D3DCOLOR color, BYTE opacity)
		: Overlay(device, color, opacity)

	{
		double dis = sqrt(pow((double)(ptFrom.x - ptTo.x), 2) + pow((double)(ptFrom.y - ptTo.y), 2));
		double tmpX = ptTo.x + (ptFrom.x - ptTo.x) * len / dis;   
		double tmpY = ptTo.y + (ptFrom.y - ptTo.y) * len / dis;   

		HRESULT hr = D3DXCreateLine(m_device, &m_line);
		m_vectors = new D3DXVECTOR2[4];

		m_vectors[0].x = ptTo.x;
		m_vectors[0].y = ptTo.y;
		m_vectors[1].x = (tmpX - ptTo.x) * cos(-radian/2) - (tmpY - ptTo.y) * sin(-radian/2) + ptTo.x;   
		m_vectors[1].y = (tmpY - ptTo.y) * cos(-radian/2) + (tmpX - ptTo.x) * sin(-radian/2) + ptTo.y;   
		m_vectors[2].x = (tmpX - ptTo.x) * cos( radian/2) - (tmpY - ptTo.y) * sin( radian/2) + ptTo.x;   
		m_vectors[2].y = (tmpY - ptTo.y) * cos( radian/2) + (tmpX - ptTo.x) * sin( radian/2) + ptTo.y;  

		m_vectors[3].x = ptTo.x;
		m_vectors[3].y = ptTo.y;

		m_numOfVectors = 4;
		m_line->SetWidth(width);
	}

	virtual ~ArrowOverlay()
	{
		delete []m_vectors;
		m_vectors = NULL;
		SafeReleaseEx(m_line);
	}

	virtual HRESULT Draw(void)
	{
		HR(m_line->Begin());
		HR(m_line->Draw(m_vectors, m_numOfVectors, m_color));
		return m_line->End();
	}

private:
	ID3DXLine* m_line;
	D3DXVECTOR2* m_vectors;
	INT m_numOfVectors;

};
class OverlayStore
{
	typedef map<SHORT, Overlay*> OverlayMap;

public:
	OverlayStore()
	{
		InitializeCriticalSection(&m_lock);
		m_overlayKey = 0;
	}

	virtual ~OverlayStore()
	{
		RemoveAll();
		DeleteCriticalSection(&m_lock);
	}

	void AddOverlay(Overlay* pOverlay, SHORT id)
	{

		EnterCriticalSection(&m_lock); 
		if(m_overlays[id] != NULL)
		{
			delete m_overlays[id];
			m_overlays[id] = NULL;

		}
		m_overlays[id] = pOverlay;
		LeaveCriticalSection(&m_lock);

	}

	void RemoveOverlay(SHORT id)
	{
		EnterCriticalSection(&m_lock); 
		Overlay* pOverlay = m_overlays[id];
		delete pOverlay;
		pOverlay = NULL;
		m_overlays.erase(id);
		LeaveCriticalSection(&m_lock);
	}

	void Draw()
	{

		if(IsEmpty())
		{
			return;
		}

		EnterCriticalSection(&m_lock); 
		for each(pair<SHORT, Overlay*> pair in m_overlays )
		{
			pair.second->Draw();
		}
		LeaveCriticalSection(&m_lock);
	}

	bool IsEmpty()
	{
		bool isEmpty = false;
		EnterCriticalSection(&m_lock); 
		isEmpty = m_overlays.empty();
		LeaveCriticalSection(&m_lock);

		return isEmpty;
	}

	void RemoveAll()
	{
		EnterCriticalSection(&m_lock);
		for each(pair<SHORT, Overlay*> pair in m_overlays )
		{

			Overlay* pOverlay = pair.second;
			delete pOverlay;
			pOverlay = NULL;			
		}
		m_overlays.clear();
		LeaveCriticalSection(&m_lock);
	}
	SHORT GetOverlayKey()
	{
		EnterCriticalSection(&m_lock);
		m_overlayKey = m_overlays.size();
		LeaveCriticalSection(&m_lock);
		return m_overlayKey;
	}

private:
	OverlayMap m_overlays;
	CRITICAL_SECTION m_lock;
	SHORT m_overlayKey;
};