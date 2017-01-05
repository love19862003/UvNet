/********************************************************************

  Filename:   NetUV

  Description:NetUV

  Version:  1.0
  Created:  31:3:2015   19:32
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#ifndef __NetUV_H__
#define __NetUV_H__
#include <functional>
#include <memory>
#ifdef WIN32
#include "net/libuv/include/uv.h"
#else
#include "uv.h"
#endif // WIN32

#include "log/MyLog.h"
#ifdef _DEBUG
#define uvError(info, e) if(e != 0){ LOGINFO("[net] ", info, uv_strerror(e)); MYASSERT(false);}
//#define uvError(info, e) if(e != 0){ LOGINFO("[net] ",info, uv_strerror(e), FILE_FUNTION_LINE);}
#else
#define uvError(info, e) if(e != 0){ LOGINFO("[net] ",info, uv_strerror(e), FILE_FUNTION_LINE);}
#endif // _DEBUG

namespace ShareSpace {
  namespace NetSpace {
    class NetSession;
    class NetThread;
    class ObjectBase;
    typedef std::shared_ptr<NetSession> SessionPtr;
    typedef std::shared_ptr<NetThread> ThreadPtr;
    typedef std::weak_ptr<NetThread> WeakThreadPtr;
    typedef std::shared_ptr<ObjectBase> ObjectPtr;
    union sockAddress{
      sockaddr_in6 in6;
      sockaddr_in in;
      sockaddr addr;
    };
  }
}
#endif