#pragma once
#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"
/*
    EchoServer类：回显服务器
*/

class EchoServer{
    private:
        TcpServer tcpserver_;
        ThreadPool threadpool_;
    public:
        EchoServer(const std::string &ip, const uint16_t port, int subthreadnum = 3, int workthreadnum = 5);
        ~EchoServer();
        void start();//启动业务
        void stop();//停止业务

        void HandleNewConnection(spConnection conn); //处理新客户端连接请求
        void HandleClose(spConnection conn);//关闭客户端的连接
        void HandleError(spConnection conn);//客户端的错误连接
        void HandleMessage(spConnection conn, std::string &message);//处理客户端的请求报文，在Tcpserver类中回调此函数
        void HandleSendComplete(spConnection conn);//数据发送完成后，在Tcpserver中回调
        void HandleTimeout(EventLoop* loop);//epoll_wait()超时，在Tcpserver类中回调

        void OnMessage(spConnection conn, std::string &message);//处理客户端的请求报文，用于添加给线程池

};