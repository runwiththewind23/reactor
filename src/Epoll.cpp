#include "Epoll.h"

/*class Epoll{
    private:
        static const int MaxEvents = 100;//epoll_wait()返回事件数组的大小
        int epollfd = -1;//epoll句柄，在构造函数中创建
        epoll_event events_[MaxEvents];//存放epoll_wait()返回事件的数组，在构造函数中分配内存
    public:
        Epoll();//在构造函数中创建epollfd_
        ~Epoll();//在析构函数中关闭epollfd_

        void addfd(int fd, uint32_t op);//吧fd和它需要监视的事件添加到红黑树
        std::vector<epoll_event>loop(int timeout = -1);//运行epoll_wait()，等待事件的发生，已发生的事件用vector返回
        
};
*/

Epoll::Epoll(){//在构造函数在创建epollfd_
    if((epollfd_=epoll_create(1)) == -1) {//创建epoll句柄（红黑树）
        printf("epoll_create() failed(%d).\n", errno);
        exit(-1);
    }
}
Epoll::~Epoll(){//在析构函数中关闭
    close(epollfd_);
}

void Epoll::updatechannel(Channel *ch){
    epoll_event ev;//声明结构体
    ev.data.ptr = ch;
    ev.events = ch -> events();//指定事件

    if(ch->inpoll()){//如果已经在树上了
        if(epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1) {
            perror("epoll_ctl() failed.\n");
            exit(-1);
        }
    }else{  //如果不在树上
        if(epoll_ctl(epollfd_,EPOLL_CTL_ADD, ch->fd(),&ev) == -1){
            perror("epoll_ctl() failed.\n");
            exit(-1);
        }
        ch->setinepoll();//把channel的inepoll_成员设置为true;
    }
}

void Epoll::removechannel(Channel* ch){
    if(ch->inpoll()){//如果已经在树上了
        printf("removechannel.\n");
        if(epoll_ctl(epollfd_, EPOLL_CTL_DEL, ch->fd(), 0) == -1) {
            perror("epoll_ctl() failed.\n");
            exit(-1);
        }
    }
}

/*
void Epoll::addfd(int fd, uint32_t op) {
     // 为服务端的listenfd准备读事件。
    epoll_event ev;              // 声明事件的数据结构。
    ev.data.fd=fd;       // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events=op;      // 让epoll监视listenfd的读事件，采用水平触发。

    if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&ev)==-1){     // 把需要监视的listenfd和它的事件加入epollfd中。
        printf("epoll_ctl()failed(%d).\n", errno);
        exit(-1);
    }
} 
*/
/*std::vector<epoll_event>Epoll::loop(int timeout){//运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回
    std::vector<epoll_event>evs;//存放epoll_wait()返回的事件
    bzero(events_, sizeof(events_));
    int infds=epoll_wait(epollfd_,events_,MaxEvents,timeout);       // 等待监视的fd有事件发生。

    // 返回失败。
    if (infds < 0){
        perror("epoll_wait() failed"); 
        exit(-1);
    }

    // 超时。
    if (infds == 0){
        printf("epoll_wait() timeout.\n"); return evs;
    }
    // 如果infds>0，表示有事件发生的fd的数量。
    for (int ii=0;ii<infds;ii++) {      // 遍历epoll返回的数组evs。
        evs.push_back(events_[ii]);
    }
    return evs;

}
*/


std::vector<Channel*>Epoll::loop(int timeout){//运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回
    std::vector<Channel*>channels;//存放epoll_wait()返回的事件
    bzero(events_, sizeof(events_));
    int infds=epoll_wait(epollfd_,events_,MaxEvents,timeout);       // 等待监视的fd有事件发生。

    // 返回失败。
    if (infds < 0){
        perror("epoll_wait() failed"); 
        exit(-1);
        //EBADF：epfd不是一个有效的描述符
        //EFAULT:参数events指向的内存区域不可写
        //EINVAL:epfd不是一个epoll文件描述符，或者参数maxevents小于等于0
        //EINTR：阻塞过程中被信号中断，epoll_wait()可以避免或者错误处理中，解析error后重新调用epoll_wait()
        //在Reactor中，不建议使用信号，因为信号处理起来很麻烦，没有必要
    }

    // 超时。
    if (infds == 0){
        //如果epoll_wait()超时，说明系统空闲，返回的channels将为空
        //printf("epoll_wait() timeout.\n"); 
        return channels;
    }
    // 如果infds>0，表示有事件发生的fd的数量。
    for (int ii=0;ii<infds;ii++) {      // 遍历epoll返回的数组evs。
        Channel *ch = (Channel*)events_[ii].data.ptr;
        ch -> setrevents(events_[ii].events);
        channels.push_back(ch);
    }
    return channels;

}
