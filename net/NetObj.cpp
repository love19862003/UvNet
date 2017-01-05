/********************************************************************
    Filename: NetObj.cpp
    Description:
    Version:  1.0
    Created:  13:4:2016   13:48

    Compiler: gcc vc
    Author:   wufan, love19862003@163.com
    Organization: lezhuogame
*********************************************************************/
#include <string.h>
#include "net/NetObj.h"
#include "net/NetThread.h"
#include "net/NetSession.h"
#include "utility/StringUtility.h"
namespace ShareSpace{
  namespace NetSpace{

    class TcpServer : public ObjectBase{
      enum {buffer_len = 256,};
    public:
      explicit TcpServer(const NetProperty& property, FunCreateSession fun) : ObjectBase(property, fun){
        m_tcp = podMalloc<uv_tcp_t>() ;
        m_allow = property.config().m_allow;
        m_address = property.config().m_address;
        m_port = property.config().m_port;
        m_maxConnect = property.config().m_maxConnect;
        /*m_ip6 = Utility::isIPV6(m_address);*/
      }
      virtual ~TcpServer(){
        if (m_tcp){
          podFree(m_tcp);
          m_tcp = nullptr;
        }
      }

      void sessionConnect(){
        uv_tcp_t*  tcp = podMalloc<uv_tcp_t>();
        int r = uv_tcp_init(m_tcp->loop, tcp);
        uvError("uv_tcp_init:", r);
        setNoPIPEOptional(tcp);
        if(0 == uv_accept((uv_stream_t*)m_tcp, (uv_stream_t*)tcp)){
          sockAddress address;
          char buf[buffer_len];
          memset(buf, 0, sizeof(buf));
          int len = sizeof(sockaddr_in6);
          r = uv_tcp_getpeername(tcp, &address.addr, &len);
          if (0 == r){
             r = uv_ip6_name(&address.in6, buf, sizeof(buf));
          } else{
            len = sizeof(sockaddr_in);
            r = uv_tcp_getpeername(tcp, &address.addr, &len);
            r = uv_ip4_name(&address.in, buf, sizeof(buf));
          }
          auto s = createSession();
          if(config().m_compress){s->setFlag(NetSession::SESSSION_COMPRESS);}
          auto t = thread();
          s->serverSession(tcp,
                           buf,
                           std::bind(&NetThread::recvMsgList, t.get(), std::placeholders::_1, std::placeholders::_2),
                           std::bind(&NetThread::realSend, t.get()),
                           std::bind(&NetThread::notifyKick, t.get()),
                           std::bind(&NetThread::notifySend, t.get(), std::placeholders::_1));
          t->addSession(s);
        } else{
          uv_close((uv_handle_t*)tcp, [](uv_handle_t* t){ podFree((uv_tcp_t*)(t));});
        }
      }

      virtual bool start() override{
        if (0 == (m_flag & _TAG_ADD )){
          return false;
        }
        m_flag &= (~_TAG_ADD);
        auto t = thread();
        uv_loop_t* loop = t->loop();
        m_tcp = podMalloc<uv_tcp_t>();
        m_tcp->data = this;
        int r = 0 ;
        r = uv_tcp_init(loop, m_tcp);
        uvError("uv_tcp_init:", r);
        sockAddress addr;
        r = uv_ip6_addr("::0", m_port, &addr.in6);
        uvError("uv_ip6_addr:", r);
        if (0 != r){
          r = uv_ip4_addr("0.0.0.0", m_port, &addr.in);
          uvError("uv_ip4_addr:", r);
        }
        r = uv_tcp_bind(m_tcp, &addr.addr, 0);
        uvError("uv_tcp_bind:", r);

        r = uv_listen((uv_stream_t*)(m_tcp),
                      m_maxConnect,
                      [](uv_stream_t* server, int status){
          if(status == -1){ return; }
          TcpServer* s = static_cast<TcpServer*>(server->data);
          if(s){ s->sessionConnect(); }
        });
        uvError("uv_listen:", r);
        return true;
      }
      virtual bool stop() override{
        uv_close((uv_handle_t*)m_tcp, nullptr);
        return true;
      }
      virtual bool allow(SessionPtr s)  override{
        if (m_allow.empty()){ return true;}
        return Utility::isPassAddress(m_allow, s->remoteAddress());
      }
      virtual bool setAllow(const std::string& allow) override{
        m_allow = allow;
        return true;
      }
      virtual uint32 value() override{return 3000;}
    protected:
    private:
       uv_tcp_t* m_tcp;
       std::string m_allow;
       std::string m_address;
       unsigned int m_port;
       unsigned int m_maxConnect;
       /*bool m_ip6;*/
    };

    //////////////////////////////////////////////////////////////////////////

