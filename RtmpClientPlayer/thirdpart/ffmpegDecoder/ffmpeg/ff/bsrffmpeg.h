#pragma  once

#define HI_DLLEXPORT __declspec( dllexport )

typedef void* HI_HDL;
typedef unsigned char   HI_U8;
typedef unsigned long   HI_U32;
typedef unsigned char   HI_UCHAR;
typedef unsigned __int64        HI_U64;
typedef signed long     HI_S32;

typedef struct hiH264_USERDATA_S
{
	HI_U32             uUserDataType;   //Type of userdata
	HI_U32             uUserDataSize;   //Length of userdata in byte
	HI_UCHAR*          pData;           //Buffer contains userdata stuff
	struct hiH264_USERDATA_S* pNext;    //Pointer to next userdata
} H264_USERDATA_S;
typedef struct hiH264_DEC_ATTR_S
{
	HI_U32  uPictureFormat;       //Decoded output picture format 0x00:YUV420 0x01:YUV422 0x02:YUV444
	HI_U32  uStreamInType;        //Input stream type
	HI_U32  uPicWidthInMB;        //The width of picture in MB
	HI_U32  uPicHeightInMB;       //The height of picture in MB
	HI_U32  uBufNum;              //Max reference frame num 
	HI_U32  uWorkMode;            //Decoder working mode 
	H264_USERDATA_S  *pUserData;  //Buffer contains userdata stuff
	HI_U32  uReserved;
} H264_DEC_ATTR_S;

typedef struct hiH264_DEC_FRAME_S
{
	HI_U8*  pY;                   //Y plane base address of the picture
	HI_U8*  pU;                   //U plane base address of the picture
	HI_U8*  pV;                   //V plane base address of the picture
	HI_U32  uWidth;               //The width of output picture in pixel
	HI_U32  uHeight;              //The height of output picture in pixel
	HI_U32  uYStride;             //Luma plane stride in pixel
	HI_U32  uUVStride;            //Chroma plane stride in pixel
	HI_U32  uCroppingLeftOffset;  //Crop information in pixel
	HI_U32  uCroppingRightOffset; //
	HI_U32  uCroppingTopOffset;   //
	HI_U32  uCroppingBottomOffset;//
	HI_U32  uDpbIdx;              //The index of dpb
	HI_U32  uPicFlag;             //0: Frame; 1: Top filed; 2: Bottom field  
	HI_U32  bError;               //0: picture is correct; 1: picture is corrupted
	HI_U32  bIntra;               //0: intra picture; 1:inter picture
	HI_U64  ullPTS;               //Time stamp
	HI_U32  uPictureID;           //The sequence ID of this output picture decoded
	HI_U32  uReserved;            //Reserved for future
	H264_USERDATA_S *pUserData;   //Pointer to the first userdata
} H264_DEC_FRAME_S;

// 创建解码器
HI_HDL  HI_DLLEXPORT ff_Hi264DecCreate( H264_DEC_ATTR_S *pDecAttr );
HI_S32 HI_DLLEXPORT ff_Hi264DecAU(
															 HI_HDL hDec,
															 HI_U8 *pStream,
															 HI_U32 iStreamLen,
															 HI_U64 ullPTS,
										 					 H264_DEC_FRAME_S *pDecFrame,
															 HI_U32 uFlags );
// 销毁解码器
void  HI_DLLEXPORT ff_Hi264DecDestroy( HI_HDL hDec );
