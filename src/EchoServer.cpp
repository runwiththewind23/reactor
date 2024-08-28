#include"EchoServer.h"


EchoServer::EchoServer(const std::string &ip, const uint16_t port, int subthreadnum, int workthreadnum):tcpserver_(ip, port, subthreadnum), threadpool_(workthreadnum, "WORKS"){
    tcpserver_.setnewconnnectioncb(std::bind(&EchoServer::HandleNewConnection, this ,std::placeholders::_1));
    tcpserver_.setcloseconnnectioncb(std::bind(&EchoServer::HandleClose, this ,std::placeholders::_1));
    tcpserver_.seterrorconnnectioncb(std::bind(&EchoServer::HandleError, this ,std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage, this ,std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete, this ,std::placeholders::_1));
    //tcpserver_.settimeoutcb(std::bind(&EchoServer::HandleTimeout, this ,std::placeholders::_1));
    
}

EchoServer::~EchoServer(){

}
void EchoServer::start(){
    tcpserver_.start();
}

void EchoServer::stop(){
    //停止工作线程
    threadpool_.stop();
    printf("工作线程已停止。\n");
    //停止IO线程(事件循环)
    tcpserver_.stop();
    
}

void EchoServer::HandleNewConnection(spConnection conn){
    //std::cout<<"New Connection Come in."<<std::endl;
    //printf("HandleNewConnection thread is %ld.\n", syscall(SYS_gettid));
    //根据业务需求，
    printf("%s New connection(fd=%d, ip=%s,port=%d) ok.\n",Timestamp::now().tostring().c_str(),conn->fd(), conn->ip().c_str(), conn->port());
} //处理新客户端连接请求

void EchoServer::HandleClose(spConnection conn){
    //std::cout<<"EchoServer conn closed."<<std::endl;
    printf("%s connection(fd=%d, ip=%s,port=%d) closed.\n",Timestamp::now().tostring().c_str(),conn->fd(), conn->ip().c_str(), conn->port());

} //关闭客户端的连接

void EchoServer::HandleError(spConnection conn){
    //std::cout<<"EchoServer connection error."<<std::endl;
}//客户端的错误连接

void EchoServer::HandleMessage(spConnection conn, std::string &message){
    //printf("EchoServer::HandleMessage thread is %ld.\n", syscall(SYS_gettid));
    //把业务添加到线程池中
    if(threadpool_.size()==0){
        //如果没有工作线程，表示在IO线程中计算
        OnMessage(conn,message);
    }
    else threadpool_.addtask(std::bind(&EchoServer::OnMessage, this, conn, message));
}//处理客户端的请求报文，在Tcpserver类中回调此函数

void EchoServer::HandleSendComplete(spConnection conn){
    //std::cout<<"message send complete."<<std::endl;
}//数据发送完成后，在Tcpserver中回调
void EchoServer::HandleTimeout(EventLoop* loop){
    std::cout<<"EchoServer timeout."<<std::endl;
}//epoll_wait()超时，在Tcpserver类中回调


void EchoServer::OnMessage(spConnection conn, std::string& message){
    //printf("%s message (eventfd=%d):%s\n",Timestamp::now().tostring().c_str(), conn->fd(),message.c_str());
    message="reply:"+message;//回显业务

    //printf("处理完业务后使用connection对象。\n");
    conn->send(message.data(),message.size());//把数据发送出去,使用野指针后果未知
}//处理客户端的请求报文，用于添加给线程池
