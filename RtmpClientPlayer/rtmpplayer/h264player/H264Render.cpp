#include "StdAfx.h"
#include "H264Render.h"

#include "utypes.h"
#include "SysDef.h"
#include "MediaDefine.h"

#include "H264Render.h"

H264Render::H264Render(void)
:video_(NULL)
{
}

H264Render::~H264Render(void)
{
}


void H264Render::init(HWND hWnd)
{
	if (video_ == NULL)
	{
		video_ = new CVideoBS(1);
		BOOL bret = video_->Init(hWnd, MDECODE_BSX);
		if (!bret)
		{
			printf("video init failed!");
		}
	}
}


void H264Render::display(char* frame, int len)
{
	BOOL bResult = FALSE;
	SdkBsfpHead* header = (SdkBsfpHead*)frame;
	if (frame == NULL || len <= 0)
	{
		return ;
	}

	FrameStruct tagCurPlayFrame;
	memset(&tagCurPlayFrame, 0, sizeof(FrameStruct));
	tagCurPlayFrame.iLen = header->length;
	tagCurPlayFrame.pBuf = (BYTE*)(frame + sizeof(SdkBsfpHead));
	tagCurPlayFrame.encType = 4;
	if (video_ != NULL)
	{
		bResult = video_->DisPlay(&tagCurPlayFrame, false );
		if (!bResult)
		{
			printf("video display failed");
			return ;
		}
	}
}


unsigned int H264Render::decodeProc(void* context)
{

  H264Render* src = (H264Render*)context;
	if (context == NULL)
	{
		return -1;
	}

	//
	int fileHeaderLen = 256;
	FILE* file = NULL;
	file = fopen(src->fileName_.c_str(), "rb");
	if (file == NULL)
	{
		return -2;
	}

	char header[257] = { 0 };
	int hlen = fread(header, 1, 256, file);
	if (hlen != 256)
	{
		return -3;
	}
	
	char* frameBuf = new char[1024 * 512];
	while (!feof(file))
	{
		memset(frameBuf, 0, 1024 * 512);

		int actualLen = 0;
		int bstpheaderLen = sizeof(SdkBsfpHead);
		SdkBsfpHead* bstp;
		int rlen = fread(frameBuf, 1, bstpheaderLen, file);
		if (rlen != bstpheaderLen)
		{
			break;
		}

		bstp = (SdkBsfpHead*)frameBuf;
		if (bstp->mark != 0xBF9D1FDB)
		{
			break;
		}

		int dataLen = bstp->length;
		rlen = fread(frameBuf + sizeof(SdkBsfpHead), 1, dataLen, file);
		if (rlen != dataLen)
		{
			int err = GetLastError();
			break;
		}

		if (bstp->type != 2)
		{
			continue;
		}

		//int wsize = m_h264TransRtp.bstp_TransportData(buf, rlen, &bstp);
		//int wsize = send(sock, frameBuf, rlen + sizeof(SdkBsfpHead), 0);
		src->display(frameBuf, rlen + sizeof(SdkBsfpHead));
		Sleep(40);
	}
	

	delete[]frameBuf;
	frameBuf = NULL;

	return 0;
}



void H264Render::renderFile(const char* fileName)
{
	fileName_ = fileName;

	unsigned int threadId = 0;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &H264Render::decodeProc, this, 0, &threadId);
}