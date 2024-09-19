#pragma once

#include"Epoll.h"
#include"Channel.h"
#include "Connection.h"
#include <functional>
#include <memory>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include<sys/timerfd.h>
#include <unistd.h>
#include<mutex>
#include<queue>
#include<map>
#include<atomic>


class Channel;
class Epoll;
class Connection;
using spConnection = std::shared_ptr<Connection>;


class EventLoop{
    private:
        int timetvl_;//闹钟间隔，单位秒
        int timeout_;//connection超时的时间，秒
        std::unique_ptr<Epoll> ep_;//每个事件循环只有一个Epoll
        std::function<void(EventLoop*)>epolltimeoutcallback_;//epoll_wait()超时的回调函数
        pid_t threadid_;//事件循环所在线程的id
        std::queue<std::function<void()>>taskqueue_;//事件循环线程被eventfd唤醒后执行的任务队列
        std::mutex mutex_;//任务队列同步的互斥锁
        int wakeupfd_;//用于唤醒时间循环线程的eventfd
        std::unique_ptr<Channel> wakechannel_;//eventfd的channel
        int timerfd_;//定时器的fd
        std::unique_ptr<Channel> timerchannel_;//定时器的channel
        bool mainloop_;//true是主事件循环，false是从事件循环
        std::mutex mmutex_;//保护conns_的互斥所
        std::map<int, spConnection> conns_;
        std::function<void(int)>timercallback_;//删除TCPserver中的connection对象，将被设置为TCPserver::removeconn()的回调
        std::atomic_bool stop_;//初始值为false，如果设置为true，表示停止事件循环

    public:
        EventLoop(bool mainloop, int timetvl=30, int timeout = 80);
        ~EventLoop();
        
        void run();//运行事件循环
        void stop();//停止事件循环
        
        void removechannel(Channel *ch);//从红黑树中删除Channel
        void updatechannel(Channel *ch);//把channel添加/更新到红黑树中，channel中有fd，也有需要监视的事件
        void setepolltimeoutcallback(std::function<void(EventLoop*)>fn);//设事件循环的回调

        bool isinloopthread();//判断当前线程是否为事件循环线程

        void queueinloop(std::function<void()>fn);//把任务添加到队列中
        void wakeup();//用eventfd唤醒事件循环
        void handlewakeup();//事件循环被eventfd唤醒后执行的函数

        void handletime();

        void newconnection(spConnection conn);//把connection对象保存在conns_中

        void settimercallback(std::function<void(int)>fn);//被设置为TCPserver::removeconn()的回调
};