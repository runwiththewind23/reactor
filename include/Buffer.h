/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-24 14:15:40
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-31 16:35:50
 * @FilePath: /sxd/reactor/reactor/39/include/Buffer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <cstring>
#include <iostream>
#include <string>
namespace reactor {
class Buffer {
 private:
  std::string buf_;  //用于存放数据
  const uint16_t
      sep_;  //报文的分隔符，0-无分隔符(固定长度，视频会议)，1-四字节的包头，2."\r\n\r\n"分隔符（http协议）
 public:
  Buffer(uint16_t sep = 1);
  ~Buffer();
  void append(const char *data, size_t size);  //把数据追加到buf_中
  void appendwithsep(const char *data,
                     size_t size);  //把数据追加到buf_中,附加报文头部
  void erase(size_t pos, size_t nn);
  size_t size();       //返回buf_的大小
  const char *data();  //返回buf_的首地址
  void clear();        //清空buf_

  bool pickmessage(
      std::string &
          ss);  //从buf_中拆分出一个报文，存放在ss中，如果buf_没有报文，返回false
};
}  // namespace reactor