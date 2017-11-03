#pragma once

enum  PixelFormat
{
	YV12 = 0,
	NV12,
	YUY2,
	UYVY,

	RGB15,
	RGB16,
	RGB32,
	ARGB32
};


enum VideoDisplayMode
{
	KeepAspectRatioEx = 0,
    FillEx = 1
};

enum FillMode
{
	KeepAspectRatio = 0,
	Fill = 1
};