    class TcpClient : public ObjectBase{
    public:
      explicit TcpClient(const NetProperty& property, FunCreateSession fun): ObjectBase(property, fun){
        m_session = nullptr;
      }
      virtual ~TcpClient(){
        m_session.reset();
      }
      virtual bool start() override{
        if(0 == (m_flag & _TAG_ADD)){
          return false;
        }

        if (nullptr == m_session){
          m_session = createSession();
        }
        m_flag &= (~_TAG_ADD);
        auto t = thread();
        MYASSERT(t);
        MYASSERT(m_session);
//         uv_tcp_t*  tcp = podMalloc<uv_tcp_t>();
//         int r = uv_tcp_init(t->loop(), tcp);
//         uvError("uv_tcp_init:", r);
//         setNoPIPEOptional(tcp);
        if(config().m_compress){m_session->setFlag(NetSession::SESSSION_COMPRESS);}
        if(config().m_autoReconnect){ m_session->setFlag(NetSession::SESSION_RECONN);}
        m_session->clientSession(t->loop(),
                                 config().m_address,
                                 config().m_port,
                                 std::bind(&NetThread::recvMsgList, t.get(), std::placeholders::_1, std::placeholders::_2),
                                 std::bind(&NetThread::realSend, t.get()),
                                 std::bind(&NetThread::notifyKick, t.get()),
                                 std::bind(&NetThread::notifySend, t.get(), std::placeholders::_1));
        t->addSession(m_session);
        return true;
      }
      virtual bool stop() override{
        m_session->clearFlag(NetSession::SESSION_RECONN);
        return true;
      }
      virtual bool kick() override{
        m_session->setKicked();
        return true;
      }
      virtual bool allow(SessionPtr /*s*/) override{
        return true;
      }
      virtual bool setAllow(const std::string& /*allow*/) override {return true;}

      virtual uint32 value() override{return 2500;}
    protected:
    private:
      SessionPtr m_session; 
    };

    //////////////////////////////////////////////////////////////////////////


    class HttpServer : public ObjectBase{
      enum{ buffer_len = 256, };
    public:
      explicit HttpServer(const NetProperty& property, FunCreateSession fun): ObjectBase(property, fun){
        m_tcp = podMalloc<uv_tcp_t>();
        m_allow = property.config().m_allow;
        m_address = property.config().m_address;
        m_port = property.config().m_port;
        m_maxConnect = property.config().m_maxConnect;
        /*m_ip6 = Utility::isIPV6(m_address);*/
      }
      virtual ~HttpServer(){
        if(m_tcp){
          podFree(m_tcp);
          m_tcp = nullptr;
        }
      }

      void sessionConnect(){
        uv_tcp_t*  tcp = podMalloc<uv_tcp_t>();
        int r = uv_tcp_init(m_tcp->loop, tcp);
        uvError("uv_tcp_init:", r);
        setNoPIPEOptional(tcp);
        if(0 == uv_accept((uv_stream_t*)m_tcp, (uv_stream_t*)tcp)){
          sockAddress address;
          char buf[buffer_len];
          memset(buf, 0, sizeof(buf));
          int len = sizeof(sockaddr_in6);
          r = uv_tcp_getpeername(tcp, &address.addr, &len);
          if(0 == r){
            r = uv_ip6_name(&address.in6, buf, sizeof(buf));
          } else{
            len = sizeof(sockaddr_in);
            r = uv_tcp_getpeername(tcp, &address.addr, &len);
            r = uv_ip4_name(&address.in, buf, sizeof(buf));
          }
          auto s = createSession();
          auto t = thread();
          s->httpServerSession(tcp,
                               buf,
                               std::bind(&NetThread::recvMsgList, t.get(), std::placeholders::_1, std::placeholders::_2),
                               std::bind(&NetThread::notifySend, t.get(), std::placeholders::_1));
          t->addSession(s);
        } else{
          uv_close((uv_handle_t*)tcp, [](uv_handle_t* t){ podFree((uv_tcp_t*)(t)); });
        }
      }

