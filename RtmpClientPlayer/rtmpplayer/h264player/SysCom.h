#ifndef _SYS_COM_H_
#define _SYS_COM_H_
#pragma once

#define SAFE_DELETE(p)          { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)    { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)         { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_CLOSE_HANDLE(h)    { if ((h) != INVALID_HANDLE_VALUE) {CloseHandle((h)); (h) = INVALID_HANDLE_VALUE;}}
#define SAFE_DEL_HANDLE(h)      { if (h != INVALID_HANDLE_VALUE) {CloseHandle(h); h = INVALID_HANDLE_VALUE;}}
#define SAFE_DEL_SOCKET(s)      { if (s != INVALID_SOCKET) { closesocket(s); s = INVALID_SOCKET;}}
#define RPC_ARRAY_SIZE(x)       ( (x).valid() ? (int)(x).size() : 0 )
#define SAFE_DEL_DLL(p)         { if (p) { FreeLibrary(p); p = NULL;}}
#define RETURN_FALSE_IF_NULL(p) { if (p == NULL) return FALSE;}
#define RETURN_IF_NULL(p)       { if (p == NULL) return;}
#define SAFE_DEL_DLG(p)         { if (p) { if (p->GetSafeHwnd()) {p->DestroyWindow();} delete p; p = NULL;}}

void     SetLogLevel(int iLevel);
void     Log(int iLevel, LPCSTR lpFormat, ...);

BOOL     CheckFileDir(CString sDir);
BOOL     CreateAllDirectories(CString strDir);

BOOL     WaitThreadExit(HANDLE hThread, DWORD dwTimeout);

void     RealtimeDispatchMessage( int loop = 100 );

void     SetComboSel(CComboBox &combo, int iData);
void     SetComboSel(CComboBox &combo, CString &sText);

int TextOutEx(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString
              , COLORREF crFone = RGB(255, 255, 255), COLORREF crBorder = RGB(0, 0, 0), int iBorder = 1);
int DrawTextEx(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, unsigned int uFormat
               , COLORREF crFone = RGB(255, 255, 255), COLORREF crBorder = RGB(0, 0, 0), int iBorder = 1);

CString  GetFolderPath(HWND hWnd, CString szTitle);
time_t   GetDateTime(COleDateTime date, COleDateTime time);
CString  GetSysErrorMsg();

time_t StringToDatetime(char *strTm);

#endif
