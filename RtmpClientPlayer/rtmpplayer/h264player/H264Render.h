#pragma once

#include "VideoBS.h"
#include <string>

class H264Render
{
public:
	H264Render(void);
	~H264Render(void);

public:
	void init(HWND hWnd);
	void renderFile(const char* fileName);
	void display(char* frame, int len);

	static unsigned int  __stdcall decodeProc(void* context);

private:
	CVideoBS* video_;
	std::string fileName_;
};
