/********************************************************************
	Filename: 	NetThread.cpp
	Description:
	Version:  1.0
	Created:  31:3:2016   10:48
	
	Compiler: gcc vc
	Author:   wufan, love19862003@163.com
	Organization: lezhuogame
*********************************************************************/
#include "net/NetThread.h"
#include "net/NetSession.h"
#include "net/NetObj.h"
namespace ShareSpace {
  namespace NetSpace {
    
    void callAsyncAddObject(uv_async_t* handle){
      NetThread* p = static_cast<NetThread*>(handle->data);
      if(p){ p->asyncAddObject(); }
    }
    void callAsyncStopThread(uv_async_t* handle) {
      NetThread* p = static_cast<NetThread*>(handle->data);
      if(p) { p->asyncStopThread(); }
    }
    void callAsyncKickSession(uv_async_t* handle) {
      NetThread* p = static_cast<NetThread*>(handle->data);
      if(p) { p->asyncKickSession(); }
    }
    void callAsyncSendMessage(uv_async_t* handle) {
      NetThread* p = static_cast<NetThread*>(handle->data);
      if(p) { p->realSend(); }
    }

    static void closeThreadSync(uv_handle_t* /*handle*/) {
      LOGDEBUG("close work thread sync");
    }
    NetThread::NetThread() : m_state(_INIT_), m_onlines(0) {
      m_loop = podMalloc<uv_loop_t>();
      m_loop->data = this;
      for(auto& a : m_asyncs) {
        a.data = this;
      }
    }
    NetThread::~NetThread() {
      podFree<uv_loop_t>(m_loop);
      m_loop = nullptr;
      m_onlieList.clear();
      m_recvList.clear();
      m_sendList.clear();
    }

    size_t  NetThread::busy() const {
      return m_onlines > 0 ? (m_totalSendSize + m_totalRealRecv) / m_onlines : 0;
    }
    void  NetThread::stop() {
      int r = uv_async_send(&m_asyncs[NetThread::async_stop_thread]);
      uvError("uv_async_send:", r);
      m_state = _STOP_;
      m_thread_work.join();
    }
    void NetThread::start() {
      m_thread_work = std::move(std::thread(std::bind( &NetThread::threadRun, this)));
    }
    void NetThread::addSession(SessionPtr s) {
      std::lock_guard<std::mutex>lock(m_mutexs[mutex_session]);
      m_onlieList.insert(s);
      m_onlines = m_onlieList.size();
    }
    bool NetThread::check() {
      std::lock_guard<std::mutex>lock(m_mutexs[mutex_send]);
      return !m_sendList.empty();
    }
    void NetThread::asyncAddObject(){
       std::lock_guard<std::mutex> locker(m_mutexs[mutex_object]);
       for (auto& obj : m_objects){
         obj->start();
       }
    }
    void NetThread::asyncStopThread(){
      for(auto& obj : m_objects){ obj->stop(); }
      realSend();
      {
        std::lock_guard<std::mutex>lock(m_mutexs[mutex_session]);
        for(auto& s : m_onlieList){s->close();}
      }
      for(auto& v : m_asyncs) { uv_close((uv_handle_t*)&v, closeThreadSync); }
      int r = uv_loop_alive(m_loop);
      while(r == 0) {
        r = uv_loop_alive(m_loop);
        uv_stop(m_loop);
        LOGDEBUG("set work listen thread stop");
        break;
      }
    }
    void NetThread::asyncKickSession(){
      for (auto& s : m_onlieList){
        if (s->flag(NetSession::SESSION_KICK)){ 
          s->close(); 
          s->clearFlag(NetSession::SESSION_KICK);
        }
      }
    }

    void NetThread::threadRun(){
      int r = uv_loop_init(m_loop);
      uvError("uv_loop_init:", r);
      r = uv_async_init(m_loop, &m_asyncs[NetThread::async_add_object], callAsyncAddObject);
      uvError("uv_async_init:", r);
      r = uv_async_init(m_loop, &m_asyncs[NetThread::async_stop_thread], callAsyncStopThread);
      uvError("uv_async_init:", r);
      r = uv_async_init(m_loop, &m_asyncs[NetThread::async_kick_session], callAsyncKickSession);
      uvError("uv_async_init:", r);
      r = uv_async_init(m_loop, &m_asyncs[NetThread::async_send_message], callAsyncSendMessage);
      uvError("uv_async_init:", r);
      for(auto& obj : m_objects) { obj->start(); }
      m_state = _RUN_;
      r = uv_run(m_loop, UV_RUN_DEFAULT);
      uvError("uv_run:", r);
      r = uv_loop_close(m_loop);
      uvError("uv_loop_close:", r);
      LOGDEBUG("exit net work thread");
    }

