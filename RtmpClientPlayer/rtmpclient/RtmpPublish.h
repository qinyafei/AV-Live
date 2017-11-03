#pragma once

#include <string>

class RtmpPublish
{
public:
	RtmpPublish(void);
	~RtmpPublish(void);

public:
	int publishH264(const char* rtmpUrl, const char* filePath);
	static unsigned int __stdcall rtmpPublishProc(void* context);
	static int readCallback(unsigned char *buf, int buf_size );

private:
	std::string rtmpUrl_;
	std::string filePath_;
	static FILE* file_;
};
