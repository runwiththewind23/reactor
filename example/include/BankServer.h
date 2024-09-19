#pragma once
#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"
/*
    BankServer类：网上银行服务器
*/
class UserInfo{
    private:
        int fd_;
        std::string ip_;
        bool isLogin_ = false;//是否已登陆
        std::string username_;//用户名
    public:
        UserInfo(int fd, const std::string &ip) : fd_(fd), ip_(ip){}
        void setLogin(bool isLogin) {isLogin_=isLogin;}
        bool isLogin(){return isLogin_;};

};//用户信息（状态机）




class BankServer{
    private:
        using spUserInfo=std::shared_ptr<UserInfo>;
        TcpServer tcpserver_;
        ThreadPool threadpool_;
        std::map<int, spUserInfo> users_;//fd和UserInfo的映射（用户状态机）
        std::mutex mutex_;//保护users_的互斥锁
    public:
        BankServer(const std::string &ip, const uint16_t port, int subthreadnum = 3, int workthreadnum = 5);
        ~BankServer();
        void start();//启动业务
        void stop();//停止业务

        void HandleNewConnection(spConnection conn); //处理新客户端连接请求
        void HandleClose(spConnection conn);//关闭客户端的连接
        void HandleError(spConnection conn);//客户端的错误连接
        void HandleMessage(spConnection conn, std::string &message);//处理客户端的请求报文，在Tcpserver类中回调此函数
        void HandleSendComplete(spConnection conn);//数据发送完成后，在Tcpserver中回调
        void HandleTimeout(EventLoop* loop);//epoll_wait()超时，在Tcpserver类中回调

        void HandleRemove(int fd);//从users_中移除fd所对应的UserInfo
        void OnMessage(spConnection conn, std::string &message);//处理客户端的请求报文，用于添加给线程池

};