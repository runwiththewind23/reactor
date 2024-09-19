#include"Acceptor.h"
#include"Connection.h"

Acceptor::Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port):loop_(loop),servsock_(createnoblocking()), acceptchannel_(loop_, servsock_.fd()) {
    //servsock_ = new Socket(createnoblocking());
    InetAddress servaddr(ip, port);//服务端的地址和协议
    servsock_.setreuseaddr(true);
    servsock_.setreuseport(true);
    servsock_.setkeepalive(true);
    servsock_.settcpnodelay(true);
    servsock_.bind(servaddr);
    servsock_.listen();

    //acceptchannel_ = new Channel(loop_, servsock_.fd());
    acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection, this));
    acceptchannel_.enablereading();
}

Acceptor::~Acceptor(){
    //delete servsock_;
    //delete acceptchannel_;
}

void Acceptor::newconnection(){   //处理新客户端连接请求
    InetAddress clientaddr;             // 客户端的地址和协议。
    std::unique_ptr<Socket>clientsock(new Socket(servsock_.accept(clientaddr)));
    clientsock->setipport(clientaddr.ip(), clientaddr.port());
    /*
     注意：clientsock只能new出来，不能在栈上，否则析构函数会关闭fd
     还有new出来的对象还没释放
    */
    //printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",clientsock->fd(),clientaddr.ip(),clientaddr.port());
    newconnectioncb_(std::move(clientsock));//回调了TCPserver::newconnection（）
    }

void Acceptor::setnewconnnectioncb(std::function<void(std::unique_ptr<Socket>)>fn){
    newconnectioncb_=fn;
}