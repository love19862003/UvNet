/********************************************************************
  Filename:   NetRedis.h
  Description:
  Version:  1.0
  Created:  31:3:2016   19:56

  Compiler: gcc vc
  Author:   wufan, love19862003@163.com
  Organization: lezhuogame
*********************************************************************/
#ifndef __NetRedis_H__
#define __NetRedis_H__
#include <iostream>
#include "net/NetMessage.h"
#include <vector>
#include "net/redisparser.h"
namespace ShareSpace{
  namespace NetSpace {
   class RedisBlock : public BlockBase{
   public:
     typedef std::unique_ptr<RedisParseSpace::RedisParse> RedisParsePointer;
     explicit RedisBlock(SessionId s);
     explicit RedisBlock(SessionId s, const std::vector<std::string>& args);
     explicit RedisBlock(SessionId s, const std::string& cmd);
     virtual ~RedisBlock();
     virtual bool done() const override;
     virtual bool recv(BufferPointer& buf) override;
     virtual BlockBase* clone(SessionId s) override;
     virtual void lock(bool /*compress*/) override { ; }
     virtual bool readBuffer(NetBuffer& buffer, bool force) override;
     virtual size_t length() override;
     virtual bool readComplete() const override ;
     const std::vector<RedisParseSpace::RedisData>& result() const;
     RedisParseSpace::reply_t type() const;
   private:
     RedisParsePointer m_parse;
     bool m_done;
     const std::string m_cmd;
     size_t m_read;
   };
  }
}

#endif
