/********************************************************************

  Filename:   NetMessage

  Description:NetMessage

  Version:  1.0
  Created:  14:9:2015   10:22
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/NetMessage.h"
#include "log/MyLog.h"
#include "utility/StringUtility.h"
#include "utility/Compress.h"

namespace ShareSpace {
  namespace NetSpace {

    NetBuffer::NetBuffer(size_t len) {
      m_buffer = (char*)malloc(len);
      memset(m_buffer, 0, len);
      m_maxLen = len;
      m_writePos = 0;
      m_readPos = 0;
      m_lock = false;
    }
    NetBuffer::~NetBuffer() {
      free(m_buffer);
      m_buffer = nullptr;
      m_maxLen = m_writePos = m_readPos = 0;
      m_lock = false;
    }

    NetBuffer& NetBuffer::operator = (NetBuffer&& buffer) {
      free(m_buffer);
      m_buffer = buffer.m_buffer;
      m_writePos = buffer.m_writePos;
      m_readPos = buffer.m_readPos;
      m_maxLen = buffer.m_maxLen;
      m_lock = buffer.m_lock;

      buffer.m_buffer = nullptr;
      buffer.m_writePos = buffer.m_readPos = buffer.m_maxLen = 0;
      buffer.m_lock = false;
      return *this;
    }

    char*  NetBuffer::writeData(size_t len) {
      if(m_writePos + len > m_maxLen) { MYASSERT(false, "net buffer write pos error"); return nullptr; }
      char* p = writeData();
      m_writePos += len;
      return p;
    }
    char* NetBuffer::readData(size_t len) {
      MYASSERT(canRead(len), "net buffer read len error");
      if(!canRead(len)) { return nullptr; }
      char* p = readData();
      m_readPos += len;
      return p;
    }

    void NetBuffer::clearReadBuffer() {
      MYASSERT(m_readPos <= m_writePos, "net buffer m_readPos error");
      memmove(m_buffer, m_buffer + m_readPos, m_writePos - m_readPos);
      m_writePos -= m_readPos;
      m_readPos = 0;
    }
    void NetBuffer::reset(bool force ) {
      if(!force && m_lock) { MYASSERT(false, "buffer is lock"); return; }
      memset(m_buffer, 0, m_writePos);
      m_writePos = 0;
      m_readPos = 0;
    }
    bool NetBuffer::tailNetBuffer(const NetBuffer& buffer) {
      if(this == &buffer) { return false; }
      resize(m_writePos + buffer.m_writePos);
      return tailData(buffer.data(), buffer.length());
    }
    bool NetBuffer::tailResize(const char* base, size_t len){
      resize(m_writePos +  len);
      return tailData(base, len);
     }
    bool NetBuffer::tailData(const char* base, size_t len) {
      return tail(static_cast<void*>(const_cast<char*>(base)), len);
    }
    bool NetBuffer::headNetBuffer(const NetBuffer& buffer) {
      if(this == &buffer) { return false; }
      resize(m_writePos + buffer.m_writePos);
      return headData(buffer.data(), buffer.length());
    }
    bool NetBuffer::headData(const char* base, size_t len) {
      return head(static_cast<void*>(const_cast<char*>(base)), len);
    }
    bool NetBuffer::readBuffer(NetBuffer& buffer) {
      if(this == &buffer) { return false; }
      if(buffer.isFull()) { return false; }
      if(!hasWrite()) { return false; }
      size_t len = std::min<size_t>(m_writePos - m_readPos, buffer.maxLength() - buffer.m_writePos);
      char* p = readData(len);
      if(!p) { return false; }
      bool result = buffer.tail(p, len);
      return result;
    }

    bool NetBuffer::resize(size_t len) {
      if(m_lock) { MYASSERT(false, "buffer is lock"); return false; }
      if(len > m_maxLen) {
        void* p = std::realloc(m_buffer, len * 2);
        if (!p){ throw std::bad_alloc();}
        m_buffer = (char*)(p);
        m_maxLen = len * 2;
      }
      return true;
    }
    bool NetBuffer::tail(void* base, size_t len) {
      if(m_lock) { MYASSERT(false, "buffer is lock"); return false; }
      if(len <= 0) { return false; }
      MYASSERT(len + m_writePos <= m_maxLen, "buffer not has enough space");
      memcpy(m_buffer + m_writePos, base, len);
      m_writePos += len;
      return true;
    }
    bool NetBuffer::head(void* base, size_t len) {
      if(m_lock) { MYASSERT(false, "buffer is lock"); return false; }
      if(len <= 0) { return false; }
      MYASSERT(len + m_writePos <= m_maxLen, "buffer not has enough space");
      memmove(m_buffer + len, m_buffer, m_writePos);
      memcpy(m_buffer, base, len);
      m_writePos += len;
      return true;
    }
    bool NetBuffer::setHead(void* base, size_t len) {
      if(m_lock) { MYASSERT(false, "buffer is lock"); return false; }
      MYASSERT(len <= m_writePos , "buffer not has enough space in head");
      if(len <= m_writePos) {
        memcpy(m_buffer, base, len);
        return true;
      }
      return false;
    }

    NetBlock::NetBlock(SessionId s) :BlockBase(s), m_state(_STATE_INIT_){
      memset(&m_head, 0, sizeof(head));
    }
    NetBlock::NetBlock(SessionId s, BufferPointer buffer, uint16 mask) 
      : BlockBase(s){
      memset(&m_head, 0, sizeof(head));
      m_data = std::move(buffer);
      m_state = _STATE_DONE_;
      m_head._len = m_data->length();
      mask &= (~(1 << NetBlock::head::_MASK_COMPRESS_));
      m_head._mask = mask;
      m_head._check = Utility::crc32Buf(m_data->data() + sizeof(head), m_head._len - sizeof(head));
      m_data->setHeadPod(m_head);
    }

    void NetBlock::readBuffer(NetBuffer& buffer) {
      m_data->readBuffer(buffer);
    }
    BlockBase* NetBlock::clone(SessionId s) {
      NetBlock* p = new NetBlock(s);
      p->m_head = m_head;
      p->m_state = m_state;
      p->m_data = std::move(BufferPointer(new NetBuffer(m_data->length())));
      p->m_data->tailData(m_data->data(), m_data->length());
      p->m_data->lock();
      return p;
    }
    
    NetBlock::~NetBlock() {
      m_data.reset();
    }

    bool NetBlock::recv(BufferPointer& buf) {
      MYASSERT(nullptr != buf, "the recv buffer is null");
      if(!buf) { return false; }
      return recvHead(buf);
    }

    void NetBlock::lock(bool com) {
      if(m_data->isLock()) {
        MYASSERT(false);
        return;
      }
      m_head._len = m_data->length();
      //m_head._mask = m_head._mask;
      m_head._check = Utility::crc32Buf(m_data->data() + sizeof(head), m_head._len - sizeof(head));
      if(com) { compress(); }
      m_data->setHeadPod(m_head);
      m_data->lock();
    }

    //压缩消息接口
   void  NetBlock::compress() {
      if(m_head._mask & (1 << NetBlock::head::_MASK_COMPRESS_)) { return; }
      if(m_data && m_data->maxLength() - sizeof(NetBlock::head) > NetBlock::head::_COMPRESS_SIZE_) {
        std::string str;
        auto len = m_data->maxLength() - sizeof(NetBlock::head);
        if(!Utility::compressBuf(m_data->data() + sizeof(NetBlock::head), len, str)) {
          MYASSERT(false, "uncompress failed...");
          return;
        }
        if(len < str.length()) { return; }
        LOGDEBUG("compress len[", len, "] to [", str.length(), "] ", str.length() / float(len));
        m_data->reset();
        m_head._mask |= (1 << NetBlock::head::_MASK_COMPRESS_);
        m_head._len =sizeof(m_head) + str.length();
        m_data->tailPod<NetBlock::head>(m_head);
        m_data->tailData(str.c_str(), str.length());
      }
    }


    bool NetBlock::recvHead(BufferPointer& buf) {
      if(_STATE_INIT_ == m_state) {
        if(buf->readPod<head>(m_head)) {
          buf->clearReadBuffer();
          m_state = _STATE_BODY_;
          return recvBody(buf);
        } else { 
          return false; 
        }
      } 
      return recvBody(buf);
    }

    void NetBlock::uncompress() {
      if(done() && m_head._mask & (1 << NetBlock::head::_MASK_COMPRESS_)) {
        std::string str;
        if(!Utility::unCompressBuf(m_data->data(), m_data->maxLength(), str)) {
          MYASSERT(false, "uncompress failed...");
          return;
        }
        m_data->reset();
        m_data->tailResize(str.c_str(), str.length());
      }
    }
    bool NetBlock::recvBody(BufferPointer& buf) {
      if(_STATE_BODY_ == m_state) {
        if (m_head._len > 0xFFFFFF ){
          MYASSERT(false, "msg len is too large");
          m_state = _STATE_DONE_;
          setError(true);
          return true;
        }
        size_t len = m_head._len - sizeof(head);
        if(!m_data) { m_data = BufferPointer(new NetBuffer(len)); }
        buf->readBuffer(*m_data);
        buf->clearReadBuffer();
        if(m_data->isFull()) {
          m_state = _STATE_DONE_;
          uncompress();
          m_data->lock();
          auto crc = Utility::crc32Buf(m_data->data(), m_data->length());
          setError(m_head._check != crc);
          if (error()){LOGDEBUG("m_head._check:", m_head._check, " data crc:", crc);}
          return true;
        }
      }
      return false;
    }
  }
}