      virtual bool start() override{
        if(0 == (m_flag & _TAG_ADD)){
          return false;
        }
        m_flag &= (~_TAG_ADD);
        auto t = thread();
        uv_loop_t* loop = t->loop();
        m_tcp = podMalloc<uv_tcp_t>();
        m_tcp->data = this;
        int r = 0;
        r = uv_tcp_init(loop, m_tcp);
        uvError("uv_tcp_init:", r);
        sockAddress addr;
        r = uv_ip6_addr("::0", m_port, &addr.in6);
        uvError("uv_ip6_addr:", r);
        if(0 != r){
          r = uv_ip4_addr("0.0.0.0", m_port, &addr.in);
          uvError("uv_ip4_addr:", r);
        }
        r = uv_tcp_bind(m_tcp, &addr.addr, 0);
        uvError("uv_tcp_bind:", r);

        r = uv_listen((uv_stream_t*)(m_tcp),
                      m_maxConnect,
                      [](uv_stream_t* server, int status){
          if(status == -1){ return; }
          HttpServer* s = static_cast<HttpServer*>(server->data);
          if(s){ s->sessionConnect(); }
        });
        uvError("uv_listen:", r);
        return true;
      }
      virtual bool stop() override{
        uv_close((uv_handle_t*)m_tcp, nullptr);
        return true;
      }
      virtual bool allow(SessionPtr s)  override{
        if(m_allow.empty()){ return true; }
        return Utility::isPassAddress(m_allow, s->remoteAddress());
      }
      virtual bool setAllow(const std::string& allow) override{
        m_allow = allow;
        return true;
      }
      virtual uint32 value() override{ return 3000; }
    protected:
    private:
      uv_tcp_t* m_tcp;
      std::string m_allow;
      std::string m_address;
      unsigned int m_port;
      unsigned int m_maxConnect;
    };

    //////////////////////////////////////////////////////////////////////////

    class HttpClient : public ObjectBase{
    public:
      explicit HttpClient(const NetProperty& property, FunCreateSession fun): ObjectBase(property, fun){
        m_session = nullptr;
        m_message = nullptr;
      }
      virtual ~HttpClient(){
        m_session.reset();
      }

      virtual void request(MessagePtr m) override{
       m_message = m;
      }

      virtual bool start() override{
        if(0 == (m_flag & _TAG_ADD)){
          return false;
        }
        if(nullptr == m_session){ m_session = createSession();}

        m_flag &= (~_TAG_ADD);
        auto t = thread();
        MYASSERT(t);
        MYASSERT(m_session);
//         uv_tcp_t*  tcp = podMalloc<uv_tcp_t>();
//         int r = uv_tcp_init(t->loop(), tcp);
//         uvError("uv_tcp_init:", r);
        m_session->httpClientSession(t->loop(),
                                     config().m_address,
                                     config().m_port,
                                     std::bind(&NetThread::recvMsgList, t.get(), std::placeholders::_1, std::placeholders::_2),
                                     m_message);
        t->addSession(m_session);
        return true;
      }
      virtual bool stop() override{
        m_session->clearFlag(NetSession::SESSION_RECONN);
        return true;
      }
      virtual bool kick() override{
        m_session->setKicked();
        return true;
      }
      virtual bool allow(SessionPtr /*s*/) override{
        return true;
      }
      virtual bool setAllow(const std::string& /*allow*/) override{ return true; }

      virtual uint32 value() override{ return 2500; }
    protected:
    private:
      SessionPtr m_session;
      MessagePtr m_message;
    };

    //////////////////////////////////////////////////////////////////////////

    ObjectBase::ObjectBase(const NetProperty& property, FunCreateSession fun):m_property(property), m_fun(fun){
      m_flag |= _TAG_ADD;

    }
    ObjectBase::~ObjectBase(){

    }

    void ObjectBase::setNoPIPEOptional(uv_tcp_t* t){
      if(t){
#ifndef WIN32
        int optval = 1;
#ifdef __linux__  
        uv_socket_sockopt((uv_handle_t*)t, MSG_NOSIGNAL, &optval);
#else  
        uv_socket_sockopt((uv_handle_t*)t, SO_NOSIGPIPE, &optval);
#endif 
#endif //
     }
    }
    bool ObjectBase::bindThread(ThreadPtr t){
      m_thread = t;
      t->addObject(shared_from_this());
      return true;
    }

    ThreadPtr  ObjectBase::thread(){
      if(!m_thread.expired()){
        return m_thread.lock();
      } else{
        return nullptr;
      }
    }

    SessionPtr ObjectBase::createSession(){
      auto s = m_fun(m_property.config(), m_property.makeBlockFun());
      LOGINFO("[net] create session:", s->id(),
              " with net object:", m_property.config().m_name, 
              " type:", ServiceName[m_property.config().m_serviceType]);
      return s;
    }

    ObjectPtr ObjectBase::create(const NetProperty& property, FunCreateSession fun){
      if (_SERVER_FLAG_ == property.config().m_serviceType){
        return std::make_shared<TcpServer>(property, fun);
      }
      if (_CLIENT_FLAG_ == property.config().m_serviceType){
        return std::make_shared<TcpClient>(property, fun);
      }
      if(_HTTP_SERVER_ == property.config().m_serviceType){
        return std::make_shared<HttpServer>(property, fun);
      }
      if(_HTTP_CLIENT_ == property.config().m_serviceType){
        return std::make_shared<HttpClient>(property, fun);
      }
      MYASSERT(false);
      return nullptr;
    }


  }
}
