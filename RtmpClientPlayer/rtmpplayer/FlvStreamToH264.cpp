#include "FlvStreamToH264.h"


//----------------
static int csH264StartCode = 0x01000000;//H264内容间隔标识00000001

#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|x&0xff00)
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
	(x << 8 & 0xff0000) | (x << 24 & 0xff000000))


bool Init();
void Clear();

bool parse8(int &i8, unsigned char* p);
bool parse16(int &i16, unsigned char* p);
bool parse24(int &i24, unsigned char* p);
bool parse32(int &i32, unsigned char* p);
bool parseTime(int &itime, unsigned char* p);



bool parse8(int &i8, unsigned char* p)
{
	memcpy(&i8, p, 1);
	return true;
}
bool parse16(int &i16, unsigned char* p)
{
	memcpy(&i16, p, 2);
	i16 = HTON16(i16);
	return true;
}
bool parse24(int &i24, unsigned char* p)
{
	memcpy(&i24, p, 3);
	i24 = HTON24(i24);
	return true;
}
bool parse32(int &i32, unsigned char* p)
{
	memcpy(&i32, p, 4);
	i32 = HTON32(i32);
	return true;
}


bool parseTime(int &itime, unsigned char* p)
{
	int temp = 0;
	memcpy(&temp, p, 4);
	itime = HTON24(temp);
	itime |= (temp & 0xff000000);
	return true;
}



FlvStreamToH264::FlvStreamToH264()
{
	frameCallback_ = NULL;
	bOneFrame_ = true;

	frameCache_ = NULL;
	leftBuff_ = new unsigned char[1024 * 1024];
	memset(leftBuff_, 0, 1024 * 1024);
	leftLen_ = 0;
}


FlvStreamToH264::~FlvStreamToH264()
{
	if (frameCache_ != NULL)
	{
		delete []frameCache_;
		frameCache_ = NULL;
	}

	if (leftBuff_ != NULL)
	{
		delete []leftBuff_;
		leftBuff_ = NULL;
	}

}


int FlvStreamToH264::pushPacket(unsigned char* pData, int len, bool bFirst)
{
	int ret = 0;
	do
	{
		unsigned char* tempPtr = pData;
		int tempLen = len;

		//flv流header
		if (bFirst)
		{
			//flv header
			ret = parseFlvHeader(&tempPtr, &tempLen);
			if (ret < 0)
			{
				break;
			}

			//body
			if (tempLen <= 0)
			{
				break;
			}

			ret = parseBody(tempPtr, &tempLen);
			if (ret < 0)
			{
				break;
			}
		}
		else
		{
			ret = parseBody(tempPtr, &tempLen);
			if (ret < 0)
			{
				break;
			}
		}

	} while (0);

	return ret;
}


int FlvStreamToH264::getPacket(BSTPHeader* frame)
{
	return 0;
}



int FlvStreamToH264::parseFlvHeader(unsigned char** pData, int* len)
{
	int ret = 0; 
	do
	{
		if (pData == NULL || len == NULL || *len <= 0)
		{
			ret = -1;
			break;
		}
		unsigned char* tempPtr = *pData;

		int headlength = 0;
		int filetype = 0;
		parse24(filetype, tempPtr);
		tempPtr = tempPtr + 3;
		int typel = 'flv';
		int typeh = 'FLV';
		if (filetype != typeh && filetype != typel)
		{
			ret = -1;
			break;
		}

		tempPtr = tempPtr + 2;

		parse32(headlength, tempPtr);
		/////////跳过头部长度/////
		*pData = *pData + headlength;
		*len -= headlength;

	} while (0);

	return ret;
}