    void NetThread::recvMsgList(const std::list<MessagePtr>& list, size_t size) {
      std::lock_guard<std::mutex>lock(m_mutexs[mutex_recive]);
      m_recvList.insert(m_recvList.end(), list.begin(), list.end());
      m_totalRealRecvCount += list.size();
      m_totalRealRecv += size;
    }
    void NetThread::realSend(){
      auto sessionFun = [&](SessionId id)->SessionPtr {
        auto it = std::find_if(m_onlieList.begin(), m_onlieList.end(), [id](const SessionPtr& s) { return s->id() == id; });
        if(it != m_onlieList.end()) { return  *it; }
        return nullptr;
      };
      std::set<SessionPtr> sset;
      sset.clear();
      std::lock_guard<std::mutex>lock(m_mutexs[mutex_send]);
      if(m_sendList.empty()) { return; }
      auto it = m_sendList.begin();
      auto itEnd = m_sendList.end();
      while(it != itEnd) {
        auto& msg = *it;
        size_t size = msg->length();
        auto s = sessionFun(msg->session());
        if(!s) {
          it = m_sendList.erase(it);
          continue;
        }
        if(!s->flag(NetSession::SESSION_CONNECT)) {
          ++it;
          continue;
        }
       
        if (!s->takeToWriteBuffer(msg)){
          ++it;
          continue;
        }
        sset.insert(s);
        if(msg->readComplete()) {
          it = m_sendList.erase(it);
          m_totalRealSendCount++;
          m_totalRealSendSize += size;
        } else {
          ++it;
        }
      }
      //read send
      for(auto& s : sset) { s->write(); }
    }

    bool NetThread::pollThread(std::list<MessagePtr>& msg, std::list<SessionPtr>& conn, std::list<SessionPtr>&dis){
      if (_RUN_ == m_state){
        {
          std::lock_guard<std::mutex>lock(m_mutexs[mutex_recive]);
          msg.insert(msg.end(),m_recvList.begin(), m_recvList.end());
          m_recvList.clear();
        }  
        {
          std::lock_guard<std::mutex>lock(m_mutexs[mutex_session]);
          for (auto it = m_onlieList.begin(); it != m_onlieList.end();){
            auto s = *it;
            if(s->flag(NetSession::SESSION_CALL_CONN)){ 
              conn.push_back(s);
              s->clearFlag(NetSession::SESSION_CALL_CONN);
            }
            if(s->flag(NetSession::SESSION_CAll_CLOS)){ 
              dis.push_back(s);
              s->clearFlag(NetSession::SESSION_CAll_CLOS);
              it = m_onlieList.erase(it);
              continue;
            }
            ++it;
          }
           m_onlines = m_onlieList.size();
        }
        return true;
      }
      return false;
    }


    void NetThread::notifySend(MessagePtr msg){
      m_totalSendCount++;
      m_totalSendSize += msg->length();
      std::lock_guard<std::mutex>lock(m_mutexs[mutex_send]);
      m_sendList.push_back(msg);
      int r = uv_async_send(&m_asyncs[NetThread::async_send_message]);
      uvError("uv_async_send:", r);
    }
   
    bool NetThread::notifyKick(){
      int r = uv_async_send(&m_asyncs[async_kick_session]);
      uvError("uv_async_send:", r);
      return true;
    }
    //增加一个网络对象
    bool NetThread::addObject(ObjectPtr obj){
      if (_RUN_ == m_state){
        std::lock_guard<std::mutex> locker(m_mutexs[mutex_object]);
        m_objects.push_back(obj);
        int r = uv_async_send(&m_asyncs[NetThread::async_add_object]);
        uvError("uv_async_send:", r);
      } else{
        m_objects.push_back(obj);
      }
      return true;
    }
    //////////////////////////////////////////////////////////////////////////
    


  }
}