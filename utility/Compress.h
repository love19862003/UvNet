/********************************************************************
	Filename: 	Compress.h
	Description:
	Version:  1.0
	Created:  28:3:2016   10:32
	
	Compiler: gcc vc
	Author:   wufan, love19862003@163.com
	Organization: lezhuogame
*********************************************************************/
#ifndef __Compress_H__
#define __Compress_H__
#include <string>
namespace ShareSpace {
  namespace Utility {
    bool compressBuf(const void* in, size_t il, std::string& out);
    bool unCompressBuf(const void* in, size_t il, std::string& out);
    unsigned int crc32Buf(const void*, size_t);
  }
}
#endif // __Compress_H__