// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 BSDECODER_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// BSDECODER_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef BSDECODER_EXPORTS
#define BSDECODER_API __declspec(dllexport)
#else
#define BSDECODER_API __declspec(dllimport)
#endif


struct bsDecFrameInfo
{
	unsigned char* pY;                 //Y plane base address of the picture
	unsigned char* pU;                 //U plane base address of the picture
	unsigned char* pV;                 //V plane base address of the picture
	unsigned int   uWidth;               //The width of output picture in pixel
	unsigned int   uHeight;              //The height of output picture in pixel
	unsigned int   uYStride;             //Luma plane stride in pixel
	unsigned int   uUVStride;            //Chroma plane stride in pixel
	unsigned int   uCroppingLeftOffset;  //Crop information in pixel
	unsigned int   uCroppingRightOffset; //
	unsigned int   uCroppingTopOffset;   //
	unsigned int   uCroppingBottomOffset;//
	unsigned int   uPicFlag;             //0: Frame; 1: Top filed; 2: Bottom field  
	unsigned int   bError;               //0: picture is correct; 1: picture is corrupted
	unsigned int   bIntra;               //0: intra picture; 1:inter picture
	unsigned int   uPictureID;           //The sequence ID of this output picture decoded
	unsigned int   uReserved;            //Reserved for future
	void           *pUserData;           //Pointer to the first userdata
};

//创建解码器

BSDECODER_API HANDLE bsDecCreate( );

/*
	return value

	-1 = 位初始化
	0  = OK
    1  = 需要更多数据
*/
//解码
BSDECODER_API int    bsDecFrame( HANDLE hDec, char *pFrame, int len, unsigned char encType, bsDecFrameInfo *pOutInfo );

//销毁解码器
BSDECODER_API void   bsDecDestroy( HANDLE hDec );