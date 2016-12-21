/********************************************************************

  Filename:   NetSession

  Description:NetSession

  Version:  1.0
  Created:  31:3:2015   16:03
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#ifndef __NetSession_H__
#define __NetSession_H__
#include "net/NetCore.h"
#include "net/NetUV.h"
#include <atomic>
namespace ShareSpace {
  namespace NetSpace {
    class NetSession : public std::enable_shared_from_this<NetSession> {
    private:
      NetSession(const NetSession&) = delete;
      NetSession& operator = (const NetSession&) = delete;
    public:
      friend class NetThread;

      typedef std::function<void(const std::list<MessagePtr>&, size_t)> RecvCall;
      typedef std::function<void()> WriteCall;
      typedef std::function<void()> KickCall;
      typedef std::function<void(MessagePtr)> SendCall;


      enum{
        SESSION_CONNECT = 0x0001,         // connected
        SESSION_CALL_CONN = 0x0002,       // call connect
        SESSION_CAll_CLOS = 0x0004,       // call close
        SESSION_SEND = 0x0008,            // in sending
        SESSION_RECONN = 0x0010,          // reconnect flag
        SESSSION_COMPRESS = 0x0020,       // need compress
        SESSION_KICK = 0x0040,            // be kicked
        SESSION_CLEAR = 0x0080,           // clear request
      };
      explicit NetSession(SessionId id,
                          size_t len,
                          const Config& config,
                          const FunMakeBlock& fun);
      ~NetSession();
      // check flag
      bool flag(unsigned int f) const;
      // set flag
      void setFlag(unsigned int f); 
      //clear flag
      void clearFlag(unsigned int f);

      // main thread
      void pushWrite(MessagePtr msg);
      // main thread
      bool setKicked();
      //thread safe
      const std::string& netName() const { return m_ObjName; }
      // work thread
      void clientSession(uv_tcp_t * tcp,
                         const std::string& addr,
                         int port,
                         const RecvCall& recvNotify,
                         const WriteCall& writeNotify,
                         const KickCall& kickNotify,
                         const SendCall& sendNotify);
      // work thread
      bool serverSession(uv_tcp_t* tcp, 
                         const std::string& addr,
                         const RecvCall& recvNotify,
                         const WriteCall& writeNotify,
                         const KickCall& kickNotify,
                         const SendCall& sendNotify);


      //word thread
      bool httpServerSession(uv_tcp_t* tcp,
                             const std::string& addr,
                             const RecvCall& recvNotify,
                             const SendCall& sendNotify);
      // work thread
      void httpClientSession(uv_tcp_t * tcp,
                             const std::string& addr,
                             int port,
                             const RecvCall& recvNotify,
                             MessagePtr ptr);
      // thread safe
      const std::string& remoteAddress(){ return m_ip; }

      std::string info() const;
      // thread safe
      SessionId id() const{ return m_sessionId; }
    private:
      // work thread
      void close();
      // work thread
      bool takeToWriteBuffer(MessagePtr m);
      // work thread
      void write();
      // work thread
      void connetResult(int status);
      // work thread
      /*void afterClose();*/
      // work thread
      void allocReadBuffer(size_t len, uv_buf_t* buff);
      // work thread
      bool afterRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
      // work thread
      void afterWrite(int status);
    private:
      // work thread
      MessagePtr readMessage();
      // work thread
      bool connectServer();
      // work thread
      void read();

      //
      void startTimer();
      void stopTimer();
    private:
      uv_tcp_t*      m_tcp;       //tcp
      uv_connect_t*  m_connect;   //conn
      uv_write_t*    m_write;     //write
      uv_shutdown_t* m_shutDown;  //shut down
      uv_timer_t*    m_timer;     //timer

      SessionId m_sessionId;        //id
      BufferPointer m_bufferSend;   //cache for send
      BufferPointer m_bufferRecv;   //cache for recv
      MessagePtr m_waiteMessage;    //unpack message
      const std::string m_ObjName;  //net object name
      FunMakeBlock m_makeBlockFun;  //message creator
      std::string m_ip = "";        //address
      int m_port = 0;
      size_t m_recvTotalLen = 0;    //total bytes recv
      size_t m_recvTotalCount = 0;  //total message count recv
      size_t m_sendTotalLen = 0;    //total bytes send
      size_t m_sendTotalCount = 0;  //total message count send
      struct addrinfo * m_res = nullptr;

      unsigned int m_flag = 0 ;
      RecvCall m_threadAfterRead = nullptr;
      WriteCall m_threadAfterWrite = nullptr;
      KickCall m_notifyKick = nullptr;
      SendCall m_nofitySend = nullptr;
    };

  }
}
#endif