#include "Buffer.h"
#include"errno.h"
#include <sys/uio.h>
#include <unistd.h>
Buffer::Buffer(size_t initialSize)
    :buffer_(kCheapPreprend+initialSize)
    ,readIndex_(kCheapPreprend)
    ,writeIndex_(kCheapPreprend)
{

}

ssize_t Buffer::readFd(int fd, int& saveError)
{
    char extrabuf[65536]={0};
    iovec vec[2];
    const size_t writable=writableBytes();
    vec[0].iov_base=begin()+writeIndex_;
    vec[0].iov_len=writable;
    vec[1].iov_base=extrabuf;
    vec[1].iov_len=sizeof extrabuf;
    const int cnt=(writable<sizeof extrabuf)?2:1;
    const size_t n=::readv(fd,vec,cnt);
    if(n<0){
        saveError=errno;
    }
    else if(n<=writable){
        writeIndex_+=n;
    }
    else{
        writeIndex_=buffer_.size();
        append(extrabuf,n-writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int &saveError)
{
    ssize_t n=::write(fd,peek(),readableBytes());
    if(n<0){
        saveError=errno;
    }
    return n;
}
