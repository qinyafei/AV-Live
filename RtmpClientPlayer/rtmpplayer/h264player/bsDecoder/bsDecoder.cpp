// bsDecoder.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "bsDecoder.h"


#include "IHW265Dec_Api.h"
#include "bsrffmpeg.h"

#include "h265parser/H265Parser.h"

#include "malloc.h"

#pragma comment( lib, "HW_H265dec_Win32D.lib" )
#pragma comment( lib, "h265parser/h265_Parser.lib" )

#pragma comment( lib, "ffmpegDecoder.lib")

#define STREAM_FLAGSIZE 4

#define NAL_SEI		6	// SEI
#define NAL_SPS		7	// SPS帧
#define NAL_PPS		8	// PPS帧


bool isSpsOrPps( unsigned char* pFrame, int len )
{
    if( len < STREAM_FLAGSIZE )
    {
        return false;
    }

    for( int i = 0; i < len - STREAM_FLAGSIZE; i++ )
    {
        if( pFrame[i] == 0x00 && pFrame[i+1] == 0x00 && pFrame[i+2] == 0x01 )
        {
			int nTemp = pFrame[i+3] & 0x1F;
            if( nTemp == NAL_SEI 
				|| nTemp == NAL_SPS 
				|| nTemp == NAL_PPS )
            {
                return true;
            }
        }
    }

    return false;
}

//枚举
enum VideoEncode
{
	ENCODE_UNKNOWN,   //未知类型
	ENCODE_H264 = 4,  //h264
	ENCODE_H265,      //H265
};

union OutInfo
{
	H264_DEC_FRAME_S h264;
	IH265DEC_OUTARGS h265;
};

struct DecoderInfo
{
	unsigned char encType; //编码类型  0 = 未知, 4 = h264, 5 = h265

	unsigned char res[3];

	HANDLE hDec;  //解码器句柄

	GstH265Parser *parser;

	//以下信息针对H265
	int maxWidth;  //最大宽度
	int maxHeight; //最大高度
	int refNum;    //参考帧数

	
	OutInfo outInfo;

	DecoderInfo( )
	{
		encType   = 0;
		hDec      = NULL;

		maxWidth  = 0;
		maxHeight = 0;
		refNum    = 0;
		parser    = NULL;

		memset( &outInfo, 0, sizeof(outInfo) );
	}
};


void *HW265D_Malloc(UINT32 channel_id, UINT32 size) 
{
  return (void *)malloc(size);
}


void HW265D_Free(UINT32 channel_id, void * ptr) 
{
  free(ptr);
}

void HW265D_Log( UINT32 channel_id, IHWVIDEO_ALG_LOG_LEVEL eLevel, INT8 *p_msg, ...)
{

}

//创建解码器

BSDECODER_API HANDLE bsDecCreate( )
{
	DecoderInfo *pInfo = new DecoderInfo();

	return (HANDLE)pInfo;
}

