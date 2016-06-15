/********************************************************************
	Filename: 	NetSession.cpp
	Description:
	Version:  1.0
	Created:  31:3:2016   11:11

	Compiler: gcc vc
	Author:   wufan, love19862003@163.com
	Organization: lezhuogame
*********************************************************************/
#include "net/NetSession.h"
#include "net/NetThread.h"
#include "utility/Compress.h"
#include "utility/StringUtility.h"
namespace ShareSpace {
  namespace NetSpace {

//     void callWrite(uv_write_t* req, int status) {
//       SessionPtr s = static_cast<NetSession*>(req->data)->shared_from_this();
//       if (s){
//         s->afterWrite(status);
//       }
//     }
//     void callConnect(uv_connect_t* req, int status){
//       SessionPtr s = static_cast<NetSession*>(req->data)->shared_from_this();
//       if (s){
//         s->connetResult(status);
//       }
//     }
//     void callAlloc(uv_handle_t* handle, size_t len, uv_buf_t* buff) {
//       SessionPtr s = static_cast<NetSession*>(handle->data)->shared_from_this();
//       if(s) {
//         s->allocReadBuffer(len, buff);
//       }else{
//         MYASSERT(false);
//       }
//     }
//     void callRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
//       SessionPtr s = static_cast<NetSession*>(stream->data)->shared_from_this();
//       if (s){
//         s->afterRead(stream, nread, buf);
//       }
//     }
//     void callShutDown(uv_shutdown_t* req, int /*status*/) {
//       SessionPtr s = static_cast<NetSession*>(req->data)->shared_from_this();
//       if(s) {
//         s->close();
//       }
//     }
//     void callClose(uv_handle_t* handle){
//       SessionPtr s = (static_cast<NetSession*>(handle->data))->shared_from_this();
//       if(s) { s->afterClose(); }
//     }
    NetSession::NetSession(SessionId id,
                        size_t len,
                        const Config& config,
                        const FunMakeBlock& fun)
                        : m_sessionId(id)
                        , m_bufferSend(new NetBuffer(len))
                        , m_bufferRecv(new NetBuffer(len))
                        , m_waiteMessage(nullptr)
                        , m_ObjName(config.m_name)
                        ,m_makeBlockFun(fun){
      m_tcp = nullptr;
      m_connect = podMalloc<uv_connect_t>();
      m_write = podMalloc<uv_write_t>();
      m_shutDown = podMalloc<uv_shutdown_t>();

      m_connect->data = this;
      m_write->data = this;
      m_shutDown->data = this;
      m_flag = 0;
    }
    NetSession::~NetSession() {
      if(m_tcp){podFree(m_tcp); }
      podFree(m_connect);
      podFree(m_write);
      podFree(m_shutDown);

      m_bufferSend.reset();
      m_bufferRecv.reset();
      m_waiteMessage.reset();
      if (nullptr != m_res){
        uv_freeaddrinfo(m_res);
        m_res = nullptr;
      }
    }

