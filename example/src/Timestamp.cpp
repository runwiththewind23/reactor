#include<time.h>

#include"Timestamp.h"

/*
class Timestamp{
    private:
        time_t secsinceepoch_;//整数表示的时间（1970到现在已逝去的秒数）
    public:
        Timestamp();//用当前时间初始化对象
        Timestamp(int64_t secinceepoch_);//用一个整数表示的时间初始化对象

        static Timestamp now();//返回当前时间的Timmestamp对象

        time_t toint()const;//返回整数表示的时间
        std::string tostring()const;//返回字符串表示的时间，格式yyyy-mm-dd hh24:mi:ss


};*/

Timestamp::Timestamp(){
    secsinceepoch_=time(0);//取系统当前时间
}

Timestamp::Timestamp(int64_t secinceepoch_){

}

Timestamp Timestamp::now(){
    return Timestamp();//返回当前时间
}

time_t Timestamp::toint()const{
    return secsinceepoch_;
}

std::string Timestamp::tostring()const{
    char buf[128] = {0};
    tm *tm_time = localtime(&secsinceepoch_);
    snprintf(buf, 128, "%4d-%02d-%02d %02d:%02d:%02d",
    tm_time->tm_year+1900,
    tm_time->tm_mon+1,
    tm_time->tm_mday,
    tm_time->tm_hour,
    tm_time->tm_min,
    tm_time->tm_sec);
    return buf;
}
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