#include "StdAfx.h"
#include "SysCom.h"

int LogLevel = 1;
void SetLogLevel(int iLevel)
{
  LogLevel = iLevel;
}

void Log(int iLevel, LPCSTR lpFormat, ...)
{
  if (iLevel <= LogLevel)
  {
    CString sLog;
    va_list args;
    va_start(args, lpFormat);
    sLog.FormatV(lpFormat, args);
    va_end(args);
#ifdef _DEBUG
    //TRACE(sLog);
		OutputDebugString(sLog);
#else
    OutputDebugString(sLog);
#endif
  }
}

BOOL CheckFileDir(CString sDir)
{
  int nFound = sDir.ReverseFind('\\');
  if (nFound > 0)
  {
    return CreateAllDirectories(sDir.Left(nFound));
  }

  return TRUE;
}

BOOL CreateAllDirectories(CString strDir)
{
  if(strDir.Right(1)=="\\")
    strDir=strDir.Left(strDir.GetLength()-1);

  if(GetFileAttributes(strDir)!=-1)
    return TRUE;

  int nFound = strDir.ReverseFind('\\');
  BOOL bFind = FALSE;
  if (nFound > 0)
    bFind = CreateAllDirectories(strDir.Left(nFound));

  if (bFind)
  {
    CreateDirectory(strDir,NULL);
    return TRUE;
  }
  else
    return FALSE;		
}

BOOL WaitThreadExit(HANDLE hThread, DWORD dwTimeout)
{
  BOOL bResult = FALSE;
  DWORD dwStatus;
  int iCount = dwTimeout / 10;
  for (int i = 0; i < iCount; i++)
  {
    ::GetExitCodeThread(hThread, &dwStatus);
    if (dwStatus != STILL_ACTIVE)
    {
      bResult = TRUE;
      break;
    }
    Sleep(10);
  }

  if (!bResult)
  {
    TerminateThread(hThread, 0xff);
    TRACE("Terminate thread(%x)\n", hThread);
  }
  //  ::CloseHandle(hThread);

  return bResult;
}

void RealtimeDispatchMessage( int loop )
{
  int i  = 0;
  while (i < loop)
  {
    MSG msg;
    if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
    {
      GetMessage(&msg, NULL, NULL, NULL);
			if( msg.message == 2195 )
			{
				continue;
			}
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    i ++;
  }
}

void SetComboSel(CComboBox &combo, int iData)
{
  int iCount = combo.GetCount();
  for (int i = 0; i < iCount; i++)
  {
    if ((int)combo.GetItemData(i) == iData)
    {
      combo.SetCurSel(i);
      break;
    }
  }
}

void SetComboSel(CComboBox &combo, CString &sText)
{
  CString str;
  int iLen;
  for (int i = 0; i < combo.GetCount(); i++)
  {
    iLen = combo.GetLBTextLen(i);
    combo.GetLBText(i, str.GetBuffer(iLen));
    if (sText == str)
    {
      combo.SetCurSel(i);
      break;
    }
    str.ReleaseBuffer();
  }
}

int TextOutEx(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString
              , COLORREF crFone/* = RGB(255, 255, 255)*/, COLORREF crBorder/* = RGB(0, 0, 0)*/, int iBorder/* = 1*/)
{
  SetTextColor(hdc, crBorder);
  for (int i = 1; i <= iBorder; i++)
  {
    TextOut(hdc, nXStart - i, nYStart, lpString, cbString);
    TextOut(hdc, nXStart + i, nYStart, lpString, cbString);
    TextOut(hdc, nXStart, nYStart + i, lpString, cbString);
    TextOut(hdc, nXStart, nYStart - i, lpString, cbString);
  }

  SetTextColor(hdc, crFone);
  return TextOut(hdc, nXStart, nYStart, lpString, cbString);
}

int DrawTextEx(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, unsigned int uFormat
               , COLORREF crFone/* = RGB(255, 255, 255)*/, COLORREF crBorder/* = RGB(0, 0, 0)*/, int iBorder/* = 1*/)
{
  CRect rcDes;
  SetTextColor(hDC, crBorder);
  for (int i = 1; i <= iBorder; i++)
  {
    rcDes = lpRect;
    rcDes.left  -= i;
    rcDes.right -= i;
    DrawText(hDC, lpString, nCount, rcDes, uFormat);

    rcDes = lpRect;
    rcDes.left  += i;
    rcDes.right += i;
    DrawText(hDC, lpString, nCount, rcDes, uFormat);

    rcDes = lpRect;
    rcDes.top    += i;
    rcDes.bottom += i;
    DrawText(hDC, lpString, nCount, rcDes, uFormat);

    rcDes = lpRect;
    rcDes.top    -= i;
    rcDes.bottom -= i;
    DrawText(hDC, lpString, nCount, rcDes, uFormat);
  }

  SetTextColor(hDC, crFone);
  return DrawText(hDC, lpString, nCount, lpRect, uFormat);
}

CString GetFolderPath(HWND hWnd, CString szTitle)
{
  CString strPath;

  char szDir[MAX_PATH];
  BROWSEINFO bi;
  bi.hwndOwner = hWnd;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = szDir;
  bi.lpszTitle = szTitle;
  bi.ulFlags = BIF_STATUSTEXT | BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
  bi.lpfn = NULL;
  bi.lParam = 0;
  bi.iImage = 0;
  ITEMIDLIST *pidl = SHBrowseForFolder(&bi);
  if (pidl != NULL)
  {
    if (SHGetPathFromIDList(pidl, szDir))
    {
      strPath = szDir;
    }
  }
  return strPath;
}

time_t GetDateTime(COleDateTime date, COleDateTime time)
{
  CTime ct = CTime(date.GetYear(), date.GetMonth(), date.GetDay()
    , time.GetHour(), time.GetMinute(), time.GetSecond());
  return (time_t)ct.GetTime();
}

//获取GetLastError错误信息
CString GetSysErrorMsg()
{
	LPVOID lpMsgBuf;
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		::GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
		);
	return (LPCTSTR)lpMsgBuf;
}


time_t StringToDatetime(char *strTm)  
{  
    tm tm_;  
    int year, month, day, hour, minute,second;  
    sscanf(strTm,"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);  
    tm_.tm_year  = year-1900;  
    tm_.tm_mon   = month-1;  
    tm_.tm_mday  = day;  
    tm_.tm_hour  = hour;  
    tm_.tm_min   = minute;  
    tm_.tm_sec   = second;  
    tm_.tm_isdst = 0;  
  
    time_t t_ = mktime(&tm_); //已经减了8个时区  
    return t_; //秒时间  
} 