int FlvStreamToH264::parseBody(unsigned char* pData, int* len)
{
	int ret = 0;
	do
	{
		if (pData == NULL || len == NULL || *len <= 0)
		{
			ret = -1;
			break;
		}

		//parse TagHeader:length=11
		int type = 0;
		int time = 0;
		int htime = 0;
		int datalength = 0;
		int info = 0;

		bool has4len = false;
		if (!bOneFrame_)
		{
			has4len = true;

			memcpy(leftBuff_ + leftLen_, pData, *len);
			pData = leftBuff_;
			leftLen_ += *len;
		}

		unsigned char* buf = NULL;
		parse8(type, pData);
		if (type == 9)
		{
			buf = pData;
		}
		else
		{
			//ignor 4 bytes of pretag length
			buf = pData + 4;
		}

		parse8(type, buf);
		if (type != 9)
		{
			if (bOneFrame_)
			{
				memcpy(leftBuff_ + leftLen_, pData, *len);
				leftLen_ += *len;
				bOneFrame_ = false;
				if (*len != 4)
				{
					printf("not a header");
					if (0)
					{
						//保存要发送的数据
						static FILE *saveFile = NULL;
						static int saveCount = 0;
						if (saveFile == NULL)
						{
							saveFile = fopen("e:/rtmp2222.h264", "wb");
						}

						if (saveFile != NULL)
						{
							fwrite(pData, 1, *len, saveFile);
							fflush(saveFile);
						}
					}
				}
				break;
			}
		}

		buf = buf + 1;
		parse24(datalength, buf);
		if (datalength <= 0)
		{
			break;
		}

		buf = buf + 3;
		parseTime(time, buf);
		buf = buf + 4;

		////////跳过StreamID/////
		buf = buf + 3;
		//
		parse8(info, buf);

		if (*len - 15 < datalength)
		{
			//uncomplete packet
			//bOneFrame_ = false;
		}

		//parse TagData
		if (type == 9)
		{
			parseTagData(buf, &datalength);
			leftLen_ = 0;
		}
		else
		{
			//audio, script frame
		}

		buf = buf + datalength;
		pData = buf;

		if (has4len)
		{
			*len = *len - 11 - datalength;
		}
		else
		{
			*len = *len - 15 - datalength;
		}

		if (bOneFrame_)
		{
			BSTPHeader* pheader = (BSTPHeader*)frameCache_;
			frameCallback_(pheader);
		}

	} while (*len > 0);

	return ret;
}


int FlvStreamToH264::parseTagData(unsigned char* pData, int* len)
{
	int ret = 0;
	do
	{
		if (pData == NULL || len == NULL || *len <= 0)
		{
			ret = -1;
			break;
		}

		if (frameCache_ != NULL)
		{
			delete []frameCache_;
			frameCache_ = NULL;
		}

		int frameLen = *len + 4 * 2 + sizeof(BSTPHeader);
		frameCache_ = new (std::nothrow) unsigned char[frameLen];
		if (frameCache_ == NULL)
		{
			ret = -2;
			break;
		}

		BSTPHeader* pheader = (BSTPHeader*)frameCache_;
		memset(pheader, 0, sizeof(BSTPHeader));

		unsigned char* buf = pData;
		int datalen = *len;

		int info = 0;
		parse8(info, buf); //17,27
		buf = buf + 1;
		if (info == 17)
		{
			//I frame and avc
			pheader->format[3] = 1;
		}
		else if (info == 27)
		{
			//inner frame(P,B frame)
			pheader->format[3] = 3;
		}
		else
		{
			//sps pps
		}

		//这里只考虑视频
		pheader->type = 2;

		int avctype = 0;
		parse8(avctype, buf);
		buf = buf + 1;

		buf = buf + 3;

		int templength = 0;
		unsigned char* tempbuff = frameCache_ + sizeof(BSTPHeader);
		//sps pps part
		if (avctype == 0)
		{
			buf = buf + 6;

			parse16(templength, buf);
			buf = buf + 2;
			//printf("sssize:%d\n", templength);

			memcpy(tempbuff, (void*)&csH264StartCode, 4);
			memcpy(tempbuff + 4, buf, templength);
			buf = buf + templength;
			tempbuff = tempbuff + 4 + templength;

			pheader->length = 4 + templength;

			parse8(templength, buf);//ppsnum
			buf = buf + 1;

			parse16(templength, buf);//ppssize
			//printf("ppsize:%d\n", templength);
			buf = buf + 2;

			memcpy(tempbuff, &csH264StartCode, 4);
			memcpy(tempbuff + 4, buf, templength);

			pheader->length += 4 + templength;

			bOneFrame_ = true;
		}
		else if (avctype == 1)
		{
			//可能存在多个nal，需全部读取
			//17 + 1 + 00 00 00
			int countsize = 1 + 1 + 3;
			while (countsize < datalen)
			{
				//datalen
				parse32(templength, buf);
				buf = buf + 4;

				memcpy(tempbuff, &csH264StartCode, 4);
				memcpy(tempbuff + 4, buf, templength);

				pheader->length += 4 + templength;

				countsize += (templength + 4);
			}

			bOneFrame_ = true;
		}
		else
		{
			//
		}
	} while (0);

	return ret;
}