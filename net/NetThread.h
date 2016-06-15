/********************************************************************

  Filename:   NetThread

  Description:NetThread

  Version:  1.0
  Created:  31:3:2015   15:33
  Revison:  none
  Compiler: gcc vc

  Author:   wufan, love19862003@163.com

  Organization:
*********************************************************************/
#ifndef __NetThread_H__
#define __NetThread_H__
#include "net/NetCore.h"
#include "net/NetUV.h"
#include <atomic>
#include <list>
#include <set>
#include <thread>
#include <mutex>
namespace ShareSpace {
  namespace NetSpace {
     class NetThread {
     private:
       NetThread(const NetThread&) = delete;
       NetThread& operator=(const NetThread&) = delete;

       enum {
         TIME_INTERVATE = 10000,
       };
     public:
       enum ThreadState {
         _INIT_ = 0,
         _RUN_ = 1,
         _STOP_ = 2,
       };

       enum {
         async_add_object,
         async_stop_thread,
         async_send_message,
         async_kick_session,
         async_max_,
       };
       enum {
         mutex_send,
         mutex_recive,
         mutex_session,
         mutex_object,
         mutex_max_,
       };

       explicit  NetThread();
       ~NetThread();
       uv_loop_t* loop() { return m_loop; }
       // set stop work thread  main thread
       void stop();
       // start thread  main thread
       void start();
       //work thread 
       void threadRun();
       //work thread  
       void realSend();
       //work thread 
       void addSession(SessionPtr s);
       //work thread 
       void recvMsgList(const std::list<MessagePtr>& list, size_t size);
       //main thread
       bool pollThread(std::list<MessagePtr>& msg, std::list<SessionPtr>& conn, std::list<SessionPtr>&dis);
       //main thread
       void notifySend(MessagePtr msg);
       //main thread
       bool notifyKick();
       //main thread
       bool check();
       //main thread
       bool addObject( ObjectPtr obj);
       //thread info
       std::string info() const;
       // speed for send and recv
       inline size_t getSpeed() const{return m_lastTimeRecv + m_lastTimeSend;}
       //
       inline size_t value() const{ return m_value;}
     protected:
       //work thread
       void asyncAddObject();
       //work thread
       void asyncStopThread();
       //work thread
       void asyncKickSession();
     private:
       uv_loop_t*  m_loop;                  //�߳����ڵ�loop
       uv_timer_t  m_time;                  //io���ݶ�ʱͳ��
       std::thread m_thread_work;           //�߳�
       uv_async_t  m_asyncs[async_max_];    //�첽�ź�
       std::mutex  m_mutexs[mutex_max_];    //��
       ThreadState m_state;                 //�߳�����״̬
       std::list<ObjectPtr>   m_objects;    //�������
       std::set<SessionPtr>   m_onlieList;  //��ǰ���ߵ�����
       std::list<MessagePtr>  m_recvList;   //���߳��յ�����Ϣ�б�
       std::list<MessagePtr>  m_sendList;   //���߳���Ҫ�·�����Ϣ�б�
       size_t m_onlines = 0;                //���ߵ�session����
       size_t m_totalRealRecv = 0;          //�յ����ֽ���
       size_t m_totalRealRecvCount = 0;     //�յ�����Ϣ��
       size_t m_totalRealSendCount = 0;     //���͵���Ϣ��
       size_t m_totalRealSendSize = 0;      //���͵��ֽ���
       size_t m_totalSendCount = 0;         //�ܼ������͵���Ϣ��
       size_t m_totalSendSize = 0;          //�ܼ������͵��ֽ���
       size_t m_timerSend = 0;              //��������ͳ��
       size_t m_timerRecv = 0;              //��ȡ����ͳ��
       size_t m_lastTimeSend = 0;           //�ϴεķ�������ͳ��
       size_t m_lastTimeRecv = 0;           //�ϴε���ȡ����ͳ��
       size_t m_value = 0;                  //
       
     };
  }
}
#endif