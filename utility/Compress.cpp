/********************************************************************
	Filename: 	Compress.cpp
	Description:
	Version:  1.0
	Created:  28:3:2016   10:34
	
	Compiler: gcc vc
	Author:   wufan, love19862003@163.com
	Organization: lezhuogame
*********************************************************************/
#include <string.h>
#include "utility/Compress.h"
#include "zlib.h"
#include "utility/ThreadStatic.h"
   
namespace ShareSpace{
  namespace Utility{
    struct ThreadByte{
      explicit ThreadByte(size_t len){
        m_buf = (Bytef*)malloc(len);
        memset(m_buf, 0, len);
      }
      ~ThreadByte(){
        free(m_buf);
        m_buf = nullptr;
      }

      Bytef* buffer() { return m_buf; }
    private:
      Bytef* m_buf;
    };
    Bytef* getByte(uLongf* len){
      const size_t s_len = 65535 * 2;
      if(*len > s_len) { return nullptr; }
      *len = s_len;
      return getThreadLocal<ThreadByte>(*len)->buffer();
    }

    bool compressBuf(const void* in, size_t il, std::string& out) {
      uLongf destLen = compressBound(il);
      Bytef* dest = getByte(&destLen);
      std::unique_ptr<Bytef> destptr(nullptr);
      if (nullptr == dest){
        dest = (Bytef*)malloc(destLen);
        destptr.reset(dest);
      }
      auto r = compress(dest, &destLen, static_cast<const Bytef*>(in), il);
      if(Z_OK == r) {
        out.clear();
        out.append((char*)(dest), destLen);
      }
      return Z_OK == r;
    }

    static int uncompress_real(const Bytef* in, uLongf il,  uLongf* ol, std::string& out ) {
      Bytef* dest = getByte(ol);
      std::unique_ptr<Bytef> destptr(nullptr);
      if(nullptr == dest) {
        dest = (Bytef*)malloc(*ol);
        destptr.reset(dest);
      }
      auto r = uncompress(dest, ol, in, il);
      if (r == Z_BUF_ERROR){
        *ol = *ol * 2;
        return uncompress_real(in, il, ol, out);
      }
      if (r == Z_OK){
        out.clear();
        out.append((char*)(dest), *ol);
      }
      return r;
    }
    bool unCompressBuf(const void* in, size_t il, std::string& out) {
      uLongf destLen = 3 * il;
      if(Z_OK == uncompress_real(static_cast<const Bytef*>(in), il, &destLen, out)) {
        return true;
      }
      return false;
    }

    unsigned int crc32Buf(const void* buf, size_t len) {
      return crc32(0L, static_cast<const Bytef*>(buf), len);
    }


  }
}