//解码
BSDECODER_API int    bsDecFrame( HANDLE hDec, char *pFrame, int len, unsigned char encType, bsDecFrameInfo *pOutInfo )
{

	int ret = -1;

	DecoderInfo *pDec = (DecoderInfo*)hDec;

	do
	{
		if( pDec == NULL )
		{
			break;
		}

		if( pOutInfo != NULL )
		{
			memset( pOutInfo, 0, sizeof(bsDecFrameInfo) );
		}

		if( pDec->encType != encType )
		{
			//
			if( pDec->hDec != NULL )
			{
				if( pDec->encType == ENCODE_H264 )
				{
					ff_Hi264DecDestroy( pDec->hDec );
					
				}else if( pDec->encType == ENCODE_H265 )
				{
					IHW265D_Delete( pDec->hDec );

					if( pDec->parser != NULL )
					{
						gst_h265_parser_free( pDec->parser );
						pDec->parser = NULL;
					}

				}

				pDec->hDec = NULL;
				pDec->encType = 0;
				pDec->maxHeight = 0;
				pDec->maxWidth  = 0;
				pDec->refNum    = 0;
			}

			if( encType == ENCODE_H264 )
			{
				H264_DEC_ATTR_S decAttr = {0, };
				pDec->hDec    = ff_Hi264DecCreate( &decAttr );
				if( pDec->hDec != NULL )
				{
					pDec->encType = encType;
				}else
				{
					pDec->encType = -1;
				}

			}else if( encType == ENCODE_H265 )
			{
				//
				if( pDec->parser == NULL )
				{
					pDec->parser = gst_h265_parser_new( );

				}else
				{
					break;
				}

				pDec->encType = encType;
			}
		}

		//初始化解码器
		if( encType == ENCODE_H264 )
		{
			if( pDec->hDec != NULL && pDec->encType == encType )
			{
				memset( &pDec->outInfo, 0, sizeof(pDec->outInfo) );
				ret =  ff_Hi264DecAU( pDec->hDec, (HI_U8 *)pFrame, (HI_U32) len,  0, &pDec->outInfo.h264, 0 );
				if( ret == 0 )
				{
					pOutInfo->pY        = pDec->outInfo.h264.pY;
					pOutInfo->pU        = pDec->outInfo.h264.pU;
					pOutInfo->pV        = pDec->outInfo.h264.pV;
					pOutInfo->uWidth    = pDec->outInfo.h264.uWidth;
					pOutInfo->uHeight   = pDec->outInfo.h264.uHeight;
					pOutInfo->uYStride  = pDec->outInfo.h264.uYStride;
					pOutInfo->uUVStride = pDec->outInfo.h264.uUVStride;

					pOutInfo->uCroppingLeftOffset   = pDec->outInfo.h264.uCroppingLeftOffset;
					pOutInfo->uCroppingRightOffset  = pDec->outInfo.h264.uCroppingRightOffset;
					pOutInfo->uCroppingTopOffset    = pDec->outInfo.h264.uCroppingTopOffset;
					pOutInfo->uCroppingBottomOffset = pDec->outInfo.h264.uCroppingBottomOffset;

					pOutInfo->uPicFlag   = pDec->outInfo.h264.uPicFlag;
					pOutInfo->bError     = pDec->outInfo.h264.bError;
					pOutInfo->bIntra     = pDec->outInfo.h264.bIntra;
					pOutInfo->uPictureID = pDec->outInfo.h264.uPictureID;
					pOutInfo->pUserData  = pDec->outInfo.h264.pUserData;
				}else
				{
                    if( !isSpsOrPps( (unsigned char*)pFrame, len ) )
                    {
                        ff_Hi264DecDestroy( pDec->hDec );
					    pDec->hDec = NULL;
					    pDec->encType = 0;
                    }
					break;

				}
			}else
			{
				break;
			}	
		}else if( encType == ENCODE_H265 )
		{
			GstH265SPS sps = {0, };

			if( pDec->parser == NULL )
			{
				pDec->parser = gst_h265_parser_new( );
			}

			if( pDec->parser == NULL )
			{
				break;
			}
			
			if( gst_h265_parser_sps_info( pDec->parser, &sps, (const guint8 *)pFrame, len) )
			{
				if( pDec->hDec == NULL || pDec->maxHeight != sps.pic_height_in_luma_samples || 
					pDec->maxWidth != sps.pic_width_in_luma_samples || pDec->refNum != sps.long_term_ref_pics_present_flag )
				{
					if( pDec->hDec != NULL )
					{
						IHW265D_Delete( pDec->hDec ); 
						pDec->hDec = NULL;
					}


					pDec->maxWidth  = sps.pic_width_in_luma_samples;
					pDec->maxHeight = sps.pic_height_in_luma_samples;
					pDec->refNum    = sps.long_term_ref_pics_present_flag;

					IHW265D_INIT_PARAM stInitParam = {0, };

					stInitParam.uiChannelID = 0;
					if ( pDec->maxWidth < 8 )
					{
						pDec->maxWidth=8;
					}

					if (pDec->maxWidth>4096)
					{
						pDec->maxWidth=4096;
					}

					if (pDec->maxHeight<8)  
					{
						pDec->maxHeight=8;
					}

					if ( pDec->maxHeight > 2160 )
					{
						pDec->maxHeight=2160;
					}

					stInitParam.iMaxWidth   = pDec->maxWidth;
					stInitParam.iMaxHeight  = pDec->maxHeight;
					stInitParam.iMaxRefNum  = pDec->refNum;
					

					INT32 MultiThreadEnable = 1;	// default is single thread mode
					stInitParam.eThreadType = MultiThreadEnable? IH265D_MULTI_THREAD: IH265D_SINGLE_THREAD;
					INT32 DispOutput = 0; // default is decode order
					stInitParam.eOutputOrder= DispOutput? IH265D_DISPLAY_ORDER:IH265D_DECODE_ORDER;

					stInitParam.MallocFxn  = HW265D_Malloc;
					stInitParam.FreeFxn    = HW265D_Free;
					stInitParam.LogFxn     = HW265D_Log;

					int iRet = IHW265D_Create( &pDec->hDec, &stInitParam);
					if( IHW265D_OK != iRet )
					{
						pDec->maxWidth  = 0;
						pDec->maxHeight = 0;
						pDec->refNum    = 0;

						break;
					}

					pDec->encType = encType;
				}
			}

			if( pDec->hDec != NULL )
			{
				IH265DEC_INARGS stInArgs={0,};

				stInArgs.eDecodeMode =  IH265D_DECODE;
				stInArgs.pStream =(UINT8*)pFrame;
				stInArgs.uiStreamLen = len;
				//出参
				
				memset(&pDec->outInfo.h265, 0, sizeof(pDec->outInfo.h265));

				pDec->outInfo.h265.eDecodeStatus = (HW265D_DECODESTATUS)-1;
				pDec->outInfo.h265.uiBytsConsumed = 0;
				
				ret = IHW265D_DecodeFrame( pDec->hDec, &stInArgs, &pDec->outInfo.h265 );
				if( 0 == ret )
				{
					pOutInfo->pY        = pDec->outInfo.h265.pucOutYUV[0];
					pOutInfo->pU        = pDec->outInfo.h265.pucOutYUV[1];
					pOutInfo->pV        = pDec->outInfo.h265.pucOutYUV[2];
					pOutInfo->uWidth    = pDec->outInfo.h265.uiDecWidth;
					pOutInfo->uHeight   = pDec->outInfo.h265.uiDecHeight;
					pOutInfo->uYStride  = pDec->outInfo.h265.uiYStride;
					pOutInfo->uUVStride = pDec->outInfo.h265.uiUVStride;

					pOutInfo->uCroppingLeftOffset   = 0;
					pOutInfo->uCroppingRightOffset  = 0;
					pOutInfo->uCroppingTopOffset    = 0;
					pOutInfo->uCroppingBottomOffset = 0;

					pOutInfo->uPicFlag   = 0;
					pOutInfo->bError     = pDec->outInfo.h265.bIsError;
					pOutInfo->bIntra     = 0;
					pOutInfo->uPictureID = pDec->outInfo.h265.uiChannelID;
					pOutInfo->pUserData  = NULL;
				}

			}
		}
			
	}while( false );

	return ret;
}

//销毁解码器
BSDECODER_API void   bsDecDestroy( HANDLE hDec )
{
	DecoderInfo *pDec = (DecoderInfo*)hDec;

	do
	{
		if( pDec == NULL )
		{
			break;
		}

		if( pDec->hDec != NULL )
		{
			if( pDec->encType == ENCODE_H264 )
			{	
				ff_Hi264DecDestroy( pDec->hDec );
				pDec->hDec = NULL;
			}else 
			{
				IHW265D_Delete( pDec->hDec );
			}
		}

		pDec->hDec = NULL;

		if( pDec->parser != NULL )
		{
			gst_h265_parser_free( pDec->parser );
			pDec->parser = NULL;
		}

		delete pDec;
		pDec = NULL;

	}while( false );

}