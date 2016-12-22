#include <atomic>
#include <signal.h>
#include "log/MyLog.h"
#include "net/Version.h"
#include "net/NetHttp.h"
#include "utility/StringUtility.h"
#include "net/NetManager.h"
#include "demo/demo.h"
#include <memory>
using namespace ShareSpace::NetSpace;
namespace Demo{

  

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

  void command(const std::string& cmd){
    httpRequestGet(gContext->_net, cmd);
    httpRequestPost(gContext->_net, cmd);
  }

  void createDemo(NetManager& net){

    if(!gContext){
      gContext = std::make_shared<Context>(net);
    }

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