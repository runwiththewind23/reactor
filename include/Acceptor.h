#pragma once
#include <functional>
#include <memory>

#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
namespace reactor {
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
}  // namespace reactor