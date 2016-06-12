/********************************************************************

  Filename:   StringUtility

  Description:StringUtility

  Version:  1.0
  Created:  8:4:2015   11:16
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#ifndef __StringUtility_H__
#define __StringUtility_H__
#include <string>
#include <vector>
#include "utility/NativeType.h"
namespace ShareSpace {
namespace Utility {
   std::string toLowerCopy(const std::string&);
   std::string toUpperCopy(const std::string&);
   std::string& toLower(std::string&);
   std::string& toUpper(std::string&);

   std::wstring utf8ToWstr(const std::string& str);
   std::string wstrToUtf8(const std::wstring& wstr);

   bool isPassAddress(const std::string& str, const std::string& ip);
   bool isIPV4(const std::string& address);
   bool isIPV6(const std::string& address);
   bool isPhone(const std::string& address);
   bool isMail(const std::string& address);
   std::vector<std::string> split(const std::string& str, const std::string& c);
   std::string join(const std::vector<std::string>& v, const std::string& c);
   unsigned int makeCRC32(const char*, size_t);
   std::string makeMd5(const std::string& str);
   inline std::string luaMd5(const std::string str) { return  std::move(makeMd5(str)); }
   std::string makeUuid();

   inline uint64 stou64(const std::string& str, uint64 def = 0){
     try{
       return  std::stoull(str);
     } catch(...){
       return def;
     }
   }
   inline int64 stoi64(const std::string& str, int64 def = 0){
     try{
       return  std::stoll(str);
     } catch(...){
       return def;
     }
   }

   inline int32 stoi32(const std::string& str, int32 def = 0){
     try{
       return  std::stoi(str);
     } catch(...){
       return def;
     }
   }

   inline uint32 stou32(const std::string& str, uint32 def = 0){
     try{
       return  std::stoul(str);
     } catch(...){
       return def;
     }
   }
   inline float stof(const std::string& str, float def = 0.f){
     try{
       return std::stof(str);
     } catch(...){
       return def;
     }
   }

   inline double stod(const std::string& str, double def = 0.0){
     try{
       return std::stod(str);
     } catch(...){
       return def;
     }
   }



}
}


#endif  /*__StringUtility_H__*/
