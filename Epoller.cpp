#include "Epoller.h"
#include"Logger.h"
#include <unistd.h>
#include"Channel.h"
#include"CurrentThread.h"
#include <strings.h>

Epoller::Epoller(Eventloop *loop)
    : loop_(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventlistSize)
{
    if(epollfd_<0){
        LOG_FATAL("epoll_create1 error:%d\n",errno);
    }
}

Epoller::~Epoller()
{
    ::close(epollfd_);
}

Timestamp Epoller::poll(int timeoutMs, ChannelList &activeChannels)
{
    LOG_DEBUG("func=%s => fd total count:%lu\n",__FUNCTION__,channelsMap_.size());
    int numEvents=::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno=errno;
    Timestamp now=Timestamp::now();
    if(numEvents>0){
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents==events_.size()){
            events_.resize(2*numEvents);
        }
    }
    else if(numEvents==0){
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }
    else{
        if(saveErrno!=EINTR){
            errno=saveErrno;
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;
}


void Epoller::fillActiveChannels(int num, ChannelList &activeChannels) const
{
    for(int i=0;i<num;i++){
        Channel* channel=static_cast<Channel*>(events_[i].data.ptr);
        //events_[i].events字段里存放了真正发生的事件，给外面用的
        LOG_INFO("%d,%p",channel->revents(),channel);
        channel->set_revents(events_[i].events);
        activeChannels.push_back(channel);
    }
}

void Epoller::updateChannel(Channel *channel)
{
    LOG_INFO("%d\n", CurrentThread::tid());
    LOG_INFO("func=%s => fd=%d events=%d\n", __FUNCTION__, channel->fd(), channel->events());
    int fd=channel->fd();

    if(channelsMap_.count(fd)){
        if(channel->isNoneEvent()){
            channelsMap_.erase(fd);
            update(channel,EPOLL_CTL_DEL);
        }
        else{
            update(channel,EPOLL_CTL_MOD);
        }
    }
    else{
        if(channel->isReading()||channel->isWriting()){
            channelsMap_[fd]=channel;
            update(channel,EPOLL_CTL_ADD);
        }
    }
}

void Epoller::removeChannel(Channel *channel)
{
    int fd=channel->fd();
    if(channelsMap_.count(fd)){
        channelsMap_.erase(fd);
    }
    update(channel,EPOLL_CTL_DEL);
}

bool Epoller::hasChannel(Channel *channel) const
{
    return channelsMap_.count(channel->fd());
}

void Epoller::update(Channel *channel, int op)
{
    int fd=channel->fd();
    epoll_event event;
    bzero(&event,sizeof event);
    event.events=channel->events();
    event.data.ptr=static_cast<void*>(channel);
    //fd为需要进行操作的目标文件描述符，event是要去内部改成的东西
    if(::epoll_ctl(epollfd_,op,fd,&event)<0){
        if(op==EPOLL_CTL_DEL){
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else{
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}
