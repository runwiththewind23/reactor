#include "Channel.h"


Channel::Channel(EventLoop *loop, int fd):loop_(loop),fd_(fd){//构造函数

}

Channel::~Channel(){//析构函数
    //析构函数中，不要销毁loop_，也不能关闭fd_，这两个不属于Channel成员
}


int Channel::fd(){
    return fd_;
}                   //返回fd成员
void Channel::useet(){
    events_= events_|EPOLLET;
}               //采用边缘触发
void Channel::enablereading(){
    events_|= EPOLLIN;
    loop_ -> updatechannel(this);
}   //让epoll_wait()监视fd_的读事件

void Channel::disablereading(){
    events_&=~EPOLLIN;
    loop_ -> updatechannel(this);

}        //取消读事件

void Channel::enablewriting(){
    events_|=EPOLLOUT;
    loop_ -> updatechannel(this);

}        //注册写事件

void Channel::disablewriting(){
    events_&=~EPOLLOUT;
    loop_ -> updatechannel(this);

}        //取消写事件

void Channel::disableall(){
    events_=0;
    loop_ -> updatechannel(this);

}

void Channel::remove(){
    disableall();//先取消全部的事件
    loop_ -> updatechannel(this);//从红黑树删除fd
}


void Channel::setinepoll(){
    inepoll_ = true;
}    //把inepoll_成员的值设置未true
void Channel::setrevents(uint32_t ev){
    revents_ = ev;
    //设置revents_成员的值未参数ev
}
bool Channel::inpoll(){
    return inepoll_;
}            //返回inepoll_成员
uint32_t Channel::events(){
    return events_;
}        //返回events_成员
uint32_t Channel::revents(){
    return revents_;
}      //返回revents_成员

void Channel::handleevent(){
    if (revents_ & EPOLLRDHUP) {                    // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
        closecallback_();
    }                                //  普通数据  带外数据
    else if (revents_ & (EPOLLIN|EPOLLPRI))   // 接收缓冲区中有数据可以读。
    {   
        readcallback_();
    }
    else if (revents_ &EPOLLOUT)                  // 有数据需要写，暂时没有代码，以后再说。
    {
        writecallback_();
    }
    else                                                                   // 其它事件，都视为错误。
    {
        errorcallback_();
    }
}

void Channel::setreadcallback(std::function<void()>fn) {
    readcallback_=fn;
}

void Channel::setclosecallback(std::function<void()>fn) {
    closecallback_=fn;
}

void Channel::seterrorcallback(std::function<void()>fn) {
    errorcallback_=fn;
}

void Channel::setwritecallback(std::function<void()>fn){
    writecallback_=fn;
}

