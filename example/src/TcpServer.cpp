#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip, const uint16_t port, int threadnum):threadnum_(threadnum),mainloop_(new EventLoop(true)),acceptor_(mainloop_.get(),ip, port),threadpool_(threadnum_, "IO"){
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));

    acceptor_.setnewconnnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));
    

    //创建从事件循环
    for(int ii = 0; ii < threadnum_; ++ii) {
        subloops_.emplace_back(new EventLoop(false,5,10));//创建从事件循环，放入subloops容器中
        subloops_[ii]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));//设置timeout超时的回调
        subloops_[ii]->settimercallback(std::bind(&TcpServer::removeconn, this, std::placeholders::_1));
        threadpool_.addtask(std::bind(&EventLoop::run, subloops_[ii].get()));//在线程池中加入从事件循环
        sleep(1);
    }
}
TcpServer::~TcpServer(){
}

void TcpServer::start(){
    mainloop_->run();
}

void TcpServer::stop(){
    mainloop_->stop();
    printf("主事件已停止。\n");

    //停止从事件循环
    for(int ii = 0; ii < threadnum_; ++ii){
        subloops_[ii]->stop();
    }
    printf("从事件已停止.\n");
    //停止IO线程
    threadpool_.stop();
    printf("线程池已停止.\n");
}

void TcpServer::newconnection(std::unique_ptr<Socket> clientsock) {
    spConnection conn(new Connection(subloops_[clientsock->fd()%threadnum_].get(),std::move(clientsock)));
    conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1,std::placeholders::_2));
    conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete, this, std::placeholders::_1));

    {
        std::lock_guard<std::mutex>gd(mmutex_);
        conns_[conn->fd()]=conn;//把conn存放在map容器中
    }
    subloops_[conn->fd()%threadnum_]->newconnection(conn);//把conn存放到EventLoop的map容器中

    if(newconnectioncb_)newconnectioncb_(conn);//建立完毕再回调
}

void TcpServer::closeconnection(spConnection conn){
    if(closeconnectioncb_)closeconnectioncb_(conn);//先回调再关闭
    //printf("client(eventfd=%d) disconnected.\n",conn ->fd());
    {
        std::lock_guard<std::mutex>gd(mmutex_);
        conns_.erase(conn->fd());
    }
    //delete conn;
}//关闭客户端的连接
void TcpServer::errorconnection(spConnection conn){
    if(errorconnectioncb_)errorconnectioncb_(conn);
    //printf("client(eventfd=%d) error.\n",conn->fd());
    {
        std::lock_guard<std::mutex>gd(mmutex_);
        conns_.erase(conn->fd());
    }
    //delete conn;
    //close(conn->fd());
}//客户端的错误连接

void TcpServer::onmessage(spConnection conn, std::string &message) {
    if(onmessagecb_)onmessagecb_(conn,message);
}

void TcpServer::sendcomplete(spConnection conn) {//数据发送完成后，在connection中回调
    //printf("send complete.\n");
    if(sendcompletecb_)sendcompletecb_(conn);
}

void TcpServer::epolltimeout(EventLoop *loop){//epoll_wait超时，在Eventloop类中回调
    if(timeoutcb_)timeoutcb_(loop);
    //根据业务需要，添加代码
}

void TcpServer::setnewconnnectioncb(std::function<void(spConnection)>fn){
    newconnectioncb_=fn;
}

void TcpServer::setcloseconnnectioncb(std::function<void(spConnection)>fn){
    closeconnectioncb_=fn;
}
void TcpServer::seterrorconnnectioncb(std::function<void(spConnection)>fn){
    errorconnectioncb_=fn;
}
void TcpServer::setonmessagecb(std::function<void(spConnection, std::string &message)>fn){
    onmessagecb_=fn;
}
void TcpServer::setsendcompletecb(std::function<void(spConnection)>fn){
    sendcompletecb_=fn;
}
void TcpServer::settimeoutcb(std::function<void(EventLoop*)>fn){
    timeoutcb_=fn;
}

void TcpServer::removeconn(int fd){
    {
        std::lock_guard<std::mutex>gd(mmutex_);
        conns_.erase(fd);//从map中删除conn
    }
    if(removeconncb_)removeconncb_(fd);
}//删除conns_中的connection对象，在eventloop::handletimer中回调     

void TcpServer::setremoveconncb(std::function<void(int)> fn){
    removeconncb_=fn;
}       