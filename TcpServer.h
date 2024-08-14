#pragma once
#include "Eventloop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventloopThreadPool.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "Buffer.h"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

class TcpServer:noncopyable
{
private:
    Eventloop* loop_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventloopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_; // 有新连接时的回调
    MessageCallback messageCallback_; // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调

    ThreadInitCallback threadInitCallback_; // loop线程初始化的回调

    std::atomic_int started_;
    int nextConnId_;
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    ConnectionMap connMaps_;

    void newConnection(int connfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);
public:
    enum Option{
        KNoReusePort,
        KReusePort,
    };
    TcpServer(Eventloop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            Option option=KReusePort);
    ~TcpServer();

    void setThreadInitcallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void setThreadNum(int numThreads){threadPool_->setThreadNum(numThreads);}
    void start();
};


