#ifndef __UVNET_version_h__
#define __UVNET_version_h__
#include <string>
namespace{
  const std::string VERSION_MAJOY("0");
  const std::string VERSION_MINOR("0");
  const std::string VERSION_BUILD("1");
  const std::string VERSION_REVISION("");
  //version number 
}
inline std::string uvNetVersion(){
  return VERSION_MAJOY + "." + VERSION_MINOR + "." + VERSION_BUILD + "." + VERSION_REVISION;
}
#endif //__UVNET_version_h__
