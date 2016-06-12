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
#if  (defined(_MSC_VER) && _MSC_VER >1800) || (defined(__GNUC__) &&  __GNUC__ >= 5)
#define UTF8CXXOX
#else
#undef UTF8CXXOX
#endif
#ifdef UTF8CXXOX
#include <codecvt>
#endif
#include <regex>
#include "utility/StringUtility.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include "utility/md5.h"

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
std::string ShareSpace::Utility::makeMd5(const std::string& str) {
  return std::move(Utility::toLowerCopy(MD5(str).hexdigest()));
}
unsigned int ShareSpace::Utility::makeCRC32(const char* buffer, size_t len) {
  boost::crc_32_type result;
  result.process_block(buffer, buffer + len);
  return result.checksum();
}
bool ShareSpace::Utility::isPassAddress(const std::string& str, const std::string& ip) {
  if (isIPV4(ip)){
    std::string reg = str;
    reg = "^" + str + "$";
    boost::algorithm::ireplace_all(reg, "*", "(((25[0-5])|(1([0-9]){2})|(2[0-4][0-9])|(([0-9]){1,2})))");
    return std::regex_match(ip.c_str(), std::regex(reg));
  }
  if (isIPV6(ip)){
    std::string reg = str;
    reg = "^" + str + "$";
    boost::algorithm::ireplace_all(reg, "*", "([0-9A-Fa-f]{1,4})");
    return std::regex_match(ip.c_str(), std::regex(reg)); 
  }
  return false;
}
std::string ShareSpace::Utility::join(const std::vector<std::string>& v, const std::string& c){
  return std::move(boost::algorithm::join(v, c));
}
std::vector<std::string> ShareSpace::Utility::split(const std::string& str, const std::string& c){
  std::vector<std::string> r;
  boost::algorithm::split(r, str, boost::is_any_of(c), boost::algorithm::token_compress_on);
  return std::move(r);
}

std::string ShareSpace::Utility::toLowerCopy(const std::string& str) {
  return std::move(boost::algorithm::to_lower_copy(str));
}

std::string& ShareSpace::Utility::toLower(std::string& str) {
  boost::algorithm::to_lower(str);
  return str;
}

std::string ShareSpace::Utility::toUpperCopy(const std::string& str){
  return std::move(boost::algorithm::to_upper_copy(str));
}
std::string& ShareSpace::Utility::toUpper(std::string& str){
  boost::algorithm::to_upper(str);
  return str;
}

std::string  ShareSpace::Utility::makeUuid(){
  static boost::uuids::random_generator gen;
  boost::uuids::uuid id = gen();
  std::string r ;
  std::for_each(id.begin(), id.end(), [&](uint8 c){r.push_back(c);});
  return std::move(r);
}

void utf8toWStrReal(std::wstring& dest, const std::string& src){
  dest.clear();
  wchar_t w = 0;
  int bytes = 0;
  wchar_t err = L' ';
  for (size_t i = 0; i < src.size(); i++){
    unsigned char c = (unsigned char)src[i];
    if (c <= 0x7f){//first byte
      if (bytes){
        dest.push_back(err);
        bytes = 0;
      }
      dest.push_back((wchar_t)c);
    }
    else if (c <= 0xbf){//second/third/etc byte
      if (bytes){
        w = ((w << 6) | (c & 0x3f));
        bytes--;
        if (bytes == 0)
          dest.push_back(w);
      }
      else
        dest.push_back(err);
    }
    else if (c <= 0xdf){//2byte sequence start
      bytes = 1;
      w = c & 0x1f;
    }
    else if (c <= 0xef){//3byte sequence start
      bytes = 2;
      w = c & 0x0f;
    }
    else if (c <= 0xf7){//3byte sequence start
      bytes = 3;
      w = c & 0x07;
    }
    else{
      dest.push_back(err);
      bytes = 0;
    }
  }
  if (bytes)
    dest.push_back(err);
}
void wstrToUtf8Real(std::string& dest, const std::wstring& src){
  dest.clear();
  for (size_t i = 0; i < src.size(); i++){
    wchar_t w = src[i];
    if (w <= 0x7f)
      dest.push_back((char)w);
    else if (w <= 0x7ff){
      dest.push_back(0xc0 | ((w >> 6) & 0x1f));
      dest.push_back(0x80 | (w & 0x3f));
    }
    else if (w <= 0xffff){
      dest.push_back(0xe0 | ((w >> 12) & 0x0f));
      dest.push_back(0x80 | ((w >> 6) & 0x3f));
      dest.push_back(0x80 | (w & 0x3f));
    }
    else if (w <= 0x10ffff){
      dest.push_back(0xf0 | ((w >> 18) & 0x07));
      dest.push_back(0x80 | ((w >> 12) & 0x3f));
      dest.push_back(0x80 | ((w >> 6) & 0x3f));
      dest.push_back(0x80 | (w & 0x3f));
    }
    else
      dest.push_back(' ');
  }
}
std::wstring ShareSpace::Utility::utf8ToWstr(const std::string& str)
{
#ifdef UTF8CXXOX
  typedef std::codecvt_utf8<wchar_t> convert_typeX;
  std::wstring_convert<convert_typeX, wchar_t> converterX;
  return std::move(converterX.from_bytes(str));
#else
  std::wstring ws;
  utf8toWStrReal(ws, str);
  return ws;
#endif
}
std::string ShareSpace::Utility::wstrToUtf8(const std::wstring& wstr)
{
#ifdef UTF8CXXOX
  typedef std::codecvt_utf8<wchar_t> convert_typeX;
  std::wstring_convert<convert_typeX, wchar_t> converterX;
  return std::move(converterX.to_bytes(wstr));
#else
  std::string str;
  wstrToUtf8Real(str, wstr);
  return str;
#endif
}
