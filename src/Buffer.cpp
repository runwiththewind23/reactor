#include "Buffer.h"

/*class Buffer{
    private:
        std::string buf_; //用于存放数据
    public:
        Buffer();
        ~Buffer();
        void append(const char *data, size_t size); //把数据追加到buf_中
        size_t size(); //返回buf_的大小
        const char *data(); //返回buf_的首地址
        void clear(); //清空buf_
};*/

Buffer::Buffer(uint16_t sep):sep_(sep){

}

Buffer::~Buffer(){

}

void Buffer::append(const char *data, size_t size){
    buf_.append(data,size);
} //把数据追加到buf_中
size_t Buffer::size(){
    return buf_.size();
} //返回buf_的大小
const char* Buffer::data(){
    return buf_.data();
} //返回buf_的首地址
void Buffer::clear(){
    buf_.clear();
} //清空buf_

void Buffer::erase(size_t pos, size_t nn){
    buf_.erase(pos,nn);
}
void Buffer::appendwithsep(const char* data, size_t size){
    if(sep_==0) buf_.append(data,size);
    else if(sep_==1){
        buf_.append((char*)&size, 4);
        buf_.append(data,size);
    }

}//把数据追加到buf_中,附加报文头部

bool Buffer::pickmessage(std::string &ss){
    if(buf_.size()==0)return false;

    if(sep_==0){//没有分隔符
        ss=buf_;
        buf_.clear();
    }else if(sep_==1){//四字节的报头
        int len;
        memcpy(&len,buf_.data(),4);
        if(buf_.size()<len+4) return false; 
        ss= buf_.substr(4,len);
        buf_.erase(0,len+4);
    }
    return true;
}//从buf_中拆分出一个报文，存放在ss中，如果buf_没有报文，返回false