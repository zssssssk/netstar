#pragma once
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"
#include "Eventloop.h"

#include <functional>

class Acceptor : noncopyable
{
private:
    // Acceptor用的就是用户定义的那个baseLoop，也称作mainLoop
    Eventloop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    using NewConnectionCallback = std::function<void(int, const InetAddress &)>;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;

    void handleRead();

public:
    Acceptor(Eventloop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb){
        newConnectionCallback_=cb;
    }
    bool listenning() const{
        return listenning_;
    }
    void listen();
    Channel* acceptChannelptr(){return &acceptChannel_;}
};
