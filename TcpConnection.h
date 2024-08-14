#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"
#include "Eventloop.h"
#include "Socket.h"
#include "Channel.h"
#include <memory>
#include <string>
#include <atomic>

/**
 * TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
 * => TcpConnection 设置回调 =>   Channel的回调操作
 *
 */
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
private:
    Eventloop *loop_;
    const std::string name_;
    // 这里和Acceptor类似   Acceptor=>mainLoop    TcpConenction=>subLoop
    // 但是这里用unique_ptr管理
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;       // 有新连接时的回调
    MessageCallback messageCallback_;             // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;

    Buffer inputBuffer_;  // 接收数据的缓冲区
    Buffer outputBuffer_; // 发送数据的缓冲区

    enum State{kDisconnected,kConnecting,kConnected,kDisconnecting};
    std::atomic_int state_;
    void setState(State state){state_=state;}
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
public:
    // loop的所有权由Pool管理
    TcpConnection(Eventloop *loop,
                  const std::string &name,
                  int connfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr);
    ~TcpConnection();

    Eventloop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddress() const { return localAddr_; }
    const InetAddress &peerAddress() const { return peerAddr_; }

    //三步走，并且像shutdown需要等待数据发送完，故需要State
    //同时，外部调用者也需要State
    void establish();
    void shutdown();//关闭写
    void shutdownInLoop();
    void destroy();//关闭读
    
    bool connected() const { return state_ == kConnected; }
    void send(const std::string &buf);
    void sendInLoop(const void *data, size_t len);

    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback &cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    void setCloseCallback(const CloseCallback &cb)
    {
        closeCallback_ = cb;
    }
};
