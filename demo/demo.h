/********************************************************************
    Filename: demo.h
    Description:
    Version:  1.0
    Created:  12:6:2016   15:56
	
    Compiler: gcc vc
    Author:   wufan, love19862003@163.com
    Organization: lezhuogame
*********************************************************************/
#ifndef __demo_H__
#define __demo_H__
namespace ShareSpace{
  namespace NetSpace{
    class NetManager;
  }
}

namespace Demo{
  
  struct Context{
    explicit Context(ShareSpace::NetSpace::NetManager& net): _net(net){}
    ShareSpace::NetSpace::NetManager& _net;
  };

  static std::shared_ptr<Context> gContext = nullptr;

  void createDemo(ShareSpace::NetSpace::NetManager& net);
  void command(const std::string& cmd);
}

#endif // __demo_H__