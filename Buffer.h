#pragma once
#include<vector>
#include <string>
class Buffer
{
private:
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;//writerIndex_后面为空
    
    char* begin(){
        return &*buffer_.begin();
    }
    const char* begin() const{
        return &*buffer_.begin();
    }
    //for inline
    void makeSpace(size_t len){
        if(writableBytes()+prependableBytes()<len+kCheapPreprend){
            buffer_.resize(writeIndex_+len);
        }
        else{
            size_t n=readableBytes();
            std::copy(begin()+readIndex_,
                    begin()+writeIndex_,
                    begin()+kCheapPreprend);
            readIndex_=kCheapPreprend;
            writeIndex_=n;
        }
    }
public:
    static const size_t kCheapPreprend=8;
    static const size_t kInitialSize=1024;

    explicit Buffer(size_t initialSize=kCheapPreprend);
    ~Buffer()=default;

    size_t readableBytes() const{
        return writeIndex_-readIndex_;
    }
    size_t writableBytes() const{
        return buffer_.size()-writeIndex_;
    }
    size_t prependableBytes() const{
        return readIndex_;
    }
    //返回缓冲区中可读数据的起始地址
    const char* peek() const{
        return begin()+readIndex_;
    }
    //配合asString使用
    void retrieve(size_t len){
        if(len<readableBytes()){
            readIndex_+=len;
        }
        else{//len==readableBytes()
            retrieveAll();
        }
    }
    void retrieveAll(){
        readIndex_=writeIndex_=kCheapPreprend;
    }
    std::string retrieveAllasString(){
        return retrieveAsString(readableBytes());
    }
    std::string retrieveAsString(size_t len){
        std::string ans(peek(),len);
        retrieve(len);
        return ans;
    }
    void ensureWriteablebytes(size_t len){
        if(writableBytes()<len){
            makeSpace(len);
        }
    }
    // 把[data, data+len]内存上的数据，添加到writable缓冲区当中
    void append(char* data,size_t len){
        ensureWriteablebytes(len);
        std::copy(data,data+len,begin()+writeIndex_);
        writeIndex_+=len;
    }

    // 从fd上读取数据(参数里：引用，属性里：指针)
    ssize_t readFd(int fd,int& saveError);
    // 通过fd发送数据
    ssize_t writeFd(int fd,int& saveError);

};



