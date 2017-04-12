/********************************************************************
  Filename:   NetSession.cpp
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
      m_connect = nullptr;
      //m_connect = podMalloc<uv_connect_t>();
      m_write = podMalloc<uv_write_t>();
      m_shutDown = podMalloc<uv_shutdown_t>();
      m_timer = nullptr;

      //m_connect->data = this;
      m_write->data = this;
      m_shutDown->data = this;
      m_flag = 0;
      m_compressLen = config.m_compressLen;
    }
    NetSession::~NetSession() {
      if(m_tcp){ podFree(m_tcp); }
      if(m_connect){ podFree(m_connect); }
      if(m_write){ podFree(m_write); }
      if(m_shutDown){ podFree(m_shutDown); }
      if(m_timer){ podFree(m_timer); }

      m_bufferSend.reset();
      m_bufferRecv.reset();
      m_waiteMessage.reset();
      if (nullptr != m_res){
        uv_freeaddrinfo(m_res);
        m_res = nullptr;
      }

      m_tcp = nullptr;
      m_connect = nullptr;
      m_write = nullptr;
      m_timer = nullptr;

      LOGINFO("[net] free session:", m_sessionId);
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
          if (p->error()){ MYASSERT(false, "[net] session:", m_sessionId, " obj:", m_ObjName);}
          //MYASSERT(!p->error(), "crc32 check is error");
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
      if (nread < 0){
        int r = uv_shutdown(m_shutDown,
                            stream,
                            [](uv_shutdown_t* req, int){ auto s = static_cast<NetSession*>(req->data)->shared_from_this(); if(s){ if(!s->checkResetConnect()){ s->close(); } }});
        //LOGINFO("[net] afterRead session:", m_sessionId, " shutdown:", m_ObjName);
        uvError("uv_shutdown:", r);
      }
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
        stopTimer();
        setFlag(SESSION_CONNECT);
        setFlag(SESSION_CALL_CONN);
        setFlag(SESSION_FORCE);
        read();
        if(m_bufferSend->length() > 0 ){write();}
      }else{
        LOGINFO("[net] connect:", m_ObjName, " session:", m_sessionId, " failed status:", uv_strerror(status));
        if(flag(SESSION_RECONN)) {
          startTimer();
          /*LOGINFO("[net] reconnect server ", m_ip, " port:", m_port);*/
        } else {
          close();
        }
      }
    }

    void NetSession::close( /*bool call*/){
      if (uv_is_closing((uv_handle_t*)m_tcp)){
        LOGINFO("[net] s:", m_sessionId, " ", m_ObjName, " is closeing");
        return ;
      }

      if (m_timer){
        uv_timer_stop(m_timer);
        uv_close((uv_handle_t*)m_timer, nullptr);
      }

      uv_close((uv_handle_t*)m_tcp, [](uv_handle_t* handle){
        auto s = static_cast<NetSession*>(handle->data)->shared_from_this();
        if(s){ s->setFlag(SESSION_CAll_CLOS);}
      });
      clearFlag(SESSION_CONNECT);
    }

    void NetSession::clientSession(uv_loop_t * loop,
                                   const std::string& addr,
                                   int port,
                                   const RecvCall& recvNotify,
                                   const WriteCall& writeNotify,
                                   const KickCall& kickNotify,
                                   const SendCall& sendNotify){
      //m_addr = addr;
      m_sessionType = STYPE_TCP_CLIENT;
      m_tcp = podMalloc<uv_tcp_t>();
      int r = uv_tcp_init(loop, m_tcp);
      uvError("uv_tcp_init:", r);
      m_tcp->data = this;
      r = uv_tcp_nodelay(m_tcp, 1);
      uvError("uv_tcp_nodelay:", r);
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
      r = uv_getaddrinfo(loop, req, onResolved, m_ip.c_str(), std::to_string(m_port).c_str(),nullptr);
      if (r != 0){
        MYASSERT(false);
        podFree(req);
      }

      //connectServer();
    }


    void NetSession::httpClientSession(uv_loop_t * loop,
                                       const std::string& addr,
                                       int port,
                                       const RecvCall& recvNotify,
                                       MessagePtr ptr){
      m_sessionType = STYPE_HTTP_CLIENT;
      m_tcp = podMalloc<uv_tcp_t>();
      int r = uv_tcp_init(loop, m_tcp);
      uvError("uv_tcp_init:", r);
      m_tcp->data = this;
      m_ip = addr;
      m_port = port;
      m_threadAfterRead = recvNotify;
      m_threadAfterWrite = nullptr;
      m_nofitySend = nullptr;
      m_notifyKick = nullptr;
      if(ptr){takeToWriteBuffer(ptr);}

      uv_getaddrinfo_t* req = podMalloc<uv_getaddrinfo_t>();
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
      r = uv_getaddrinfo(loop, req, onResolved, m_ip.c_str(), std::to_string(m_port).c_str(), nullptr);
      if(r != 0){
        MYASSERT(false);
        podFree(req);
      }
    }


    void NetSession::startTimer(){
      MYASSERT(m_tcp);
      clearFlag(SESSION_CONNECT);
      uv_loop_t* loop = m_tcp->loop;
      uv_close((uv_handle_t*)m_tcp, [](uv_handle_t* handle){podFree<uv_tcp_t>((uv_tcp_t*)(handle)); });
      m_tcp = podMalloc<uv_tcp_t>();
      int r = uv_tcp_init(loop, m_tcp);
      uvError("uv_tcp_init:", r);
      m_tcp->data = this;
      r = uv_tcp_nodelay(m_tcp, 1);
      uvError("uv_tcp_nodelay:", r);

      if(!m_timer){
        m_timer = podMalloc<uv_timer_t>();
        m_timer->data = this;

        auto tcall = [](uv_timer_t* handle){
          SessionPtr s = static_cast<NetSession*>(handle->data)->shared_from_this();
          if(s){ s->connectServer(); }
        };
        uv_timer_init(m_tcp->loop, m_timer);
        uv_timer_start(m_timer, tcall, 500, 10000);
      } else{
        uv_timer_set_repeat(m_timer, 10000);
        uv_timer_again(m_timer);
      }

      m_bufferSend->reset(true);
      m_bufferRecv->reset(true);
      m_waiteMessage.reset();
    }

    void NetSession::stopTimer(){
      if(m_timer){
        uv_timer_stop(m_timer);
        //uv_close((uv_handle_t*)m_timer ,nullptr);
        //podFree(m_timer);
        //m_timer = nullptr;
      }
    }
    bool NetSession::connectServer() {
      MYASSERT(m_res);
      if(!m_res){return false;}
      auto call = [](uv_connect_t* req, int status){
        SessionPtr s = static_cast<NetSession*>(req->data)->shared_from_this();
        if(s){ s->connetResult(status);}
      };
      if(m_connect){
        podFree<uv_connect_t>(m_connect); 
        m_connect = nullptr;
      }
      m_connect = podMalloc<uv_connect_t>();
      m_connect->data = this;
      int r = uv_tcp_connect(m_connect, m_tcp, m_res->ai_addr, call);
      if(r != 0) {
        //LOGINFO("[net] connect ", m_ObjName, " error:", uv_err_name(r));
        startTimer();
      } else{
        stopTimer();
      }
      return true;
    }
    bool NetSession::serverSession(uv_tcp_t* tcp,
                                   const std::string& addr,
                                   const RecvCall& recvNotify,
                                   const WriteCall& writeNotify,
                                   const KickCall& kickNotify,
                                   const SendCall& sendNotify){
      m_sessionType = STYPE_TCP_SERVER;
      m_tcp = tcp;
      m_tcp->data = this;
      m_ip = addr;
      uv_tcp_keepalive(m_tcp, 1,1000);
      uv_tcp_nodelay(m_tcp, 1);
      m_threadAfterRead = recvNotify;
      m_threadAfterWrite = writeNotify;
      m_nofitySend = sendNotify;
      m_notifyKick = kickNotify;
      setFlag(SESSION_CONNECT);
      setFlag(SESSION_CALL_CONN);
      read();
      return true;
    }

    bool NetSession::httpServerSession(uv_tcp_t* tcp,
                                       const std::string& addr,
                                       const RecvCall& recvNotify,
                                       const SendCall& sendNotify){
      m_sessionType = STYPE_HTTP;
      m_tcp = tcp;
      m_tcp->data = this;
      m_ip = addr;
      m_threadAfterRead = recvNotify;
      m_threadAfterWrite = std::bind(&NetSession::close, this);
      m_nofitySend = sendNotify;
      m_notifyKick = nullptr;
      setFlag(SESSION_CONNECT);
      setFlag(SESSION_CALL_CONN);
      read();
      return true;
    }

    void NetSession::pushWrite(MessagePtr msg){
      msg->lock(flag(SESSSION_COMPRESS), m_compressLen);
      ++m_sendTotalCount;
      if(m_nofitySend){ m_nofitySend(msg); }
    }
    bool NetSession::takeToWriteBuffer(MessagePtr m){
      if (flag(SESSION_SEND) || m_bufferSend->isFull() || !m){return false;}
      if (!flag(SESSION_CONNECT) && m_sessionType != STYPE_HTTP_CLIENT){return false;}
      if (m_bufferSend->isLock()){MYASSERT(false); return false;}
      if (flag(SESSION_FORCE)){
        clearFlag(SESSION_FORCE);
        MYASSERT(m_bufferSend->length() == 0);
        //LOGINFO("[net] session[", m_sessionId, "] take to write with force, len");
        return m->readBuffer(*m_bufferSend, true);
       
      }else{ 
        return m->readBuffer(*m_bufferSend, false);
      }
    }
    void NetSession::write(){
      if (!flag(SESSION_CONNECT)){return;}
      if (flag(SESSION_SEND)){MYASSERT(false);return;}
      auto call = [](uv_write_t* req, int status){
        SessionPtr s = static_cast<NetSession*>(req->data)->shared_from_this();
        if(s){ s->afterWrite(status);}
      };
      if (m_bufferSend->isLock() || m_bufferSend->length() <= 0){
        MYASSERT(false);
        return ;
      }
      uv_buf_t buf;
      buf.base = m_bufferSend->data();
      buf.len = m_bufferSend->length();
      int r = uv_write(m_write, (uv_stream_t*)m_tcp, &buf, 1, call);
      uvError("uv_write:", r);
      m_bufferSend->lock();
      setFlag(SESSION_SEND);
    }

    void NetSession::afterWrite(int status) {
      if(status > 0) {
        int r = uv_shutdown(m_shutDown,
                            (uv_stream_t*)m_tcp,
                            [](uv_shutdown_t* req, int){ auto s = static_cast<NetSession*>(req->data)->shared_from_this();if(s){if(!s->checkResetConnect()){ s->close(); }};});
        uvError("uv_shutdown:", r);
        //LOGINFO("[net] afterWrite session:", m_sessionId, " shutdown:", m_ObjName);
        return;
      }
      m_sendTotalLen += m_bufferSend->length();
      m_bufferSend->reset(true);
      clearFlag(SESSION_SEND);
      if (m_threadAfterWrite){
        m_threadAfterWrite();
      }else{/*MYASSERT(false); */}
    }
    bool NetSession::setKicked(){
      setFlag(SESSION_KICK);
      clearFlag(SESSION_RECONN);
      if (m_notifyKick){
        m_notifyKick();
        return true;
      }
     /* MYASSERT(false);*/
      return false;
    }

    bool NetSession::checkResetConnect(){
     if (!flag(SESSION_RECONN)){return false;}
     if (STYPE_HTTP == m_sessionType){return false;}
     if (STYPE_TCP_SERVER == m_sessionType){return false;}
     startTimer();
     return true;
    }

    std::string NetSession::info() const{
      return std::move(MyLog::Log::makeString("session:", m_sessionId,
                                              "\naddress:", m_ip,
                                              "\nrecv len:", m_recvTotalLen, " count:", m_recvTotalCount,
                                              "\nsend len:", m_sendTotalLen, " count:", m_sendTotalCount,
                                              "\nname:", m_ObjName, " flag:", m_flag) );
    }
    //////////////////////////////////////////////////////////////////////////
  }
}
