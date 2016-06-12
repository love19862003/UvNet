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
  std::wstring utf8ToWstr(const std::string& str);
  std::string wstrToUtf8(const std::wstring& wstr);

   bool isPassAddress(const std::string& str, const std::string& ip);
   bool isIPV4(const std::string& address);
   bool isIPV6(const std::string& address);
   bool isPhone(const std::string& address);
   bool isMail(const std::string& address);
}
}


#endif  /*__StringUtility_H__*/
