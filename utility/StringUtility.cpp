/********************************************************************

  Filename:   StringUtility

  Description:StringUtility

  Version:  1.0
  Created:  9:4:2015   10:06
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#include <algorithm>
#include <locale>
#include <codecvt>
#include <regex>
#include "utility/StringUtility.h"

#define MAILREG   ("^([a-z]|[A-Z]|[0-9])(\\w+)*@\\w+[.]\\w+$")
#define PHONEREG  ("^1([3-9])\\d{9}$")
#define IPV4SUB   "((((25[0-5])|(1([0-9]){2})|(2[0-4][0-9])|(([0-9]){1,2}))).){3}(((25[0-5])|(1([0-9]){2})|(2[0-4][0-9])|(([0-9]){1,2})))"
#define IPV6REG8  "(([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})"
#define IPV6REG7  "(([0-9A-Fa-f]{1,4}:){1,7}:)"
#define IPV6REG6  "(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})"
#define IPV6REG5  "(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:){0,1}[0-9A-Fa-f]{1,4})"
#define IPV6REG4  "(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})"
#define IPV6REG3  "(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})"
#define IPV6REG2  "(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})"
#define IPV6REG1  "(([0-9A-Fa-f]{1,4}:){1}:([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})"
#define IPV6REG0  "(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})"
#define IPV6V40   "(([0-9A-Fa-f]{1,4}:){6}" IPV4SUB ")" 
#define IPV6V41   "(([0-9A-Fa-f]{1,4}:){0,5}:" IPV4SUB ")"
#define IPV6V42   "(::([0-9A-Fa-f]{1,4}:){0,5}" IPV4SUB ")"
#define IPV4REG   ("^(" IPV4SUB ")$")
#define IPV6REG   ("^(" IPV6REG0 "|" IPV6REG1 "|" IPV6REG2 "|" IPV6REG3 "|" IPV6REG4 "|" IPV6REG5 "|" IPV6REG6 "|" IPV6REG7 "|" IPV6REG8 "|" IPV6V40 "|" IPV6V41 "|" IPV6V42 ")$") 
bool ShareSpace::Utility::isIPV4(const std::string& address){
  static const std::regex reg(IPV4REG);
  return std::regex_match(address.c_str(), reg);
}
bool ShareSpace::Utility::isIPV6(const std::string& address){
  static const std::string str(IPV6REG);
  std::regex reg(str);
  if(std::regex_match(address.c_str(), reg)){return true;}
  return false;
}
bool ShareSpace::Utility::isMail(const std::string& mail){
  static const std::regex reg(MAILREG);
  return std::regex_match(mail.c_str(), reg);
}
bool ShareSpace::Utility::isPhone(const std::string& ph){
  static const std::regex reg(PHONEREG);
  return std::regex_match(ph.c_str(), reg);
}


bool ShareSpace::Utility::isPassAddress(const std::string& str, const std::string& ip){
  return true;
}

#undef MAILREG
#undef PHONEREG
#undef IPV4SUB  
#undef IPV6REG8 
#undef IPV6REG7 
#undef IPV6REG6 
#undef IPV6REG5 
#undef IPV6REG4 
#undef IPV6REG3 
#undef IPV6REG2 
#undef IPV6REG1 
#undef IPV6REG0 
#undef IPV6V40  
#undef IPV6V41  
#undef IPV6V42  
#undef IPV4REG  
#undef IPV6REG  


std::wstring ShareSpace::Utility::utf8ToWstr(const std::string& str)
{
  typedef std::codecvt_utf8<wchar_t> convert_typeX;
  std::wstring_convert<convert_typeX, wchar_t> converterX;
  return std::move(converterX.from_bytes(str));
}
std::string ShareSpace::Utility::wstrToUtf8(const std::wstring& wstr)
{
  typedef std::codecvt_utf8<wchar_t> convert_typeX;
  std::wstring_convert<convert_typeX, wchar_t> converterX;
  return std::move(converterX.to_bytes(wstr));
}
