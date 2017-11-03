#include "stdafx.h"
#include "bsrffmpeg.h"
#include "CDecoderffmpeg.h"



// 创建解码器
/*********************************************************************
* Function Name  : ff_264DecCreate
* Description    : Create and initialize H.264 decoder handle
* Parameters     : pDecAttr:   a pointer referring to H264_DEC_ATTR_S 
*                              which contain the needed  attributes to 
*                              initialize the decoder
* Return Type    : if success, return a decoder handle; 
*                  otherwise,  return NULL. 
* Last Modified  : 
*********************************************************************/
HI_HDL  HI_DLLEXPORT ff_Hi264DecCreate( H264_DEC_ATTR_S *pDecAttr )
{
	CDecoderffmpeg * pDecoder = new CDecoderffmpeg();
	if (NULL != pDecoder && pDecoder->CreateDecoder())
	{
		return (HANDLE)pDecoder;
	}
	if (NULL != pDecoder)
	{
		delete pDecoder;
		pDecoder = NULL;
	}
	return NULL;
}
// 解码
/*********************************************************************************
* Function Name  : ff_264DecAU
* Description    : input an au-stream and decode one au (single-thread version)
* Parameters     : hDec       : decoder handle created by HI_H264DEC_Create
*                : pStream    : stream buffer
*                : iStreamLen : stream length in byte
*                : ullPTS     : time stamp
*                : pDecFrame  : denoting whether there is a picture frame to display,
*                               and the decoded Parameters of the picture frame
*                               (referring to H264_DEC_FRAME_S ).
*                : uFlags     : working mode, invalid 
* Return Type    : if success : return HI_H264DEC_OK;
*                  otherwise  : return the corresponding error code.
* Last Modified  : 
**********************************************************************************/
HI_S32 HI_DLLEXPORT ff_Hi264DecAU(
									 HI_HDL hDec,
									 HI_U8 *pStream,
									 HI_U32 iStreamLen,
									 HI_U64 ullPTS,
									 H264_DEC_FRAME_S *pDecFrame,
									 HI_U32 uFlags )
{

	CDecoderffmpeg *pDecoder = (CDecoderffmpeg*)hDec;
	if(NULL != pDecoder)
	{
		DecodeInfo decoderInfo;
		if (0 == pDecoder->DecoderFrame((char*)pStream, iStreamLen, decoderInfo))
		{
			if (NULL != pDecFrame)
			{
				pDecFrame->pY = decoderInfo.pY;
				pDecFrame->pU = decoderInfo.pU;
				pDecFrame->pV = decoderInfo.pV;
				pDecFrame->uWidth = decoderInfo.uWidth;
				pDecFrame->uHeight = decoderInfo.uHeight;
				pDecFrame->uYStride = decoderInfo.uYStride;
				pDecFrame->uUVStride = decoderInfo.uUVStride;
				return 0L;
			}
		}
	}
	;
	return -1L;
}
// 销毁解码器
/***************************************************************** 
* Function Name  : ff_264DecDestroy
* Description    : destroy H.264 decoder handle
* Parameters     : hDec:  decoder handle created by ff_264DecCreate
* Return Type    : none
* Last Modified  : 
*****************************************************************/
void  HI_DLLEXPORT ff_Hi264DecDestroy( HI_HDL hDec )
{
	CDecoderffmpeg * pDecoder = (CDecoderffmpeg*)hDec;
	if (NULL != pDecoder)
	{
		pDecoder->DestroyDecoder();
		delete pDecoder;
		pDecoder = NULL;
	}
}