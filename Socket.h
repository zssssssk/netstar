#pragma once
#include"noncopyable.h"
#include"InetAddress.h"

class Socket:noncopyable
{
private:
    const int sockfd_;
public:
    explicit Socket(int sockfd):sockfd_(sockfd){}
    ~Socket();
    int fd() const{return sockfd_;}
    void bindAddress(const InetAddress& localAddr);
    void listen();
    int accept(InetAddress& peerAddr);
    void shutdownWrite();
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
};


