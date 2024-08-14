#include"Channel.h"
#include"Eventloop.h"
#include"Logger.h"
#include <sys/epoll.h>
#include"CurrentThread.h"
Channel::Channel(Eventloop *loop, int fd):
    loop_(loop),
    fd_(fd),
    tied_(false),
    events_(0),
    revents_(0)
{
    //LOG_INFO("%d,%d",CurrentThread::tid(),revents_);
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_=obj;
    tied_=true;
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    //为了在Channel::handleEvent处理期间，防止因其owner对象被修改，进而导致Channel被析构，最后出现不可预估错误。 
    //Channel::tie()的作用就是将Channel的owner对象进行绑定保护起来。
    if(tied_){
        auto guard=tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }
    else{
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d\n",revents_);
    if((revents_&EPOLLHUP)&!(revents_&EPOLLIN)){
        if(closeCallback_){
            closeCallback_();
        }
    }
    if(revents_&EPOLLERR){
        if(errorCallback_){
            errorCallback_();
        }
    }
    if(revents_&(EPOLLIN|EPOLLPRI)){
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }
    if(revents_&EPOLLOUT){
        if(writeCallback_){
            writeCallback_();
        }
    }
}


