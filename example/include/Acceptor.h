/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-28 19:06:18
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-31 17:00:21
 * @FilePath: /sxd/reactor/reactor/Reactor/example/include/Acceptor.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <functional>
#include <memory>

#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
class Acceptor {
 private:
  EventLoop *loop_;  // Acceptor对应的事件循环，在构造函数中传入
  Socket servsock_;  //服务端用于监听的socket，在构造函数中创建
  Channel acceptchannel_;  // Acceptor对应的channel，在构造函数中创建
  std::function<void(std::unique_ptr<Socket>)>
      newconnectioncb_;  //处理新客户端连接请求的回调函数，指向TCPServer::newconnnection()
 public:
  Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port);
  void newconnection();
  ~Acceptor();

  void setnewconnnectioncb(std::function<void(std::unique_ptr<Socket>)>
                               fn);  //设置新客户端连接请求的回调函数
};