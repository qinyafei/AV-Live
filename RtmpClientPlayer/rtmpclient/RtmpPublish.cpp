#include "StdAfx.h"
#include "RtmpPublish.h"
#include <process.h>
#include "librtmp_send264.h"

FILE* RtmpPublish::file_ = NULL;

RtmpPublish::RtmpPublish(void)
{
}

RtmpPublish::~RtmpPublish(void)
{
}


int RtmpPublish::publishH264(const char* rtmpUrl, const char* filePath)
{
	if (rtmpUrl == NULL || strlen(rtmpUrl) <= 0)
	{
		return -1;
	}

	rtmpUrl_ = rtmpUrl;
	filePath_ = filePath;

	unsigned int threadId = 0;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &RtmpPublish::rtmpPublishProc, this, 0, &threadId);

	return 0;
}


unsigned int __stdcall RtmpPublish::rtmpPublishProc(void* context)
{
	int ret = 0;
	do
	{
		RtmpPublish* publish = (RtmpPublish*)context;
		if (publish == NULL)
		{
			break;
		}

		if (RtmpPublish::file_ != NULL)
		{
			fclose(RtmpPublish::file_);
		}

		RtmpPublish::file_ = fopen(publish->filePath_.c_str(), "rb");
		if (RtmpPublish::file_ == NULL)
		{
			ret = -1;
			break;
		}

		//初始化并连接到服务器
		RTMP264_Connect(publish->rtmpUrl_.c_str());

		//发送
		RTMP264_Send(&RtmpPublish::readCallback);

		//断开连接并释放相关资源
		RTMP264_Close();


	}while(0);

	return ret;
}


int RtmpPublish::readCallback(unsigned char *buf, int buf_size )
{
	if(!feof(RtmpPublish::file_))
	{
		int true_size=fread(buf,1,buf_size,RtmpPublish::file_);
		return true_size;
	}
	else
	{
		return -1;
	}
}
