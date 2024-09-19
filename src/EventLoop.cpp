#include "EventLoop.h"
namespace reactor {
int createtimerfd(int sec = 30) {
  int tfd = timerfd_create(CLOCK_MONOTONIC,
                           TFD_CLOEXEC | TFD_NONBLOCK);  // 创建timerfd。
  struct itimerspec timeout;  // 定时时间的数据结构。
  memset(&timeout, 0, sizeof(struct itimerspec));
  timeout.it_value.tv_sec = sec;  // 定时时间为5秒。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(tfd, 0, &timeout, 0);  // 开始计时。alarm(5)
  return tfd;
}
EventLoop::EventLoop(bool mainloop, int timetvl, int timeout)
    : ep_(new Epoll),
      mainloop_(mainloop),
      wakeupfd_(eventfd(0, EFD_NONBLOCK)),
      wakechannel_(new Channel(this, wakeupfd_)),
      timerfd_(createtimerfd(timeout_)),
      timerchannel_(new Channel(this, timerfd_)),
      timetvl_(timetvl),
      timeout_(timeout),
      stop_(false) {
  wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup, this));
  wakechannel_->enablereading();
  timerchannel_->setreadcallback(std::bind(&EventLoop::handletime, this));
  timerchannel_->enablereading();
}
EventLoop::~EventLoop() {}

void EventLoop::run() {
  threadid_ = syscall(SYS_gettid);  //获取事件循环所在id
  while (stop_ == false)            // 事件循环。
  {
    std::vector<Channel *> channels =
        ep_->loop(10 * 1000);  //存放epoll_wait()返回事件
    //如果channels为空，表示超时，回调TcpServer::epolltimeout()
    if (channels.size() == 0) {
      epolltimeoutcallback_(this);
    } else {
      for (auto &ch : channels) {
        ch->handleevent();
      }
    }
  }
}

void EventLoop::stop() {
  stop_ = true;
  wakeup();  //唤醒事件循环，如果不加这行，事件循环将在下次闹钟响的时候或者epoll_wait()超时才会停下来
}

void EventLoop::updatechannel(Channel *ch) { ep_->updatechannel(ch); }

void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop *)> fn) {
  epolltimeoutcallback_ = fn;
}

void EventLoop::removechannel(Channel *ch) { ep_->removechannel(ch); }

bool EventLoop::isinloopthread() {
  return threadid_ == syscall(SYS_gettid);
}  //判断当前线程是否为事件循环线程bool

//把任务添加到队列中
void EventLoop::queueinloop(std::function<void()> fn) {
  {
    std::lock_guard<std::mutex> gd(mutex_);  //给任务队列加锁
    taskqueue_.push(fn);                     //任务入队
  }
  wakeup();  //唤醒事件循环
}
//用eventfd唤醒事件循环
void EventLoop::wakeup() {
  uint64_t val = 1;
  write(wakeupfd_, &val, sizeof(val));
}

void EventLoop::handlewakeup() {
  uint64_t val;
  read(wakeupfd_, &val,
       sizeof(val));  //从eventfd中读取数据，如果不读取，eventfd读事件会一直触发
  std::function<void()> fn;
  std::lock_guard<std::mutex> gd(mutex_);  //给任务队列加锁

  while (taskqueue_.size() > 0) {
    fn = std::move(taskqueue_.front());  //出队一个元素
    taskqueue_.pop();
    fn();  //执行
  }
}

void EventLoop::handletime() {
  //闹钟响时开始执行
  struct itimerspec timeout;  // 定时时间的数据结构。
  memset(&timeout, 0, sizeof(struct itimerspec));
  timeout.it_value.tv_sec = timetvl_;  // 定时时间为5秒。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(timerfd_, 0, &timeout, 0);  // 开始计时。alarm(5)
  if (mainloop_) {
  } else {
    time_t now = time(0);
    for (auto it = conns_.begin(); it != conns_.end();) {
      // printf("%d",it->first);
      if (it->second->timeout(now, timeout_)) {
        timercallback_(it->first);
        std::lock_guard<std::mutex> gd(mmutex_);
        it = conns_.erase(it);  //从EventLoop的map中删除超时的conn
      } else
        it++;
    }
  }
}

void EventLoop::newconnection(spConnection conn) {
  {
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_[conn->fd()] = conn;
  }

}  //把connection对象保存在conns_中

void EventLoop::settimercallback(std::function<void(int)> fn) {
  timercallback_ = fn;
}
}  // namespace reactor