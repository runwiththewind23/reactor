/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-24 14:15:40
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-31 16:30:52
 * @FilePath: /sxd/reactor/reactor/39/src/Acceptor.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Acceptor.h"

#include "Connection.h"
namespace reactor {

Acceptor::Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port)
    : loop_(loop),
      servsock_(createnoblocking()),
      acceptchannel_(loop_, servsock_.fd()) {
  // servsock_ = new Socket(createnoblocking());
  InetAddress servaddr(ip, port);  //服务端的地址和协议
  servsock_.setreuseaddr(true);
  servsock_.setreuseport(true);
  servsock_.setkeepalive(true);
  servsock_.settcpnodelay(true);
  servsock_.bind(servaddr);
  servsock_.listen();

  // acceptchannel_ = new Channel(loop_, servsock_.fd());
  acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection, this));
  acceptchannel_.enablereading();
}

Acceptor::~Acceptor() {
  // delete servsock_;
  // delete acceptchannel_;
}

void Acceptor::newconnection() {  //处理新客户端连接请求
  InetAddress clientaddr;         // 客户端的地址和协议。
  std::unique_ptr<Socket> clientsock(new Socket(servsock_.accept(clientaddr)));
  clientsock->setipport(clientaddr.ip(), clientaddr.port());
  /*
   注意：clientsock只能new出来，不能在栈上，否则析构函数会关闭fd
   还有new出来的对象还没释放
  */
  // printf ("accept client(fd=%d,ip=%s,port=%d)
  // ok.\n",clientsock->fd(),clientaddr.ip(),clientaddr.port());
  newconnectioncb_(std::move(clientsock));  //回调了TCPserver::newconnection（）
}

void Acceptor::setnewconnnectioncb(
    std::function<void(std::unique_ptr<Socket>)> fn) {
  newconnectioncb_ = fn;
}
}  // namespace reactor