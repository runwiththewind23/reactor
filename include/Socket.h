#pragma once
#include <errno.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "InetAddress.h"
namespace reactor {
//创建一个非阻塞的fd
int createnoblocking();
// socket类
class Socket {
 private:
  const int fd_;  //构造函数持有的fd，在构造函数中传进来
  std::string
      ip_;  //如果是listenfd存放服务端监听的ip，如果是客户端，存放对端的ip
  uint16_t
      port_;  //果是listenfd存放服务端监听的port，如果是客户端，存放对端的port
 public:
  Socket(int fd);  //构造函数，传入一个准备好的fd
  ~Socket();       //在析构函数中，关闭fd_

  int fd() const;                                        //返回fd成员
  std::string ip() const;                                //返回ip成员
  uint16_t port() const;                                 //返回port成员
  void setipport(const std::string& ip, uint16_t port);  //设置ip,port函数

  void setreuseaddr(bool on);   //设置SO_REUSEADDR
  void setreuseport(bool on);   //设置SO_REUSEPORT
  void settcpnodelay(bool on);  //设置TCP_NODELAY
  void setkeepalive(bool on);   //设置SO_KEEPALIVE

  void bind(const InetAddress& servaddr);
  void listen(int nn = 128);
  int accept(InetAddress& clientaddr);
};
}  // namespace reactor