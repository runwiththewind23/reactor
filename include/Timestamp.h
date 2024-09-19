/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-24 14:15:40
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-31 16:39:19
 * @FilePath: /sxd/reactor/reactor/39/include/Timestamp.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <iostream>
#include <string>
namespace reactor {
//时间戳
class Timestamp {
 private:
  time_t secsinceepoch_;  //整数表示的时间（1970到现在已逝去的秒数）
 public:
  Timestamp();                       //用当前时间初始化对象
  Timestamp(int64_t secinceepoch_);  //用一个整数表示的时间初始化对象

  static Timestamp now();  //返回当前时间的Timestamp对象

  time_t toint() const;  //返回整数表示的时间
  std::string tostring()
      const;  //返回字符串表示的时间，格式yyyy-mm-dd hh24:mi:ss
};
}  // namespace reactor