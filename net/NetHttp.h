/********************************************************************
    Filename: NetHttp.h
    Description:
    Version:  1.0
    Created:  20:12:2016   16:00

    Compiler: gcc vc
    Author:   wufan, love19862003@163.com
    Organization: lezhuogame
*********************************************************************/
#ifndef __NetHttp_H__
#define __NetHttp_H__
#include "net/NetMessage.h"
struct http_parser;
namespace ShareSpace{
  namespace NetSpace{
     class HttpBlock : public BlockBase{
     public:
       enum Action : unsigned int{
         AC_DELETE = 0,
         AC_GET = 1,
         AC_HEAD = 2,
         AC_POST = 3,
         AC_PUT = 4,
         AC_MAX
       };

       static void initHttpParserSetting();
       static bool checkAction(uint32 ac){return ac < AC_MAX;}
       typedef std::unique_ptr<http_parser>  HttpParserPtr;
       explicit HttpBlock(SessionId s, bool server);
       explicit HttpBlock(SessionId s, uint32 ac, const std::string& host, const std::string& path, const std::string& cmd);
       explicit HttpBlock(SessionId s, uint32 code, const std::string& response);
       virtual ~HttpBlock();
       virtual bool done() const override;
       virtual bool recv(BufferPointer& buf) override;
       virtual BlockBase* clone(SessionId s) override;
       virtual void lock(bool, uint32)override{;}
       virtual bool readBuffer(NetBuffer& buffer, bool force)override;
       virtual size_t length() override;
       virtual bool readComplete() const override;
       const std::string& content() const;
       const std::string& url() const;
     protected:
     private:
       HttpParserPtr m_parser;
       std::string m_request;
       std::string m_url;
       bool m_done = false;
     };
  }
}
#endif // __NetHttp_H__