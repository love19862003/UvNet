/********************************************************************

  Filename:   NetMessage

  Description:NetMessage

  Version:  1.0
  Created:  30:3:2015   14:09
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#ifndef __NetMessage_H__
#define __NetMessage_H__
#include <algorithm>
#include <memory>
#include <assert.h>
#include <functional>
#include "utility/NativeType.h"

namespace ShareSpace {
  namespace NetSpace {

    class  NetBuffer
    {
    public:
      explicit NetBuffer(size_t len);
      virtual ~NetBuffer();
      NetBuffer& operator = (NetBuffer&& buffer);

      inline bool isLock() const { return m_lock; }
      inline bool lock() { m_lock = true; return m_lock; }
      inline bool hasWrite() const { return length() > 0; }
      inline bool hasRead() const { return needReadLength() > 0; }
      inline bool isFull() const { return m_writePos >= m_maxLen; }
      inline bool canRead(size_t len) const { return len <= needReadLength(); }

      inline size_t length() const { return m_writePos; }
      inline size_t maxLength() const { return m_maxLen; }
      inline size_t needReadLength() const { return m_writePos - m_readPos; }

      inline const char* data() const { return m_buffer; }
      inline char* data() { return m_buffer; }

      inline const char* writeData() const { return m_buffer + m_writePos; }
      inline char* writeData() { return m_buffer + m_writePos; }
      char* writeData(size_t len);  // set  m_writePos += len && return the write pointer before add 

      inline const char* readData() const { return m_buffer + m_readPos; }
      inline char* readData() { return m_buffer + m_readPos; }
      char* readData(size_t len);   // set  m_readPos += len && return the read pointer before add 

      void clearReadBuffer();                             // clear read data
      void reset(bool force = false);                     // reset buffer
      bool tailNetBuffer(const NetBuffer& buffer);        // add buffer to this tail && if space is not enough, resize it
      bool tailData(const char* base, size_t len);        // add buffer to this tail && if space is not enough, resize it

      bool tailResize(const char* base, size_t len);

      //add a pod data to this tail && if space is not enough, resize it
      template <typename PODTYPE>
      bool tailPod(const PODTYPE& t) {
        assert(std::is_pod<PODTYPE>::value);
        resize(m_writePos + sizeof(PODTYPE));
        return tail(static_cast<void*>(const_cast<PODTYPE*>(&t)), sizeof(PODTYPE));
      }

      bool headNetBuffer(const NetBuffer& buffer);      // add buffer to this head && if space is not enough, resize it
      bool headData(const char* base, size_t len);      // add buffer to this head && if space is not enough, resize it
      
      //add a pod data to this head && if space is not enough, resize it
      template <typename PODTYPE>
      bool headPod(const PODTYPE& t) {
        assert(std::is_pod<PODTYPE>::value);
        resize(m_writePos + sizeof(PODTYPE));
        return head(static_cast<void*>(const_cast<PODTYPE*>(&t)), sizeof(PODTYPE));
      }

      // add buffer to this head, you must ensure the head is has enough space .. 
      template <typename PODTYPE>
      bool setHeadPod(const PODTYPE& t) {
        assert(std::is_pod<PODTYPE>::value);
        return setHead(static_cast<void*>(const_cast<PODTYPE*>(&t)), sizeof(PODTYPE));
      }

      // read data to buffer . the read len is std::min<m_writePos - m_readPos, buffer.m_maxLen - buffer.m_writePos >
      bool readBuffer(NetBuffer& buffer);

      // read a pod data 
      template <typename PODTYPE>
      bool readPod(PODTYPE& t) {
        assert(std::is_pod<PODTYPE>::value);
        if(!canRead(sizeof(PODTYPE))) { return false; }
        char* p = readData(sizeof(PODTYPE));
        if(!p) { return false; }
        t = *((PODTYPE*)(p));
        return true;
      }
    protected:
    private:
      // resize  a new len if len > m_maxLen;
      bool resize(size_t len);
      // add base to this tail . you must ensure has enough space 
      bool tail(void* base, size_t len);
      // add base to this head . you must ensure has enough space
      bool head(void* base, size_t len);
      // add base to this head . you must ensure len < m_writePos
      bool setHead(void* base, size_t len);
    private:
      char* m_buffer;
      size_t m_maxLen;       // buffer space len
      size_t m_writePos;     // buffer write len
      size_t m_readPos;      // buffer read len
      bool m_lock;           // buffer is lock . if lock you can only read
      NetBuffer& operator = (const NetBuffer&) = delete;
      NetBuffer(const NetBuffer&) = delete;
    };

    typedef std::unique_ptr<NetBuffer> BufferPointer;
    typedef unsigned int SessionId;
    enum {
      INVALID_SESSION_ID = 0,
    };

    class BlockBase{
    public:
      explicit BlockBase(SessionId s)
      : m_session(s)
      , m_error(false){ 

      }
      virtual ~BlockBase() { /*m_data.reset();*/ }
      typedef std::function<void(BlockBase*)> CompressFunType;
      //const BufferPointer& data() const { return m_data; }
      //BufferPointer& data() { return m_data; }
      const SessionId& session() const { return m_session; }
      bool error() const { return m_error; }

      virtual BlockBase* clone(SessionId s) = 0;
      virtual bool done() const = 0;
      virtual bool recv(BufferPointer& buf) = 0;
      virtual void lock(bool compress) = 0;
      virtual void readBuffer(NetBuffer& buffer) = 0;
      virtual size_t length() = 0;
      virtual bool readComplete() const = 0;

    protected:
      void setError(bool e) { m_error = e; }
    private:
      BlockBase(const BlockBase&) = delete;
      BlockBase& operator = (const BlockBase&) = delete;
      BlockBase& operator = (BlockBase&&) = delete;
      
      SessionId m_session;
      bool m_error = false;
    };

    class NetBlock : public BlockBase {
    public:
#pragma pack(push, 1)
      struct head {
        enum {
          _MASK_COMPRESS_ = 0,
          _COMPRESS_SIZE_ = 1024,
        };
        typedef uint32 head_len_type;
        typedef uint16 head_mask_type;
        typedef uint32 head_check_type;

        head_len_type   _len;     //message stream len
        head_mask_type  _mask;    //message mask
        head_check_type _check;   //check msg valid
      };
#pragma pack(pop)

      enum State {
        _STATE_INIT_ = 1,
        _STATE_BODY_ = 2,
        _STATE_DONE_ = 3,
      };

      // constructor for send 
      explicit NetBlock(SessionId s, BufferPointer buffer, uint16 mask = 0);
      // constructor for recv 
      explicit NetBlock(SessionId s);
      virtual ~NetBlock();
     
      const BufferPointer& data() { return m_data; }

      virtual bool done() const override{ return m_state == _STATE_DONE_; }
      virtual bool recv(BufferPointer& buf) override;
      virtual BlockBase* clone(SessionId s) override;
      virtual void lock(bool compress) override;
      virtual void readBuffer(NetBuffer& buffer) override;
      virtual size_t length() override { return m_data ? m_data->maxLength() : 0; }
      virtual bool readComplete() const override { return !m_data->hasRead(); }
      
    private:
      // read recv head
      bool recvHead(BufferPointer& buf);
      // read recv body
      bool recvBody(BufferPointer& buf);

      bool uncompress();
      void compress();
    private:
      NetBlock(const NetBlock&) = delete;
      NetBlock& operator = (const NetBlock&) = delete;
      NetBlock& operator = (NetBlock&&) = delete;
      BufferPointer m_data;
      head m_head;
      State m_state;
    };
  }
}
#endif