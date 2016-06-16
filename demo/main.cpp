#include <mutex>
#include <thread>
#include <atomic>
#include <signal.h>
#include <iostream>

#include "log/MyLog.h"
#include "net/Version.h"
#include "utility/StringUtility.h"
#include "net/NetManager.h"
#include "demo/demo.h"

using namespace ShareSpace;
static std::atomic<bool> isRunning(true);
static std::function<void()> quitFunction = nullptr;
void signalHandler(int /*sig*/){
  isRunning = false;
  if(quitFunction){ quitFunction(); }
}
int main(int argc, char* argv[]){
   LOGINIT("");
   LOGDEBUG(uvNetVersion());
   

   std::string threadStr;
   std::mutex mutex;
   std::thread t([&]()->void{
     while(true){
       std::string str;
       std::cin >> str;
       {
        std::lock_guard<std::mutex> lock(mutex);
        if (!threadStr.empty()){
          threadStr.append(" ");
        }
        threadStr.append(str);
       }
     }
   
   });

   auto fun = [&](){
     std::string cmd;
     {
       std::lock_guard<std::mutex> lock(mutex);
       cmd = threadStr;
       threadStr.clear();
     }

     if (cmd.empty()){ return ;}
     Demo::command(cmd);

   };

   quitFunction = [&] ()->void { LOGDEBUG("handle net stop");};
   signal(SIGABRT, signalHandler);
   signal(SIGINT, signalHandler);
   unsigned int tc = 4;
   NetSpace::NetManager net(tc);
   Demo::createDemo(net);
   net.start();
   while(isRunning.load()){
    fun();
    net.poll();
   }
   net.stop(3000);
   LOGRELEASE();
}