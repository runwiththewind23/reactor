/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-24 14:15:40
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-31 16:37:43
 * @FilePath: /sxd/reactor/reactor/39/include/Epoll.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <vector>

#include "Channel.h"
namespace reactor {
class Channel;
// Epoll类
class Epoll {
 private:
  static const int MaxEvents = 100;  // epoll_wait()返回事件数组的大小
  int epollfd_ = -1;                 // epoll句柄，在构造函数中创建
  epoll_event events_
      [MaxEvents];  //存放epoll_wait()返回事件的数组，在构造函数中分配内存
 public:
  Epoll();   //在构造函数中创建epollfd_
  ~Epoll();  //在析构函数中关闭epollfd_

  // void addfd(int fd, uint32_t op);//吧fd和它需要监视的事件添加到红黑树
  void removechannel(Channel *ch);  //从红黑树中删除Channel
  void updatechannel(
      Channel
          *ch);  //把channel添加/更新到红黑树上，channel有fd,也有需要监视的事件
  std::vector<Channel *> loop(
      int timeout =
          -1);  //运行epoll_wait()，等待事件的发生，已发生的事件用vector返回
  // std::vector<epoll_event>loop(int timeout =
  // -1);//运行epoll_wait()，等待事件的发生，已发生的事件用vector返回
};
}  // namespace reactor
