/********************************************************************

  Filename:   NetCore

  Description:NetCore

  Version:  1.0
  Created:  31:3:2015   14:08
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#ifndef __NetCore_H__
#define __NetCore_H__
#include <functional>
#include <memory>
#include <list>
#include <map>
#include <assert.h>
#include "net/NetMessage.h"
namespace ShareSpace {
  namespace NetSpace {
 
    template< typename PODTYPE>
    PODTYPE* podMalloc() {
      return static_cast<PODTYPE*>(malloc(sizeof(PODTYPE))); 
    }
    template< typename PODTYPE>
    void podFree(PODTYPE* t) {
      free(t);
    }
   

    enum ServiceFlag{
      _SERVER_FLAG_,
      _CLIENT_FLAG_,
      _HTTP_SERVER_,
      _HTTP_CLIENT_,
    };

    const static std::string ServiceName[_HTTP_CLIENT_ + 1] = { "tcp_server", "tcp_client", "http_server", "http_client"};
    

    typedef std::string NetName;
    typedef std::shared_ptr<BlockBase> MessagePtr;
    typedef std::function<void(const NetName&, const MessagePtr&)> FunCall;
    typedef std::function<void(const NetName&, const SessionId&)>  FunSession;
    typedef std::function<MessagePtr(const SessionId&)> FunMakeBlock;

    // net config
    struct Config{
      Config(){}
      std::string  m_name = "";
      std::string  m_address = "";
      int m_port = 0;
      int m_timeOut = 1000;
      int m_maxConnect = 1;
      ServiceFlag  m_serviceType = _SERVER_FLAG_;
      bool m_autoReconnect = true;
      bool m_compress = false;
      std::string m_allow = "";
      bool m_clearOnClose = false;
    };

    class NetProperty{
    public:
      explicit NetProperty(const Config& c, FunCall mc, FunSession fct, FunSession fcl, FunMakeBlock fb )
        : config_(c)
        , messageFun_(mc)
        , connectFun_(fct)
        , closeFun_(fcl)
        , makeBlockFun_(fb){
        
      }
      const Config& config() const{ return config_; }
      FunSession connectFun() const{ return connectFun_; }
      FunSession closeFun() const{ return closeFun_; }
      FunMakeBlock makeBlockFun() const{ return makeBlockFun_; }
      FunCall callFun()const{ return messageFun_; }
    protected:
    private:
      Config config_;
      FunCall messageFun_;
      FunSession connectFun_;
      FunSession closeFun_;
      FunMakeBlock makeBlockFun_;
    };
   
  }
}
#endif