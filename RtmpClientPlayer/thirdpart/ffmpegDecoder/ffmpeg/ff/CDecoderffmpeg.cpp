#include "stdafx.h"
#include "CDecoderffmpeg.h"
#include "process.h"
//void* __imp_toupper = toupper;
bool CDecoderffmpeg::bInitffmepg = false;
void* _1 = toupper;
void* _2 = vsnprintf;
void* _3 = _beginthreadex;

#pragma comment( lib, "libmingwex.a")
#pragma comment( lib, "libgcc.a")
#pragma comment( lib, "zlib.lib")

#pragma comment( lib, "ffmpeg.lib")


using namespace std;
CDecoderffmpeg::CDecoderffmpeg()
{
	pCodec          = NULL;
	pContext = NULL;
	pFrame          = NULL;
	//avpkt = {0, };
	pYuv  = NULL;
	picReady = 0;
	totalFrame = 0;
	pYuvSize = 0;
	if (!bInitffmepg)
	{
		avcodec_register_all();
		bInitffmepg = true;
	}
}
CDecoderffmpeg::~CDecoderffmpeg()
{

}
BOOL 
CDecoderffmpeg::CreateDecoder()
{
	BOOL bRet = FALSE;
	do 
	{
		av_init_packet( &avpkt );
		pCodec = avcodec_find_decoder( CODEC_ID_H264 );
		if( pCodec == NULL )
		{
			break;
		}

		pContext = avcodec_alloc_context3( pCodec );
		if( pContext == NULL )
		{
			break;
		}

		if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
		{
			pContext->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
		}

		if (avcodec_open2( pContext, pCodec, NULL) < 0) 
		{
			OutputDebugString("Could not open codec\n");
			break;
		}

		pFrame = avcodec_alloc_frame();
		if( pFrame == NULL )
		{
			break;
		}
		bRet = TRUE;
	} while (FALSE);

	return bRet;
}
void 
CDecoderffmpeg::DestroyDecoder()
{
	if(NULL != pFrame)
	{
		avcodec_free_frame( &pFrame );
	}

	if(NULL != pContext)
	{
		avcodec_close( pContext );
		av_free( pContext );
	}

	if( NULL != pYuv)
	{
		free( pYuv );
	}
}
int  CDecoderffmpeg::DecoderFrame(char * pData, int nlen, DecodeInfo& decoderinfo)
{
	avpkt.size = nlen;
	avpkt.data = (unsigned char*)pData;

	picReady = 0;
	avcodec_decode_video2( pContext, pFrame, &picReady, &avpkt );
	if( picReady == 0 )
	{
		return -1;
	}

	totalFrame ++;


	decoderinfo.pY = pFrame->data[0];
	decoderinfo.pU = pFrame->data[1];
	decoderinfo.pV = pFrame->data[2];
	if(1 == pFrame->top_field_first)
		decoderinfo.uHeight = pFrame->height/2;
	else
		decoderinfo.uHeight = pFrame->height;
	decoderinfo.uWidth = pFrame->width;
	decoderinfo.uYStride = pFrame->linesize[0]; // y¿ç¶È
	decoderinfo.uUVStride = pFrame->linesize[1]; // uv ¿ç¶È
	return 0;
}
