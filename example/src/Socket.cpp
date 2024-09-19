#include "Socket.h"

/*class Socket{
    private:
        const int fd_;//构造函数持有的fd，在构造函数中传进来
    public:
        Socket(int fd);//构造函数，传入一个准备好的fd
        ~Socket();//在析构函数中，关闭fd_

        int fd()const;//返回fd成员
        void setreuseaddr(bool on);//设置SO_REUSEADDR
        void setreuseport(bool on);//设置SO_REUSEPORT
        void settcpnodelay(bool on);//设置TCP_NODELAY
        void setkeepalive(bool on);//设置SO_KEEPALIVE

        void bind(const InetAddress& servaddr);
        void listen(int nn = 128);
        void accept(InetAddress& clientaddr);
};*/
int createnoblocking(){
    int listenfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
    if (listenfd < 0)
    {
        perror("socket() failed"); 
        exit(-1);
        /*printf("%s:%s:%d listened socket create error:%d\n", __FILE,_FUNCTION_,__LINE__,errno);
        exit(-1);*/
    }
    return listenfd;
}
Socket::Socket(int fd):fd_(fd){

}
Socket::~Socket(){
    ::close(fd_);
}

int Socket::fd()const{
    return fd_;
}

std::string Socket::ip()const{
    return ip_;
}

uint16_t Socket::port()const{
    return port_;
}

void Socket::settcpnodelay(bool on){
    int optval = on ?1:0;
    ::setsockopt(fd_,IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}
void Socket::setreuseaddr(bool on){
    int optval = on ?1:0;
    ::setsockopt(fd_,SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}
void Socket::setreuseport(bool on){
    int optval = on ?1:0;
    ::setsockopt(fd_,SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}
void Socket::setkeepalive(bool on){
    int optval = on ?1:0;
    ::setsockopt(fd_,SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

void Socket::bind(const InetAddress& servaddr){
    if (::bind(fd_,servaddr.addr(),sizeof(sockaddr)) < 0 ){
        perror("bind() failed"); close(fd_); exit(-1);
    }
    setipport(servaddr.ip(), servaddr.port());
}
void Socket::listen(int nn){
    if (::listen(fd_,nn) != 0 ) {        // 在高并发的网络服务器中，第二个参数要大一些。
        perror("listen() failed"); close(fd_); exit(-1);
    }
}
int Socket::accept(InetAddress& clientaddr){
    sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientfd = accept4(fd_,(sockaddr*)&peeraddr,&len,SOCK_NONBLOCK);
    clientaddr.setaddr(peeraddr);         // 客户端的地址和协议。
    return clientfd;
}

void Socket::setipport(const std::string &ip, uint16_t port){
    ip_ = ip;
    port_ = port;
}