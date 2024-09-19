/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-24 14:15:40
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-31 16:31:09
 * @FilePath: /sxd/reactor/reactor/39/src/Buffer.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Buffer.h"
namespace reactor {
Buffer::Buffer(uint16_t sep) : sep_(sep) {}

Buffer::~Buffer() {}

void Buffer::append(const char* data, size_t size) {
  buf_.append(data, size);
}  // 把数据追加到buf_中
size_t Buffer::size() { return buf_.size(); }       // 返回buf_的大小
const char* Buffer::data() { return buf_.data(); }  // 返回buf_的首地址
void Buffer::clear() { buf_.clear(); }              // 清空buf_

void Buffer::erase(size_t pos, size_t nn) { buf_.erase(pos, nn); }
void Buffer::appendwithsep(const char* data, size_t size) {
  if (sep_ == 0)
    buf_.append(data, size);
  else if (sep_ == 1) {
    buf_.append((char*)&size, 4);
    buf_.append(data, size);
  }

}  // 把数据追加到buf_中,附加报文头部

bool Buffer::pickmessage(std::string& ss) {
  if (buf_.size() == 0) return false;

  if (sep_ == 0) {  // 没有分隔符
    ss = buf_;
    buf_.clear();
  } else if (sep_ == 1) {  // 四字节的报头
    int len;
    memcpy(&len, buf_.data(), 4);
    if (buf_.size() < len + 4) return false;
    ss = buf_.substr(4, len);
    buf_.erase(0, len + 4);
  }
  return true;
}  // 从buf_中拆分出一个报文，存放在ss中，如果buf_没有报文，返回false
}  // namespace reactor