#include "BankServer.h"

BankServer::BankServer(const std::string& ip, const uint16_t port,
                       int subthreadnum, int workthreadnum)
    : tcpserver_(ip, port, subthreadnum), threadpool_(workthreadnum, "WORKS") {
  tcpserver_.setnewconnnectioncb(
      std::bind(&BankServer::HandleNewConnection, this, std::placeholders::_1));
  tcpserver_.setcloseconnnectioncb(
      std::bind(&BankServer::HandleClose, this, std::placeholders::_1));
  tcpserver_.seterrorconnnectioncb(
      std::bind(&BankServer::HandleError, this, std::placeholders::_1));
  tcpserver_.setonmessagecb(std::bind(&BankServer::HandleMessage, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
  tcpserver_.setsendcompletecb(
      std::bind(&BankServer::HandleSendComplete, this, std::placeholders::_1));
  // tcpserver_.settimeoutcb(std::bind(&BankServer::HandleTimeout, this
  // ,std::placeholders::_1));
  tcpserver_.setremoveconncb(
      std::bind(&BankServer::HandleRemove, this, std::placeholders::_1));
}

BankServer::~BankServer() {}
void BankServer::start() { tcpserver_.start(); }

void BankServer::stop() {
  //停止工作线程
  threadpool_.stop();
  printf("工作线程已停止。\n");
  //停止IO线程(事件循环)
  tcpserver_.stop();
}

void BankServer::HandleNewConnection(spConnection conn) {
  spUserInfo userinfo(new UserInfo(conn->fd(), conn->ip()));
  {
    std::unique_lock<std::mutex> lock(mutex_);
    users_[conn->fd()] = userinfo;  //用户添加到状态机中
  }
  printf("%s New connection(fd=%d, ip=%s) ok.\n",
         Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str());
}  //处理新客户端连接请求

void BankServer::HandleClose(spConnection conn) {
  //从状态机中删除用户
  printf("%s connection(fd=%d, ip=%s) closed.\n",
         Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str());
  {
    std::unique_lock<std::mutex> lock(mutex_);
    users_.erase(conn->fd());
  }
}  //关闭客户端的连接

void BankServer::HandleError(spConnection conn) {
  HandleClose(conn);  //关闭客户端的连接
}  //客户端的错误连接

void BankServer::HandleMessage(spConnection conn, std::string& message) {
  //把业务添加到线程池中
  if (threadpool_.size() == 0) {
    //如果没有工作线程，表示在IO线程中计算
    OnMessage(conn, message);
  } else
    threadpool_.addtask(std::bind(&BankServer::OnMessage, this, conn, message));
}  //处理客户端的请求报文，在Tcpserver类中回调此函数

void BankServer::HandleSendComplete(spConnection conn) {
  // std::cout<<"message send complete."<<std::endl;
}  //数据发送完成后，在Tcpserver中回调
void BankServer::HandleTimeout(EventLoop* loop) {
  std::cout << "EchoServer timeout." << std::endl;
}  // epoll_wait()超时，在Tcpserver类中回调

bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname,
                  std::string& value, const int ilen = 0) {
  // 使用 string_view 避免不必要的拷贝
  std::string_view xml_view(xmlbuffer);
  std::string start = "<" + fieldname + ">";
  std::string end = "</" + fieldname + ">";

  // 查找起始和结束标签
  auto start_pos = xml_view.find(start);
  if (start_pos == std::string_view::npos) return false;

  auto end_pos = xml_view.find(end, start_pos);
  if (end_pos == std::string_view::npos) return false;

  // 提取字段值
  start_pos += start.size();
  auto field_length = end_pos - start_pos;
  if (ilen > 0 && ilen < static_cast<int>(field_length)) {
    field_length = ilen;
  }
  value = std::string(xml_view.substr(start_pos, field_length));

  return true;
}
void BankServer::OnMessage(spConnection conn, std::string& message) {
  spUserInfo userinfo = users_[conn->fd()];  //从客户端状态机获取客户端信息
  // 解析xml报文，处理各种业务
  std::string bizcode;
  std::string replymessage;
  getxmlbuffer(message, "bizcode", bizcode);  //从报文中解析业务代码
  if (bizcode == "00101") {
    std::string username, passwd;
    getxmlbuffer(message, "username", username);
    getxmlbuffer(message, "passwd", passwd);
    std::cout << "username: " << username << ", passwd: " << passwd
              << std::endl;
    if (username == "admin" && passwd == "123456") {
      //用户名和密码准确
      replymessage =
          "<bizcode>00102</bizcode><retcode>0</retcode><message>ok</message>";
      userinfo->setLogin(true);  //设置登录状态为true
    } else {
      replymessage =
          "<bizcode>00102</bizcode><retcode>-1</"
          "retcode><message>用户名或密码不正确。</message>";
    }
    if (userinfo->isLogin() == true) {
      printf("用户%d已登录\n", conn->fd());
    } else {
      printf("用户%d未登录\n", conn->fd());
    }
  } else if (bizcode == "00201")  // 查询余额业务。
  {
    if (userinfo->isLogin() == true) {
      // 把用户的余额从数据库或Redis中查询出来。
      replymessage =
          "<bizcode>00202</bizcode><retcode>0</retcode><message>5080.80</"
          "message>";
    } else {
      replymessage =
          "<bizcode>00202</bizcode><retcode>-1</retcode><message>用户未登录。</"
          "message>";
    }
  } else if (bizcode == "00901")  // 注销业务。
  {
    if (userinfo->isLogin() == true) {
      replymessage =
          "<bizcode>00902</bizcode><retcode>0</retcode><message>ok</message>";
      userinfo->setLogin(false);  // 设置用户的登录状态为false。
    } else {
      replymessage =
          "<bizcode>00902</bizcode><retcode>-1</retcode><message>用户未登录。</"
          "message>";
    }
  } else if (bizcode == "00001")  // 心跳。
  {
    if (userinfo->isLogin() == true) {
      replymessage =
          "<bizcode>00002</bizcode><retcode>0</retcode><message>ok</message>";
    } else {
      replymessage =
          "<bizcode>00002</bizcode><retcode>-1</retcode><message>用户未登录。</"
          "message>";
    }
  }

  conn->send(replymessage.data(), replymessage.size());  // 把数据发送出去。
}

/*
// epoll_wait()超时，在TcpServer类中回调此函数。
void BankServer::HandleTimeOut(EventLoop *loop)
{
    std::cout << "BankServer timeout." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}
*/

void BankServer::HandleRemove(
    int fd)  // 客户端的连接超时，在TcpServer类中回调此函数。
{
  printf("fd(%d) 已超时。\n", fd);

  std::lock_guard<std::mutex> gd(mutex_);
  users_.erase(fd);  // 从状态机中删除用户信息。
}

// printf("处理完业务后使用connection对象。\n");
// conn->send(replymessage.data(),replymessage.size());//把数据发送出去,使用野指针后果未知//处理客户端的请求报文，用于添加给线程池
