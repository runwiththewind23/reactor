#include"Connection.h"


Connection::Connection(EventLoop *loop, std::unique_ptr<Socket>clientsock):loop_(loop), clientsock_(std::move(clientsock)),disconnect_(false), clientchannel_(new Channel(loop_, clientsock_->fd())){
    //为客户端连接准备读事件，并添加到epoll中
    //clientchannel_ = new Channel(loop_, clientsock_->fd());
    clientchannel_ -> setreadcallback(std::bind(&Connection::onmessage,this));
    clientchannel_ -> setclosecallback(std::bind(&Connection::closecallback,this));
    clientchannel_ -> seterrorcallback(std::bind(&Connection::errorcallback,this));
    clientchannel_ ->useet();//连接上采用边缘触发
    clientchannel_->  setwritecallback(std::bind(&Connection::writecallback,this));
    clientchannel_ -> enablereading();
}

Connection::~Connection(){
    
}

int Connection::fd()const{//构造函数持有的fd，在构造函数中传进来
    return clientsock_->fd();  
}        
std::string Connection::ip(){
    return clientsock_->ip();
}//如果是listenfd存放服务端监听的ip，如果是客户端，存放对端的ip
uint16_t Connection::port(){
    return clientsock_->port();
}//果是listenfd存放服务端监听的port，如果是客户端，存放对端的port

void Connection::closecallback(){
    //printf("client(eventfd=%d) disconnected.\n",fd());
    //close(fd());            // 关闭客户端的fd。
    disconnect_=true;
    clientchannel_->remove();
    closecallback_(shared_from_this());

}//TCP连接断开的回调函数，供channel回调
void Connection::errorcallback(){
    //printf("client(eventfd=%d) error.\n",fd());
    //close(fd());
    disconnect_=true;
    clientchannel_->remove();
    errorcallback_(shared_from_this());
}//TCP连接错误的回调函数，供channel回


void Connection::setclosecallback(std::function<void(spConnection)>fn){
    closecallback_=fn;
}//设置关闭fd_的回调函数
void Connection::seterrorcallback(std::function<void(spConnection)>fn){
    errorcallback_=fn;
}//设置fd_发生了错误的回调函数
//设置处理报文的回调函数
void Connection::setonmessagecallback(std::function<void(spConnection, std::string&)>fn) {
    onmessagecallback_ = fn;
}
//设置数据完成后的回调函数
void Connection::setsendcompletecallback(std::function<void(spConnection)>fn){
    sendcompletecallback_ = fn;
}

//发送数据
void Connection::send(const char *data, size_t size){
    if(disconnect_==true){
        std::cout<<"客户端连接已断开，send()直接返回。\n"<<std::endl;
        return ;
    }
    std::shared_ptr<std::string>message(new std::string(data));
    if(loop_->isinloopthread()){//判断当前线程是否为事件循环线程（IO线程）
    //如果当前线程是IO线程，直接执行发送数据的操作
        sendinloop(message);
    }else{
    //如果不是，把发送数据的操作交给IO线程
        loop_->queueinloop(std::bind(&Connection::sendinloop, this, message));
    }

    

}

/*void Connection::sendinloop(const char *data, size_t size){
    //printf("Connection::send() thread is %ld.\n", syscall(SYS_gettid));
    outputbuffer_.appendwithhead(data,size);//把需要发送的数据保存到Connection的缓冲区中
    clientchannel_->enablewriting();//注册写事件
}*/
void Connection::sendinloop(std::shared_ptr<std::string>data){
    outputbuffer_.appendwithsep(data->data(),data->size());//把需要发送的数据保存到Connection的缓冲区中
    clientchannel_->enablewriting();//注册写事件
}

//设置写事件
void Connection::writecallback(){
    int writen =::send(fd(), outputbuffer_.data(), outputbuffer_.size(), 0);//尝试把outputbuffer_中的数据发出去
    if(writen>0)outputbuffer_.erase(0,writen);//删除已成功发送的
    //printf("Connection::writecallback() thread is %ld.\n", syscall(SYS_gettid));
    if(outputbuffer_.size()==0){
    clientchannel_->disablewriting();//如果缓冲区没有数据，不关注
    sendcompletecallback_(shared_from_this());
    }
}



void Connection::onmessage(){                       //处理对端发送过来的消息
    char buffer[1024];
        while (true)             // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
        {    
            bzero(&buffer, sizeof(buffer));
            ssize_t nread = read(fd(), buffer, sizeof(buffer));
            if (nread > 0)      // 成功的读取到了数据。
            {
                // 把接收到的报文内容原封不动的发回去。
                //printf("recv(eventfd=%d):%s\n",fd(),buffer);
                //send(fd(),buffer,strlen(buffer),0);
                inputbuffer_.append(buffer,nread);//把已读取的数据追加到接收缓冲区
            } 
            else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
            {  
                continue;
            } 
            else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
            {   
                //printf("recv(eventfd=%d):%s\n",fd(),inputbuffer_.data());
                //经过了很复杂的运算
                std::string message;
                while(true){
                    if(inputbuffer_.pickmessage(message)==false) break;
                    //printf("message (eventfd=%d):%s\n",fd(), message.c_str());
                    lasttime_ = Timestamp::now();//更新connection时间戳
                    //std::cout<<"last_time为："<<lasttime_.tostring() <<std::endl;
                    onmessagecallback_(shared_from_this(),message);//回调TCPServer::onmessage()
                }
                break;
            } 
            else if (nread == 0)  // 客户端连接已断开。
            {  
                closecallback();//回调TCPSErver::closecallback()
                break;
            }
        }
    }
bool Connection::timeout(time_t now, int val){
    return now-lasttime_.toint()>val;
    
}//判断TCP连接是否超时

