#include "StdAfx.h"
#include "RtmpPlayer.h"

#include <stdio.h>
#include "librtmp/rtmp_sys.h"
#include "librtmp/log.h"

#include "librtmp\rtmp.h"   
#include "librtmp\rtmp_sys.h"


int RtmpPlayer::sockInit()
{
#ifdef WIN32     
	WORD version;    
	WSADATA wsaData;    
	version = MAKEWORD(1, 1);    
	return (WSAStartup(version, &wsaData) == 0);    
#else     
	return TRUE;    
#endif 
}


void RtmpPlayer::sockCleanup()
{
#ifdef WIN32     
	WSACleanup();    
#endif 
}

//
H264Render* RtmpPlayer::render_ = NULL;

RtmpPlayer::RtmpPlayer(void)
{
	flvTrans_ = NULL;
	render_ = NULL;
}

RtmpPlayer::~RtmpPlayer(void)
{
	if (flvTrans_ != NULL)
	{
		delete flvTrans_;
		flvTrans_ = NULL;
	}

	if (render_ != NULL)
	{
		delete render_;
		render_ = NULL;
	}

}


void RtmpPlayer::setupRtmp(const char* rtmpUrl)
{
	if (rtmpUrl == NULL || strlen(rtmpUrl) <= 0)
	{
		return ;
	}

	if (flvTrans_ == NULL)
	{
		flvTrans_ = new FlvStreamToH264();
		flvTrans_->setFrameCallback(h264FrameCallback);
	}

	rtmpUrl_ = rtmpUrl;

	unsigned int threadId = 0;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &RtmpPlayer::rtmpReceiveProc, this, 0, &threadId);
}


unsigned int __stdcall RtmpPlayer::rtmpReceiveProc(void* context)
{
	int ret = 0;
	const int bufsize=1024*1024*10;			
	char* buf= NULL;
	do
	{
		RtmpPlayer* player = (RtmpPlayer*)context;
		if (player == NULL)
		{
			break;
		}

		sockInit();

		buf = (char*)malloc(bufsize);
		if (buf == NULL)
		{
			break;
		}
		memset(buf,0,bufsize);

		//is live stream ?
		bool bLiveStream=true;	
		RTMP *rtmp=RTMP_Alloc();
		RTMP_Init(rtmp);

		//set connection timeout,default 30s
		rtmp->Link.timeout=10;	
		// HKS's live URL
		if (!RTMP_SetupURL(rtmp, (char*)(player->rtmpUrl_.c_str())))
		{
			RTMP_Log(RTMP_LOGERROR,"SetupURL Err\n");
			RTMP_Free(rtmp);
			ret = -1;
			break;
		}

		if (bLiveStream)
		{
			rtmp->Link.lFlags|=RTMP_LF_LIVE;
		}

		//1hour
		RTMP_SetBufferMS(rtmp, 3600*1000);		

		if(!RTMP_Connect(rtmp,NULL))
		{
			RTMP_Log(RTMP_LOGERROR,"Connect Err\n");
			RTMP_Free(rtmp);
			ret = -2;
			break;
		}

		if(!RTMP_ConnectStream(rtmp,0))
		{
			RTMP_Log(RTMP_LOGERROR,"ConnectStream Err\n");
			RTMP_Close(rtmp);
			RTMP_Free(rtmp);
			break;
		}

		int nRead = 0;
		while(nRead=RTMP_Read(rtmp,buf,bufsize) > 0)
		{
			static bool bfirst = true;
			if (bfirst)
			{
				player->flvTrans_->pushPacket((unsigned char*)buf, nRead, true);
				bfirst = false;
			}
			else
			{
				player->flvTrans_->pushPacket((unsigned char*)buf, nRead, false);
			}

			Sleep(40);
		}

		if(rtmp)
		{
			RTMP_Close(rtmp);
			RTMP_Free(rtmp);
			rtmp=NULL;
		}	

	}while(0);

	if (buf != NULL)
	{
		free(buf);
	}

	sockCleanup();

	return ret;
}



void RtmpPlayer::h264FrameCallback(BSTPHeader* frame)
{
	if (render_ == NULL || frame == NULL)
	{
		return ;
	}

	render_->display((char*)frame, frame->length + sizeof(BSTPHeader));
}



void RtmpPlayer::init(HWND hWnd)
{
	if (render_ == NULL)
	{
		render_ = new H264Render();
	  render_->init(hWnd);
	}

}