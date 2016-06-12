/********************************************************************
	Filename: 	NetRedis.cpp
	Description:
	Version:  1.0
	Created:  31:3:2016   19:59
	
	Compiler: gcc vc
	Author:   wufan, love19862003@163.com
	Organization: lezhuogame
*********************************************************************/
#include "net/NetRedis.h"
#include "net/redisparser.h"
namespace ShareSpace{
  namespace NetSpace{

    RedisBlock::RedisBlock(SessionId s) :BlockBase(s), m_parse(new RedisParseSpace::RedisParse()) {
 
    }
    RedisBlock::RedisBlock(SessionId s, const std::vector<std::string>& args)
      : BlockBase(s)
      , m_cmd(RedisParseSpace::makeCommand(args))
      , m_read(0)
    {
    }
    RedisBlock::RedisBlock(SessionId s, const std::string& cmd):BlockBase(s), m_cmd(cmd), m_read(0){

    }
    RedisBlock::~RedisBlock(){
      m_parse.reset();
    }
    bool RedisBlock::done() const  { 
      return m_parse ? m_parse->isDone() : readComplete();
    }
    bool RedisBlock::recv(BufferPointer& buf){
      if(!m_parse) { return false; }
      size_t pos = 0;
      bool r = m_parse->parse(buf->readData(), buf->needReadLength(), pos);
      buf->readData(pos);
      m_read += pos;
      return r;
    }
    BlockBase* RedisBlock::clone(SessionId id) {
      RedisBlock* p = new RedisBlock(id, m_cmd);
      return p;
    }
    void RedisBlock::readBuffer(NetBuffer& buffer){
      size_t len = std::min(m_cmd.length() - m_read, buffer.maxLength() - buffer.length());
      if (len > 0){
        buffer.tailData(m_cmd.c_str() + m_read, len);
        m_read += len;
      }
    }
    size_t RedisBlock::length() {
       if (m_parse){
         return m_read;
       }else{
         return m_cmd.length();
       }
    }
    bool RedisBlock::readComplete() const{
      if(m_parse) { return false; }
      return m_read + 1 >= m_cmd.length();
    }
    const std::vector<RedisParseSpace::RedisData>& RedisBlock::result() const {
      return m_parse->result();
    }
    RedisParseSpace::reply_t RedisBlock::type() const {
      return m_parse ? m_parse->type() : RedisParseSpace::no_reply;
    }


  }
}
