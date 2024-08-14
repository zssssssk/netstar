#include "TcpServer.h"
#include"Logger.h"
#include "TcpConnection.h"

#include <strings.h>
#include <functional>

static Eventloop* CheckLoopNotNull(Eventloop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

void TcpServer::newConnection(int connfd, const InetAddress &peerAddr)
{
    Eventloop* ioLoop=threadPool_->getNextloop();
    char buf[64]={0};
    snprintf(buf,sizeof buf,"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName=name_+buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
        name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
    
    // 通过connfd获取其绑定的本机的ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local,sizeof local);
    socklen_t addrlen=sizeof local;
    if(::getsockname(connfd,(sockaddr*)&local,&addrlen)<0){
        LOG_ERROR("sockets::getLocalAddr");
    }
    InetAddress localAddr(local);
    // 根据连接成功的connfd，创建TcpConnection连接对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,connfd,localAddr,peerAddr));
    connMaps_[connName]=conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
    ioLoop->runInloop(std::bind(&TcpConnection::establish,conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInloop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n", 
        name_.c_str(), conn->name().c_str());
    connMaps_.erase(conn->name());
    Eventloop* ioLoop=conn->getLoop();
    ioLoop->runInloop(std::bind(&TcpConnection::destroy,conn));
}

TcpServer::TcpServer(Eventloop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option)
    : loop_(CheckLoopNotNull(loop)), ipPort_(listenAddr.toIpPort()), name_(nameArg), acceptor_(new Acceptor(loop, listenAddr,KNoReusePort)), threadPool_(new EventloopThreadPool(loop, nameArg)), nextConnId_(1), started_(0)
{
    LOG_INFO("acceptor_:fd:%d",acceptor_->acceptChannelptr()->fd());
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for(auto& [name,conn]:connMaps_){
        conn->getLoop()->runInloop(std::bind(&TcpConnection::destroy,conn));
    }
    connMaps_.clear();
}

void TcpServer::start()
{
    if(started_++==0){// 防止一个TcpServer对象被start多次
        //LOG_INFO("acceptor_:fd:%d",acceptor_->acceptChannelptr()->fd());
        threadPool_->start(threadInitCallback_);
        //LOG_INFO("acceptor_:fd:%d",acceptor_->acceptChannelptr()->fd());
        loop_->runInloop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}
