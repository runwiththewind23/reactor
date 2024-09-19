#pragma once
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include<map>
#include<memory>
#include<mutex>

//网络服务类
class TcpServer{
    private:
        std::unique_ptr<EventLoop> mainloop_; //主事件循环
        std::vector<std::unique_ptr<EventLoop>> subloops_;//存放从事件循环的容器
        int threadnum_;//线程池的大小，即从事件循环的个数
        Acceptor acceptor_; //一个TcpServer只要一个Acceptor对象
        ThreadPool threadpool_;//线程池
        std::mutex mmutex_;//保护线程的互斥所
        //Connection *Connection_;
        std::map<int, spConnection>conns_;//一个TCPServer有多个Connection对象，存放在map容器中

        std::function<void(spConnection)>newconnectioncb_;
        std::function<void(spConnection)>closeconnectioncb_;
        std::function<void(spConnection)>errorconnectioncb_;
        std::function<void(spConnection, std::string &message)>onmessagecb_;
        std::function<void(spConnection)>sendcompletecb_;
        std::function<void(EventLoop*)>timeoutcb_;
        std::function<void(int)>removeconncb_;//在eventloop::handletimer中回调




    public:
        TcpServer(const std::string &ip, const uint16_t port, int threadnum = 3);
        ~TcpServer();
        void start();
        void stop();
        void newconnection(std::unique_ptr<Socket> clientsock); //处理新客户端连接请求
        void closeconnection(spConnection conn);//关闭客户端的连接
        void errorconnection(spConnection conn);//客户端的错误连接
        void onmessage(spConnection conn, std::string &message);//处理客户端的请求报文，在Connection类中回调此函数
        void sendcomplete(spConnection conn);//数据发送完成后，在Connection中回调
        void epolltimeout(EventLoop* loop);//epoll_wait()超时，在EventLoop类中回调

        void setnewconnnectioncb(std::function<void(spConnection)>fn);
        void setcloseconnnectioncb(std::function<void(spConnection)>fn);
        void seterrorconnnectioncb(std::function<void(spConnection)>fn);
        void setonmessagecb(std::function<void(spConnection, std::string &message)>fn);
        void setsendcompletecb(std::function<void(spConnection)>fn);
        void settimeoutcb(std::function<void(EventLoop*)>fn);

        void removeconn(int fd);//删除conns_中的connection对象，在eventloop::handletimer中回调     
        void setremoveconncb(std::function<void(int)>fn);
};