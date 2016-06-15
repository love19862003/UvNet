/********************************************************************

  Filename:   NetManager

  Description:NetManager

  Version:  1.0
  Created:  31:3:2015   19:17
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#include "net/NetManager.h"
#include "net/NetSession.h"
#include "net/NetThread.h"
#include "net/NetCore.h"
#include "net/NetUV.h"
#include "net/NetObj.h"
#include "utility/MapTemplate.h"
#include "utility/Compress.h"
#include "utility/StringUtility.h"
#include <algorithm>
#include <set>
#include <thread>
#include <iostream>
#include <atomic>
#include <functional>
namespace ShareSpace {
  namespace NetSpace {

    typedef Utility::ObjPtrMap<SessionId, NetSession>  SessionMap;
    typedef Utility::ObjPtrMap<NetName, ObjectBase> NetObjectMap;


    class NetService{
    private:
      NetService(const NetService&) = delete;
      NetService& operator = (const NetService&) = delete;
    public:
      enum ServiceState{
        _SERVICE_INIT_ = 0,
        _SERVICE_RUN_ = 1,
        _SERVICE_STOP_ = 2,
      };
      enum{
        _DEFAULT_BUFFER_SIZE = 65535,
      };


      explicit NetService(unsigned int c)
        : m_state(_SERVICE_INIT_)
        , m_currentId(INVALID_SESSION_ID)
        , m_workThreadCount(c){
        m_workThreadCount = c > 0 ? c : 1;
        m_sessions.setOptional(INVALID_SESSION_ID);
        m_nets.setOptional("");

        LOGINFO("server ip address info:", sizeof(sockaddr)," v6 " ,sizeof(sockaddr_in6)," v4 " ,sizeof(sockaddr_in));
        char buf[512];
        uv_interface_address_t *info;
        int count = 0;
        uv_interface_addresses(&info, &count);
        for (int i = 0 ; i < count; ++i){
          uv_interface_address_t interface = info[i];
          LOGINFO("Internal:", interface.is_internal ? "Yes" : "No");
          if(interface.address.address4.sin_family == AF_INET){
            uv_ip4_name(&interface.address.address4, buf, sizeof(buf));
            LOGINFO("IPv4 address:", buf);
          } else if(interface.address.address4.sin_family == AF_INET6){
            uv_ip6_name(&interface.address.address6, buf, sizeof(buf));
            LOGINFO("IPv6 address:", buf);
          }
        }
        uv_free_interface_addresses(info, count);
        LOGINFO("...");
      }
      virtual ~NetService(){
        m_threads.clear();
        m_sessions.clear();
        m_nets.clear();
      }
      //创建一个新的会话,并且分配一个NetThread(读写)线程
      SessionPtr addNewSession(const Config& config, const FunMakeBlock& fun){
        ++m_currentId;
        return  SessionPtr(new NetSession(m_currentId, _DEFAULT_BUFFER_SIZE, config, fun));
      }
      //poll消息到主线程
      bool poll(){
        //MYASSERT(m_state == NetServiceData::_SERVICE_RUN_);
        std::list<MessagePtr> msglist;
        std::list<SessionPtr> listDisconnect;
        std::list<SessionPtr> listConnect;
        for(auto& t : m_threads){
          t->pollThread(msglist, listConnect, listDisconnect);
        }
        for(auto& s : listConnect){
          auto obj = m_nets.getData(s->netName());
          MYASSERT(obj);
          if(obj->allow(s)){
            m_sessions.addData(s->id(), s);
            obj->connect(s->id());
          } else{ realKick(s);}
        }

        for(auto& m : msglist){
          auto s = m_sessions.getData(m->session());
          if(s){
            auto obj = m_nets.getData(s->netName());
            MYASSERT(obj);
            obj->call(m);
          }
        }

        for(auto& s : listDisconnect){
          auto obj = m_nets.getData(s->netName());
          MYASSERT(obj);
          if(_CLIENT_FLAG_ == obj->type()){ m_nets.eraseData(s->netName()); }
          obj->close(s->id());
          m_sessions.eraseData(s->id());
        }

        return true;
      }

      void stop(unsigned int ms){
        if(_SERVICE_STOP_ == m_state){ return; }
        m_state = _SERVICE_STOP_;
        for(auto& t : m_threads){ t->stop(); }

        std::chrono::steady_clock::time_point clockBegin;
        std::chrono::steady_clock::time_point clockEnd;
        clockBegin = clockEnd = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration delay;

        do{
          poll();
          clockEnd = std::chrono::steady_clock::now();
          delay = std::chrono::duration_cast<std::chrono::milliseconds>(clockEnd - clockBegin);
        } while(delay.count() <= ms || check());
      }

      bool isRun() const{ return m_state == _SERVICE_RUN_; }

      bool debug() const{ 
        auto fun = [](const SessionMap::pair_type& pair){
          auto s = pair.second;
          if (s){ LOGDEBUG(s->info());}
        };
        for(auto& t : m_threads){
          LOGDEBUG(t->info());
        }
        m_sessions.forEach(fun);
        return true;
      }

      bool setAllow(const NetName& name, const std::string& allow){
        auto obj = m_nets.getData(name);
        if(obj){
          return obj->setAllow(allow);
        }
        return false;
      }
      bool check() const{
        for(auto&t : m_threads){
          if(t->check()){ return true; }
        }
        return false;
      }
      //启动网络服务模块
      bool start(){
        LOGDEBUG("uv:", uv_version_string());
        for(unsigned int i = 0; i < m_workThreadCount; ++i){
          m_threads.push_back(std::make_shared<NetThread>());
        }
        m_nets.forEach([this](const NetObjectMap::pair_type& pair){
          auto p = pair.second;
          auto t = findThread();
          p->bindThread(t);
        });
        m_state = NetService::_SERVICE_RUN_;
        for(auto& t : m_threads){
          t->start();
        }
        return true;
      }
      //删除一个客户端类型的网络对象
      bool removeClient(const std::string& name){
        auto client = m_nets.getData(name);
        if(client){ return client->kick();}
        return false;
      }


      //主线程踢人
      bool kick(SessionId id){
        auto s = m_sessions.getData(id);
        return realKick(s);
      }
      //增加一个网络对象
      bool add(const NetProperty& property){
        if(property.config().m_name.empty()){ return false; }
        if(m_nets.hasData(property.config().m_name)){ return false; }
        if(_SERVICE_STOP_ == m_state){ return false; }

        ObjectBase::FunCreateSession fun = std::bind(&NetService::addNewSession,
                                                     this,
                                                     std::placeholders::_1,
                                                     std::placeholders::_2);
        auto ptr = ObjectBase::create(property, fun);
        m_nets.addData(property.config().m_name, ptr);
        if(isRun()){ ptr->bindThread(findThread()); }
        return true;
      }
      //发送消息
      bool send(const MessagePtr& msg){
        if(nullptr == msg){return false;}
        auto s = m_sessions.getData(msg->session());
        if(s){
          s->pushWrite(msg);
          return true;
        }
        return false;
      }
      //发送消息
      bool send(const NetName& name, const MessagePtr& msg){
        if(nullptr == msg){return false;}
        auto p = m_nets.getData(name);
        if(!p){ return false; }
        msg->lock(p->config().m_compress);
        m_sessions.forEach([&](const SessionMap::pair_type& pair){
          auto s = pair.second;
          if (s && s->netName() == name){
            MessagePtr ptr(msg->clone(s->id()));
            send(ptr);
          }
        });
        return true;
      }
    private:
      bool realKick(SessionPtr s){
        if(s){ return s->setKicked(); }
        return false;
      }
      ThreadPtr findThread(){
        if(isRun()){
          return *std::min_element(m_threads.begin(),
                                   m_threads.end(),
                                   [](const ThreadPtr& l, const ThreadPtr& r){return l->getSpeed() < r->getSpeed(); });
        } else{
           return *std::min_element(m_threads.begin(),
                                    m_threads.end(),
                                    [](const ThreadPtr& l, const ThreadPtr& r){return l->value() < r->value();});
        }
        
      }

    protected:
      std::list<ThreadPtr> m_threads;           //工作线程池
      SessionMap m_sessions;                    //在线链接
      NetObjectMap m_nets;                      //网络对象
      ServiceState m_state;                     //服务状态
      std::atomic<SessionId> m_currentId;       //下一个session id 
      unsigned int m_workThreadCount;           //工作收发线程数量
    };

    NetManager::NetManager(unsigned int t)
    :m_service(new NetService(t))
    { 
      ;
    }
    NetManager::~NetManager() {
      if(m_service) {
        m_service.reset();
      }
    }
    bool NetManager::start() {
      return m_service->start();
    }

    bool NetManager::add(const NetProperty& property) {
      return m_service->add(property);
    }
    bool NetManager::remove(const NetName& name) {
      return m_service->removeClient(name);
    }

    bool NetManager::poll() {
      return m_service->poll();
    }

    bool NetManager::send(const MessagePtr& msg) {
      return m_service->send(msg);
    }
    bool NetManager::send(const NetName& name, const MessagePtr& msg) {
      return m_service->send(name, msg);
    }

    bool NetManager::kick(const SessionId& id) {
      return m_service->kick(id);
    }

    bool NetManager::isRun() const {
      return m_service->isRun();
    }
    void NetManager::stop(unsigned int ms) {
      return m_service->stop(ms);
    }
    bool NetManager::setAllow(const NetName& name, const std::string& allow){
      return m_service->setAllow(name, allow);
    }
    bool NetManager::debug() const{
      return m_service->debug();
    }
  }
}