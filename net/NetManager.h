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
      //��ʼ����
      bool start();

      //�ر�����
      void stop(unsigned int ms);

      //��������
      bool add(const NetProperty& property);

      //http request
      uint32 httpRequest(const char* url, uint32 port, uint32 method, const std::string& path, const std::string& query, const RequestCall& Call);

      //http server
      bool httpServer(uint32 port,  const ResponseCall& Call);

      //�Ƴ�һ������(ֻ����client����)
      bool remove(const NetName& name);

      //��ȡ��Ϣ��״̬�ı�
      bool poll();

      //������Ϣ
      bool send(const MessagePtr& msg);

      //������Ϣ
      bool send(const NetName& name, const MessagePtr& msg);

      //�ߵ��Ự
      bool kick(const SessionId& id);

      //�Ƿ���run
      bool isRun() const;

      //���������������Ӱ�����
      bool setAllow(const NetName& name, const std::string& allow);

      //Զ��IP
      std::string remoteAddress(SessionId id) const;

      //debug ��Ϣ
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