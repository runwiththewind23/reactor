/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-24 14:15:40
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-31 16:32:59
 * @FilePath: /sxd/reactor/reactor/39/src/Epoll.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Epoll.h"
namespace reactor {
Epoll::Epoll() {                             //在构造函数在创建epollfd_
  if ((epollfd_ = epoll_create(1)) == -1) {  //创建epoll句柄（红黑树）
    printf("epoll_create() failed(%d).\n", errno);
    exit(-1);
  }
}
Epoll::~Epoll() {  //在析构函数中关闭
  close(epollfd_);
}

void Epoll::updatechannel(Channel *ch) {
  epoll_event ev;  //声明结构体
  ev.data.ptr = ch;
  ev.events = ch->events();  //指定事件

  if (ch->inpoll()) {  //如果已经在树上了
    if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1) {
      perror("epoll_ctl() failed.\n");
      exit(-1);
    }
  } else {  //如果不在树上
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1) {
      perror("epoll_ctl() failed.\n");
      exit(-1);
    }
    ch->setinepoll();  //把channel的inepoll_成员设置为true;
  }
}

void Epoll::removechannel(Channel *ch) {
  if (ch->inpoll()) {  //如果已经在树上了
    printf("removechannel.\n");
    if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, ch->fd(), 0) == -1) {
      perror("epoll_ctl() failed.\n");
      exit(-1);
    }
  }
}

std::vector<Channel *> Epoll::loop(
    int timeout) {  //运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回
  std::vector<Channel *> channels;  //存放epoll_wait()返回的事件
  bzero(events_, sizeof(events_));
  int infds = epoll_wait(epollfd_, events_, MaxEvents,
                         timeout);  // 等待监视的fd有事件发生。

  // 返回失败。
  if (infds < 0) {
    perror("epoll_wait() failed");
    exit(-1);
    // EBADF：epfd不是一个有效的描述符
    // EFAULT:参数events指向的内存区域不可写
    // EINVAL:epfd不是一个epoll文件描述符，或者参数maxevents小于等于0
    // EINTR：阻塞过程中被信号中断，epoll_wait()可以避免或者错误处理中，解析error后重新调用epoll_wait()
    //在Reactor中，不建议使用信号，因为信号处理起来很麻烦，没有必要
  }

  // 超时。
  if (infds == 0) {
    //如果epoll_wait()超时，说明系统空闲，返回的channels将为空
    // printf("epoll_wait() timeout.\n");
    return channels;
  }
  // 如果infds>0，表示有事件发生的fd的数量。
  for (int ii = 0; ii < infds; ii++) {  // 遍历epoll返回的数组evs。
    Channel *ch = (Channel *)events_[ii].data.ptr;
    ch->setrevents(events_[ii].events);
    channels.push_back(ch);
  }
  return channels;
}
}  // namespace reactor
