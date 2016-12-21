/********************************************************************

  Filename:   NetManager

  Description:NetManager

  Version:  1.0
  Created:  31:3:2015   19:14
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#ifndef __NetManager_H__
#define __NetManager_H__
#include "net/NetCore.h"
namespace ShareSpace {
  namespace NetSpace {
    class NetService;
    class NetManager {
    public:
      typedef  std::function<std::string(const std::string& path, const std::string& query)> ResponseCall;
      typedef  std::function<void(uint32, const std::string& result)> RequestCall;
      explicit NetManager(unsigned int t);
      virtual ~NetManager();
      //开始服务
      bool start();

      //关闭网络
      void stop(unsigned int ms);

      //增加网络
      bool add(const NetProperty& property);

      //http request
      uint32 httpRequest(const char* url, uint32 port, uint32 method, const std::string& path, const std::string& query, const RequestCall& Call);

      //http server
      bool httpServer(uint32 port,  const ResponseCall& Call);

      //移除一个网络(只能是client类型)
      bool remove(const NetName& name);

      //获取消息和状态改变
      bool poll();

      //发送消息
      bool send(const MessagePtr& msg);

      //发送消息
      bool send(const NetName& name, const MessagePtr& msg);

      //踢掉会话
      bool kick(const SessionId& id);

      //是否在run
      bool isRun() const;

      //设置网络对象的连接白名单
      bool setAllow(const NetName& name, const std::string& allow);

      //远程IP
      std::string remoteAddress(SessionId id) const;

      //debug 信息
      bool debug() const;
    public:
    protected:
    private:
      NetManager(const NetManager&) = delete;
      NetManager& operator = (const NetManager&) = delete;
      NetManager& operator = (NetManager&&) = delete;
      std::unique_ptr<NetService> m_service;
    };
  }
}


#endif