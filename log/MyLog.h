/********************************************************************

  Filename:   MyLog

  Description:MyLog

  Version:  1.0
  Created:  30:3:2015   12:01
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#ifndef __MyLog_H__
#define __MyLog_H__
#include <string>
#include <assert.h>
namespace ShareSpace {
  namespace MyLog {
    struct Log {

      static bool log_init(const std::string& f);
      static void log_release();
      static void log_error(const std::string& s);
      static void log_debug(const std::string& s);
      static void log_warning(const std::string& s);
      static void log_info(const std::string& s);
      static void log_fatal(const std::string& s);

      static std::string toString(const char* t) {
        return std::string(t);
      }
      static std::string toString(char* t){
        return std::string(t);
      }
      static const std::string& toString(const std::string& t) {
        return t;
      }
      template<typename T>
      static std::string toString( T t) {
        return std::to_string(t);
      }

      template<typename T>
      static std::string makeString(T t) {
        return toString(t);
      }

      template<typename T, typename ... Args>
      static std::string makeString(T t, Args ... args) {
        return toString(t) + makeString(args...);
      }
    };
#define LOGINIT( file)  ShareSpace::MyLog::Log::log_init(file)
#define LOGRELEASE()      ShareSpace::MyLog::Log::log_release()

#define FILE_FUNTION_LINE (" File:" +  std::string(__FILE__) + " Fun:" + std::string(__FUNCTION__) + " Line:" + std::to_string(__LINE__) + " ")

//#define LOGFILEFUN
#ifdef LOGFILEFUN
#define FILESTR     FILE_FUNTION_LINE
#else
#define FILESTR ("")
#endif

#ifndef NDEBUG
#define MYASSERT(condition, ...) do {  \
      if( !(condition) ){\
        ShareSpace::MyLog::Log::log_error( ShareSpace::MyLog::Log::makeString(FILE_FUNTION_LINE,  ##__VA_ARGS__));\
        assert(false);\
         }\
       } while(0)  
#else
#define MYASSERT(condition, ...) do {  \
      if( !(condition) ){\
        ShareSpace::MyLog::Log::log_error(ShareSpace::MyLog::Log::makeString(FILE_FUNTION_LINE,  ##__VA_ARGS__));\
        }\
       } while(0)  
#endif // _DEBUG



#define LOGDEBUG(s,...)  do { std::string _str = FILESTR + ShareSpace::MyLog::Log::makeString(s,  ##__VA_ARGS__);\
                                   ShareSpace::MyLog::Log::log_debug(std::move(_str)); } while(0)

#define LOGERROR(s,...)  do { std::string _str = FILESTR + ShareSpace::MyLog::Log::makeString(s, ##__VA_ARGS__);\
                                   ShareSpace::MyLog::Log::log_error(std::move(_str)); } while(0)

#define LOGWARNING(s,...)  do { std::string _str = FILESTR + ShareSpace::MyLog::Log::makeString(s, ##__VA_ARGS__);\
                                     ShareSpace::MyLog::Log::log_warning(std::move(_str)); } while(0)

#define LOGINFO(s,...)  do { std::string _str = FILESTR + ShareSpace::MyLog::Log::makeString(s, ##__VA_ARGS__);\
                                  ShareSpace::MyLog::Log::log_info(std::move(_str)); } while(0)

#define LOGFATAL(s,...)  do { std::string _str = FILESTR + ShareSpace::MyLog::Log::makeString(s, ##__VA_ARGS__);\
                                   ShareSpace::MyLog::Log::log_fatal(std::move(_str)); } while(0)

  }
}
#endif