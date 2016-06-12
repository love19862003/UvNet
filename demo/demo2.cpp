#include <atomic>
#include <signal.h>
#include "log/MyLog.h"
#include "net/Version.h"
#include "utility/StringUtility.h"
#include "net/NetManager.h"
#include "demo/demo.h"
#include <memory>
using namespace ShareSpace::NetSpace;
namespace Demo{

  MessagePtr pack(SessionId s, int count, uint16 mask = 0){
    size_t size = sizeof(NetBlock::head) + sizeof(int);
    std::unique_ptr<NetBuffer> b(new NetBuffer(size));
    b->writeData(sizeof(NetBlock::head));
    b->tailPod(count);
    return MessagePtr(new NetBlock(s, std::move(b), mask));
  }
  int unpack(const MessagePtr ptr){
    auto p = std::static_pointer_cast<NetBlock>(ptr);
    auto& b = p->data();
    int count = -1;
    b->readPod(count);
    return count;
  }


  void handleMessage(const NetName& net, const MessagePtr& ptr){
    int count = unpack(ptr);
    LOGDEBUG("net:", net, " handle message  session:", ptr->session(), " count:", count);
    gContext->_net.send(pack(ptr->session(), ++count));
  }

  void onConnect(const NetName&net, const SessionId& s){
    LOGDEBUG("net:", net, " add session:", s);
    if("demo c1" == net || "demo c2" == net || "demo c3" == net){
      auto ptr = pack(s, 0, 0);
      gContext->_net.send(ptr);
    }
  }

  void onClose(const NetName&net, const SessionId& s){
    LOGDEBUG("net:", net, " close session:", s);
  }

  MessagePtr makeBlock(const SessionId& id){
    return std::make_shared<NetBlock>(id);
  }
  bool addDemoTest(NetManager& net, const Config& config){
    NetProperty  property(config, handleMessage, onConnect, onClose, makeBlock);
    return net.add(std::move(property));
  }

  void command(const std::string& cmd){

  }
  void createDemo(NetManager& net){

    if(!gContext){
      gContext = std::make_shared<Context>(net);
    }

    bool result = true;
    Config s;
    s.m_address = "localhost";
    s.m_name = "demo server";
    s.m_port = 10000;
    s.m_maxConnect = 1000;
    s.m_serviceType = _SERVER_FLAG_;
    s.m_compress = true;
    result = addDemoTest(net, s);
    MYASSERT(result);

    Config c1;
    c1.m_address = "localhost";
    c1.m_name = "demo c1";
    c1.m_port = 10000;
    c1.m_serviceType = _CLIENT_FLAG_;
    c1.m_compress = true;
    result = addDemoTest(net, c1);
    MYASSERT(result);

    Config c2;
    c2.m_address = "localhost";
    c2.m_name = "demo c2";
    c2.m_port = 10000;
    c2.m_serviceType = _CLIENT_FLAG_;
    c2.m_compress = true;
    result = addDemoTest(net, c2);
    MYASSERT(result);

    Config c3;
    c3.m_address = "localhost";
    c3.m_name = "demo c3";
    c3.m_port = 10000;
    c3.m_serviceType = _CLIENT_FLAG_;
    c3.m_compress = true;
    result = addDemoTest(net, c3);
    MYASSERT(result);
  }
}