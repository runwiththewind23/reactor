#pragma once
#include<functional>
#include"Socket.h"
#include"InetAddress.h"
#include"Channel.h"
#include"EventLoop.h"
#include "Timestamp.h"
#include "Buffer.h"
#include <memory>
#include <atomic>
#include<sys/syscall.h>

class EventLoop;
class Channel;
class Connection;
using spConnection = std::shared_ptr<Connection>;


class Connection:public std::enable_shared_from_this<Connection>{
    private:
        EventLoop *loop_;//Acceptor对应的事件循环，在构造函数中传入
        std::unique_ptr<Socket> clientsock_;//服务端用于监听的socket，在构造函数中创建
        std::unique_ptr<Channel> clientchannel_;//Acceptor对应的channel，在构造函数中创建(160字节)
        Buffer inputbuffer_;//接收缓冲区
        Buffer outputbuffer_;//接收缓冲区

        std::atomic_bool disconnect_;//客户端连接是否断开，如果已断开，设置为true    

        std::function<void(spConnection)>closecallback_;//关闭fd_的回调函数，将回调TCp::closeconnection
        std::function<void(spConnection)>errorcallback_;//fd_发生错误的回调函数
        std::function<void(spConnection, std::string&)>onmessagecallback_;//处理报文的回调函数
        std::function<void(spConnection)>sendcompletecallback_;//发送数据完成后的回调函数
        Timestamp lasttime_;//时间戳，创建Connection对象为当前时间，没收到一个报文，更新为当前时间
    
    public:
        Connection(EventLoop *loop, std::unique_ptr<Socket>clientsock);
        ~Connection();
        int fd()const;//构造函数持有的fd，在构造函数中传进来
        std::string ip();//如果是listenfd存放服务端监听的ip，如果是客户端，存放对端的ip
        uint16_t port();//果是listenfd存放服务端监听的port，如果是客户端，存放对端的port
        
        void onmessage();                       //处理对端发送过来的消息
        void closecallback();//TCP连接断开的回调函数，供channel回调
        void errorcallback();//TCP连接错误的回调函数，供channel回调
        void writecallback();//处理写事件的回调函数，供channel回调

        void setclosecallback(std::function<void(spConnection)>fn);//设置关闭fd_的回调函数
        void seterrorcallback(std::function<void(spConnection)>fn);//设置fd_发生了错误的回调函数
        void setonmessagecallback(std::function<void(spConnection,std::string&)>fn);//设置处理客户端的请求报文
        void send(const char *data, size_t size);//发送数据,不管在任何线程中，都调用此函数
        void sendinloop(std::shared_ptr<std::string>data);//发送数据，如果当前是IO线程，直接调用，如果是工作线程，将此函数传递给IO线程
        void setsendcompletecallback(std::function<void(spConnection)>fn);//设置数据发送完成后的回调函数

        bool timeout(time_t now, int val);//判断TCP连接是否超时

};