#pragma once
extern "C"
{
#include "libavcodec/avcodec.h"
}
struct DecodeInfo
{
	unsigned char*  pY;                   //Y plane base address of the picture
	unsigned char*  pU;                   //U plane base address of the picture
	unsigned char*  pV;                   //V plane base address of the picture
	unsigned long  uWidth;               //The width of output picture in pixel
	unsigned long  uHeight;              //The height of output picture in pixel
	unsigned long  uYStride;             //Luma plane stride in pixel
	unsigned long  uUVStride;            //Chroma plane stride in pixel
};

class CDecoderffmpeg
{
public:
	CDecoderffmpeg();
	~CDecoderffmpeg();
	BOOL CreateDecoder();
	void DestroyDecoder();
	int  DecoderFrame(char * pData, int nlen, DecodeInfo& decoderinfo);

private:
	static bool bInitffmepg;
	AVCodec *pCodec          ;
	AVCodecContext *pContext ;
	AVFrame *pFrame          ;
	AVPacket avpkt ;
	unsigned char *pYuv ;
	unsigned long pYuvSize;
	int picReady ;
	int totalFrame ;
	//AVINFO   avInfo = {0, };
	//unsigned char *pData = (unsigned char *)malloc( 1024*1024 );

};