    bool NetSession::flag(unsigned int f) const{
      return (m_flag & f) > 0;
    }
    // set flag
    void NetSession::setFlag(unsigned int f){
     // LOGDEBUG(m_flag);
      m_flag |= f;
      //LOGDEBUG(m_flag);
    }
    //clear flag
    void NetSession::clearFlag(unsigned int f){
      //LOGDEBUG(m_flag);
      m_flag &= (~f);
      //LOGDEBUG(m_flag);
    }
    void NetSession::allocReadBuffer(size_t len, uv_buf_t* buff) {
      size_t ll = m_bufferRecv->maxLength() - m_bufferRecv->length();
      size_t l = std::min<size_t>(len, ll);
      uv_buf_t b = uv_buf_init(m_bufferRecv->writeData(), l);
      *buff = b;
    }
    MessagePtr NetSession::readMessage(){
      if(!m_waiteMessage) { m_waiteMessage = m_makeBlockFun(m_sessionId); }
      if(m_waiteMessage) {
        m_waiteMessage->recv(m_bufferRecv);
        if(m_waiteMessage->done()) {
          auto p = m_waiteMessage;
          MYASSERT(!p->error(), "crc32 check is error");
          m_waiteMessage = nullptr;
          return p;
        }
      }
      return nullptr;
    }
    bool NetSession::afterRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
      if(nread >= 0 && buf && buf->base && buf->len > 0) {
        std::list<MessagePtr> list;
        size_t size = 0;
        m_bufferRecv->writeData(nread);
        while(m_bufferRecv->hasWrite()) {
          auto m = readMessage();
          if(m) {
            list.push_back(m);
            m_recvTotalCount++;
            m_recvTotalLen += m->length();
            size += m->length();
          } else { break; }
        }
        if (m_threadAfterRead){
          m_threadAfterRead(list, size);
          return true;
        }else{
          MYASSERT(false);
          return false;
        }
      }
      int r = uv_shutdown(m_shutDown,
                          stream, 
                          [](uv_shutdown_t* req, int){
        auto s = static_cast<NetSession*>(req->data)->shared_from_this(); 
        if(s){s->close();}});
      uvError("uv_shutdown:", r);
      return false;
    }
    void NetSession::read(){
      auto allocCall = [](uv_handle_t* handle, size_t len, uv_buf_t* buff){
        SessionPtr s = static_cast<NetSession*>(handle->data)->shared_from_this();
        if(s){
          s->allocReadBuffer(len, buff);
        } else{
          MYASSERT(false);
        }
      };

      auto readCall = [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf){
        SessionPtr s = static_cast<NetSession*>(stream->data)->shared_from_this();
        if(s){
          s->afterRead(stream, nread, buf);
        }
      };
      int r = uv_read_start((uv_stream_t*)m_tcp, allocCall, readCall);
      uvError("begin read " + m_ObjName + " session:" + std::to_string(m_sessionId) + " r:", r);
      return;
    }
    void NetSession::connetResult(int status){
      if (0 == status){
        setFlag(SESSION_CONNECT);
        setFlag(SESSION_CALL_CONN);
        read();
      }else{
        LOGINFO("connect ", m_ObjName, " failed");
        if(flag(SESSION_RECONN)) {
          connectServer();
        } else {
          close();
        }
      }
    }

    void NetSession::close( bool call){
      if (call){
        uv_close((uv_handle_t*)m_tcp, 
                 [](uv_handle_t* handle){auto s = static_cast<NetSession*>(handle->data)->shared_from_this();if(s){s->afterClose();}});
      }else{
        clearFlag(SESSION_CONNECT);
        uv_close((uv_handle_t*)m_tcp, nullptr);
      }
      
    }
    void NetSession::afterClose(){
      if (flag(SESSION_RECONN) ){
        m_waiteMessage.reset();
        m_bufferRecv->reset(true);
        m_bufferSend->reset(true);
        MYASSERT(false);
        connectServer();
        return ;
      }
      setFlag(SESSION_CAll_CLOS);
      clearFlag(SESSION_CONNECT);
    }

//     void onResolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res){
//       if(status == -1){
//         uvError("client reslover net error", status);
//         return;
//       }
//       SessionPtr s = static_cast<NetSession*>(resolver->data)->shared_from_this();
//       MYASSERT(s);
//       s->m_res = res;
//       s->connectServer();
//       podFree(resolver);
//    
    void NetSession::clientSession(uv_tcp_t * tcp,
                                   const std::string& addr,
                                   int port,
                                   const RecvCall& recvNotify,
                                   const WriteCall& writeNotify,
                                   const KickCall& kickNotify,
                                   const SendCall& sendNotify){
      //m_addr = addr;
      m_tcp = tcp;
      m_tcp->data = this;
      m_ip = addr;
      m_port = port;
      m_threadAfterRead = recvNotify;
      m_threadAfterWrite = writeNotify;
      m_nofitySend = sendNotify;
      m_notifyKick = kickNotify;

      uv_getaddrinfo_t* req = podMalloc<uv_getaddrinfo_t>() ;
      req->data = this;
      auto onResolved = [](uv_getaddrinfo_t *resolver, int status, struct addrinfo *res){
        if(status == -1){
          uvError("client reslover net error", status);
          return;
        }
        SessionPtr s = static_cast<NetSession*>(resolver->data)->shared_from_this();
        if(s){
          s->m_res = res;
          s->connectServer();
          podFree(resolver);
        }
      };
      int r = uv_getaddrinfo(tcp->loop, req, onResolved, m_ip.c_str(), std::to_string(m_port).c_str(),nullptr);
      if (r != 0){
        MYASSERT(false);
        podFree(req);
      }

      //connectServer();
    }
    bool NetSession::connectServer() {
      MYASSERT(m_res);
      if(!m_res){return false;}
      auto call = [](uv_connect_t* req, int status){
        SessionPtr s = static_cast<NetSession*>(req->data)->shared_from_this();
        if(s){
          s->connetResult(status);
        }
      };
      int r = uv_tcp_connect(m_connect, m_tcp, m_res->ai_addr, call);
      if(r != 0) { 
        LOGDEBUG("connect ", m_ObjName, " error:", uv_err_name(r)); 
        connectServer(); 
      }
      return true;
    }
    bool NetSession::serverSession(uv_tcp_t* tcp,
                                   const std::string& addr,
                                   const RecvCall& recvNotify,
                                   const WriteCall& writeNotify,
                                   const KickCall& kickNotify,
                                   const SendCall& sendNotify){
      m_tcp = tcp;
      m_tcp->data = this;
      m_ip = addr;
      m_threadAfterRead = recvNotify;
      m_threadAfterWrite = writeNotify;
      m_nofitySend = sendNotify;
      m_notifyKick = kickNotify;
      setFlag(SESSION_CONNECT);
      setFlag(SESSION_CALL_CONN);
      read();
      return true;
    }

