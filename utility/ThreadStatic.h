/********************************************************************
    Filename: ThreadStatic.h
    Description:
    Version:  1.0
    Created:  6:4:2016   21:10
	
    Compiler: gcc vc
    Author:   wufan, love19862003@163.com
    Organization: lezhuogame
*********************************************************************/
#ifndef __ThreadStatic_H__
#define __ThreadStatic_H__
#include <memory>
#include <thread>
#include <map>
#include <mutex>

namespace ShareSpace { 
  namespace Utility {
      template<typename T, typename ... Args>
      inline T* getThreadLocal(Args ... args){
#if  (defined(_MSC_VER) && _MSC_VER >1800) || (defined(__GNUC__) &&  __GNUC__ >= 5)
        static thread_local std::unique_ptr<T> t ;
        if(nullptr == t){
          static std::mutex _mutex;
          std::lock_guard<std::mutex> lock(_mutex);
          t = std::move(std::unique_ptr<T>(new T(std::forward<Args>(args)...)) );// std::make_unique<T>();
        }
        return t.get();
#else 
        static std::mutex _mutex;
        static std::map<std::thread::id, std::unique_ptr<T> > _map;
        auto tid = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(_mutex);
        if(nullptr == _map[tid]) { _map[tid] = std::move((std::unique_ptr<T>(new T(std::forward<Args>(args)...)) );  }      //std::make_unique<T>(std::forward<Args>(args)...);
        return _map[tid].get();
#endif // _MSC_VER > 1700
      }
  } 
}
#endif // __ThreadStatic_H__