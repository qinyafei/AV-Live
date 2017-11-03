/** $Id: utypes.h 269 2009-06-09 03:24:48Z tiger $ $Date$
 *  @file utypes.h
 *  @brief short names of unsigned integer types 
 *  @version 0.0.1
 *  @since 0.0.1
 *  @author Tan Feng <tanf@bstar.com.cn>
 *  @date 2009-05-20    Created it
 */
/******************************************************************************
*@note
    Copyright 2009, BeiJing Bluestar Corporation, Limited
                 ALL RIGHTS RESERVED
Permission is hereby granted to licensees of BeiJing Bluestar, Inc. products
to use or abstract this computer program for the sole purpose of implementing
a product based on BeiJing Bluestar, Inc. products. No other rights to
reproduce, use, or disseminate this computer program,whether in part or in
whole, are granted. BeiJing Bluestar, Inc. makes no representation or
warranties with respect to the performance of this computer program, and
specifically disclaims any responsibility for any damages, special or
consequential, connected with the use of this program.
For details, see http://www.bstar.com.cn/
******************************************************************************/
#ifndef _UTYPES_H
#define _UTYPES_H

#pragma once

typedef unsigned char    uchar;
typedef unsigned int     uint;
typedef unsigned short   ushort;
typedef unsigned long    ulong;
typedef unsigned __int64 ullong;
typedef __int64 llong;



#define MAX_FRAMESIZE 1024*512


#define MAX_READUNIT 512


enum SOURCE_TYPE
{
	MEDIA_UNKNOW = 0,
	MEDIA_TCP,
	MEDIA_RTP,
};

struct SdkBsfpHead
{
	unsigned int mark;			/**< 帧头的前导标识，用于帧头定位，固定值为0xBF9D1FDB */
	unsigned char type;			/**< 帧类型标识符 1：音频 2：视频 3：语音 4: 音视频智能分析信息 9：消息通告 10: 命令请求与应答*/
	char channel;						/**< 通道号  取值范围（-127~128）*/
	unsigned short device;	/**< 设备出厂序列号 */
	unsigned int length;		/**< 本帧数据(data)总长度，不包括28 个字节的帧头 */
	unsigned int sequence;	/**< 该类型帧在该通道(channel)的序号，各类型独立计数，从系统启动后开始计数 */
	unsigned int timestamp;	/**< 该帧捕获的系统时间，为time_t 标准时间 */
	unsigned int tick;			/**< 该帧捕获的系统参考时钟计数(单位为微秒) SongZhanjun注：此项不可信 */
	
#if 1
	unsigned char codec;		
	unsigned char resolution;		/**< 分辨率 */
	unsigned char frameRate;
	unsigned char frameType;   // 1 = iframe, 3 = p frame , 2 = b frame
#else
	unsigned char format[4];
#endif

};

#ifndef USE_HKINFO
#define USE_HKINFO
#endif

//#undef USE_HKINFO

#endif
