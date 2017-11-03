#pragma once

#include "FlvStreamToH264.h"
#include "H264Render.h"
#include "BSFPDefined.hpp"
#include <string>

class RtmpPlayer
{
public:
	RtmpPlayer(void);
	~RtmpPlayer(void);
	
public:
	void init(HWND hWnd);
	void setupRtmp(const char* rtmpUrl);
	static unsigned int __stdcall rtmpReceiveProc(void* context);
	static void h264FrameCallback(BSTPHeader* frame);

	static int sockInit();
	static void sockCleanup();

private:
	FlvStreamToH264* flvTrans_;
	///rtmp://192.168.8.240/myapp/livestream
	std::string rtmpUrl_;
	static H264Render* render_;

};
