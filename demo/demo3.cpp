#include <atomic>
#include <signal.h>
#include "log/MyLog.h"
#include "net/Version.h"
#include "net/NetHttp.h"
#include "utility/StringUtility.h"
#include "net/NetManager.h"
#include "net/NetRedis.h"
#include "demo/demo.h"
#include <memory>
using namespace ShareSpace::NetSpace;
namespace Demo{


  void onClose(const NetName&net, const SessionId& s){
    LOGDEBUG("net:", net, " close session:", s);
  }

  MessagePtr makeRedisBlock(const SessionId& id){
    return std::make_shared<RedisBlock>(id);
  }

  void handleRedis(const NetName& net, const MessagePtr& ptr){
    if(!ptr) { LOGWARNING("handle a null message from net:", net); return; }
    if(ptr->error()) { MYASSERT(false, "handle msg crc error"); gContext->_net.kick(ptr->session()); return; }

    auto p = std::static_pointer_cast<RedisBlock>(ptr);
    auto result = p->result();
    auto type = p->type();
    const RedisParseSpace::RedisData& r = result[0];
    LOGDEBUG("redis result:");
    switch(type){
    case RedisParseSpace::status_code_reply:
    case RedisParseSpace::error_reply:
      {
        LOGDEBUG(boost::get<std::string>(r));
      }break;
    case RedisParseSpace::int_reply:
      {
        LOGDEBUG(boost::get<int>(r));
      }break;
    case RedisParseSpace::bulk_reply:
      {
        const std::string* str = boost::get<std::string>(&r);
        if(str != nullptr){
         LOGDEBUG(*str);
        } else{
         LOGDEBUG("nil");
        }
      }break;
    case RedisParseSpace::multi_bulk_reply:
      {  
        
        for(auto& v : result){
          const std::string* str = boost::get<std::string>(&v);
          if(str != nullptr){
            LOGDEBUG(*str);
          } else{
            LOGDEBUG("nil");
          }
        }
      }break;
    default:
      break;
    }

    LOGDEBUG("\n");
  }

  void onConnectRedis(const NetName&net, const SessionId& s){
    LOGDEBUG( "connect redis success. you can use command");
  }


  static  std::vector< std::string > split(const std::string& s, const std::string& delim){
    std::vector< std::string > ret ;
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while(index != std::string::npos){
      ret.push_back(s.substr(last, index - last));
      last = index + 1;
      index = s.find_first_of(delim, last);
    }
    if(index - last > 0){
      ret.push_back(s.substr(last, index - last));
    }
    return std::move(ret);
  }


  void command(const std::string& cmd){
     MessagePtr ptr (new RedisBlock(0, split(cmd, " ")));
     // send to c.m_name
     gContext->_net.send("redis Demo", ptr);
  }
  bool addRedisDemo(NetManager& net, const Config& config){
    MYASSERT(config.m_serviceType == _CLIENT_FLAG_);
    NetProperty property(config, handleRedis, onConnectRedis, onClose, makeRedisBlock);
    return net.add(std::move(property));
  }

  bool addHttpServer(NetManager& net){
    return net.httpServer(80, [](const std::string& path, const std::string& cmd)->std::string{
      return "{\"" + path + "\":\"" + cmd + "\"}"; 
    });
  }

  bool httpRequestGet(NetManager& net, const std::string& cmd){
    uint32 id =  net.httpRequest("localhost", 80,  HttpBlock::AC_GET, "GetTest", cmd, [](uint32 id, const std::string& result){
       LOGDEBUG("http request:", id, " value:", result);
    });
    return id > 0;
  }

  bool httpRequestPost(NetManager& net, const std::string& cmd){
    uint32 id = net.httpRequest("localhost", 80, HttpBlock::AC_POST, "PostTest", cmd, [](uint32 id, const std::string& result){
      LOGDEBUG("http request:", id, " value:", result);
    });
    return id > 0;
  }
  void createDemo(NetManager& net){

    if(!gContext){
      gContext = std::make_shared<Context>(net);
    }

    bool result = true;
    Config c;
    c.m_address = "localhost";
    c.m_name = "redis Demo";
    c.m_port = 6379;
    c.m_serviceType = _CLIENT_FLAG_;
    c.m_compress = false;
   // result = addRedisDemo(net, c);
   // MYASSERT(result);

    addHttpServer(net);
    httpRequestGet(net, "hello&&get");
    httpRequestGet(net, "hello&&get1");
    httpRequestGet(net, "hello&&get2");
    httpRequestPost(net, "hello post");
    httpRequestPost(net, "hello post1");
    httpRequestPost(net, "hello post2");
    httpRequestPost(net, "hello post3");

  }
}