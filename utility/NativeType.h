/********************************************************************
	Filename: 	NativeType.h
	Description:
	Version:  1.0
	Created:  25:3:2016   13:40

	Compiler: gcc vc
	Author:   wufan, love19862003@163.com
	Organization: lezhuogame
*********************************************************************/
#ifndef __NativeType_H__
#define __NativeType_H__
#include <stdint.h>
typedef unsigned int uint;
#ifdef _MSC_VER
typedef __int8  int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
#else
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
#endif
#endif // __NativeType_H__
