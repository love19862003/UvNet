/********************************************************************
    Filename: NetObj.h
    Description:
    Version:  1.0
    Created:  13:4:2016   12:07
	
    Compiler: gcc vc
    Author:   wufan, love19862003@163.com
    Organization: lezhuogame
*********************************************************************/
#ifndef __NetObj_H__
#define __NetObj_H__
#include "net/NetUV.h"
#include "net/NetCore.h"
namespace ShareSpace{
  namespace NetSpace{
    class ObjectBase : public std::enable_shared_from_this<ObjectBase>{
    public:
      typedef std::function<SessionPtr(const Config&, const FunMakeBlock& )> FunCreateSession;
      ObjectBase(const ObjectBase&) = delete;
      ObjectBase& operator = (const ObjectBase&) = delete;
      explicit ObjectBase(const NetProperty& property, FunCreateSession fun);
      virtual ~ObjectBase();

      static ObjectPtr create(const NetProperty& property, FunCreateSession fun);
      bool bindThread(ThreadPtr t);
      ThreadPtr  thread();
      SessionPtr createSession();
      virtual bool start() = 0;
      virtual bool stop() = 0;
      virtual bool kick(){return false;}
      virtual bool allow(SessionPtr s)  = 0;
      virtual uint32 value() = 0; 
      virtual void request(MessagePtr ){}
      enum{
        _TAG_ADD = 0x0001,
        _TAG_ALLOW = 0x0002,
      };
      virtual bool setAllow(const std::string& allow) = 0;

      ServiceFlag  type() const {return m_property.config().m_serviceType; }
      

      void connect(SessionId id){ 
        auto fun = m_property.connectFun(); 
        if(fun){ fun(m_property.config().m_name, id); }
      }
      void close(SessionId id){ 
        auto fun = m_property.closeFun(); 
        if(fun) {fun(m_property.config().m_name, id);}
      }
      void call(MessagePtr m){ 
        auto fun = m_property.callFun();
        if(fun){fun(m_property.config().m_name,m);}
      }
      const Config& config() const{return m_property.config();}
    protected:
      void setNoPIPEOptional(uv_tcp_t* t);
      unsigned int m_flag;           //标识符
    private:
      NetProperty m_property;        //配置信息
      FunCreateSession m_fun;        //创建函数
      WeakThreadPtr m_thread;        //所属线程
      
    };
  }
}
#endif // __NetObj_H__