    void NetSession::pushWrite(MessagePtr msg){
      msg->lock(flag(SESSSION_COMPRESS));
      ++m_sendTotalCount;
      if(m_nofitySend){ m_nofitySend(msg); }
    }
    bool NetSession::takeToWriteBuffer(MessagePtr m){
      if (flag(SESSION_SEND) || m_bufferSend->isFull()){return false;}
      m->readBuffer(*m_bufferSend);
      return true;
    }
    void NetSession::write(){
      auto call = [](uv_write_t* req, int status){
        SessionPtr s = static_cast<NetSession*>(req->data)->shared_from_this();
        if(s){
          s->afterWrite(status);
        }
      };
      uv_buf_t buf;
      buf.base = m_bufferSend->data();
      buf.len = m_bufferSend->length();
      int r = uv_write(m_write, (uv_stream_t*)m_tcp, &buf, 1, call);
      uvError("uv_write:", r);
      setFlag(SESSION_SEND);
    }

    void NetSession::afterWrite(int status) {
      if(status > 0) {
        int r = uv_shutdown(m_shutDown, 
                            (uv_stream_t*)m_tcp, 
                            [](uv_shutdown_t* req, int){
          auto s = static_cast<NetSession*>(req->data)->shared_from_this();
          if(s){s->close();};
        });
        uvError("uv_shutdown:", r);
        return;
      }
      m_sendTotalLen += m_bufferSend->length();
      m_bufferSend->reset();
      clearFlag(SESSION_SEND);
      if (m_threadAfterWrite){
        m_threadAfterWrite();
      }else{MYASSERT(false); }
    }
    bool NetSession::setKicked(){
      setFlag(SESSION_KICK);
      clearFlag(SESSION_RECONN);
      if (m_notifyKick){
        m_notifyKick();
        return true;
      }
      MYASSERT(false);
      return false;
    }

    std::string NetSession::info() const{
      return std::move(MyLog::Log::makeString("session:", m_sessionId,
                                              "\naddress:", m_ip, 
                                              "\nrecv len:", m_recvTotalLen, " count:", m_recvTotalCount,
                                              "\nsend len:", m_sendTotalLen, " count:", m_sendTotalCount));

    }



    //////////////////////////////////////////////////////////////////////////
  }
}
