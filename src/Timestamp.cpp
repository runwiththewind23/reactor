/*
 * @Author: runwiththewind23 19945243142@163.com
 * @Date: 2024-07-24 14:15:40
 * @LastEditors: runwiththewind23 19945243142@163.com
 * @LastEditTime: 2024-07-31 16:35:08
 * @FilePath: /sxd/reactor/reactor/39/src/Timestamp.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Timestamp.h"

#include <time.h>

namespace reactor {
Timestamp::Timestamp() {
  secsinceepoch_ = time(0);  //取系统当前时间
}

Timestamp::Timestamp(int64_t secinceepoch_) {}

Timestamp Timestamp::now() {
  return Timestamp();  //返回当前时间
}

time_t Timestamp::toint() const { return secsinceepoch_; }

std::string Timestamp::tostring() const {
  char buf[128] = {0};
  tm *tm_time = localtime(&secsinceepoch_);
  snprintf(buf, 128, "%4d-%02d-%02d %02d:%02d:%02d", tm_time->tm_year + 1900,
           tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_hour,
           tm_time->tm_min, tm_time->tm_sec);
  return buf;
}
}  // namespace reactor
   /*
   #include<unistd.h>
   
   #include<iostream>
   
   int main(){
       Timestamp ts;
       std::cout<<ts.toint()<<std::endl;
       std::cout<<ts.tostring()<<std::endl;
   
       sleep(1);
       std::cout<<Timestamp::now().toint()<<std::endl;
       std::cout<<Timestamp::now().tostring()<<std::endl;
   
   }
   //g++ -o test Timestamp.cpp
   */