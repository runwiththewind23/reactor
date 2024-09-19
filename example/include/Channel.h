#pragma once
#include<functional>
#include<sys/epoll.h>
#include"Epoll.h"
#include"InetAddress.h"
#include"Socket.h"
#include"EventLoop.h"
#include "Connection.h"
#include <memory>

class EventLoop;

class Channel{
    private:
        int fd_ = -1;               //channel拥有的fd是一对一关系
        EventLoop *loop_;
        //Epoll *ep_ = nullptr;       //channel对应的红黑树，channel与Epoll是多对一的关系，一个channel只对应一个Epoll
        bool inepoll_ = false;      //channel是否已添加到epoll树上，如果未添加，调用时用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD
        uint32_t events_ = 0;       //fd_需要监视的事件，listenfd和clientfd还可能需要监视EPOLLOUT
        uint32_t revents_ = 0;      //fd_已发生的事件
        //bool islisten_ = false;//listenfd取值为true,客户端连接的为false
        std::function<void()>readcallback_;//fd_读事件的回调函数
        std::function<void()>closecallback_;//关闭fd_的回调函数
        std::function<void()>errorcallback_;//fd_发生错误的回调函数
        std::function<void()>writecallback_;//fd+发生

    public:
        Channel(EventLoop *loop, int fd);//构造函数
        ~Channel();

        int fd();                   //返回fd成员
        void useet();               //采用边缘触发
        void enablereading();        //让epoll_wait()监视fd_的读事件
        void disablereading();        //取消读事件
        void enablewriting();        //注册写事件
        void disablewriting();        //取消写事件
        void disableall();              //取消全部的事件
        void remove();              //从事件循环中删除Channel
        void setinepoll();          //把inepoll_成员的值设置未true
        void setrevents(uint32_t ev);//设置revents_成员的值未参数ev
        bool inpoll();              //返回inepoll_成员
        uint32_t events();          //返回events_成员
        uint32_t revents();         //返回revents_成员

        void handleevent();         //事件处理函数epoll_wait()返回的时候执行

        void newconnection(Socket *servsock);   //处理新客户端连接请求
        //void onmessage();                       //处理对端发送过来的消息
        void setreadcallback(std::function<void()>fn);//设置fd_读事件的回调函数

        void setclosecallback(std::function<void()>fn);//关闭fd_的回调函数
        void seterrorcallback(std::function<void()>fn);//fd_发生错误的回调函数
        void setwritecallback(std::function<void()>fn);//设置写事件的回调函数
};