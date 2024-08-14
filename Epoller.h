#pragma once
#include"Timestamp.h"
#include<sys/epoll.h>
#include"Eventloop.h"
#include<vector>
#include<unordered_map>
class Channel;
class Epoller
{
private:
    Eventloop* loop_;
    int epollfd_;
    static const int kInitEventlistSize=16;
    std::vector<epoll_event> events_;
    // fd:channel
    std::unordered_map<int,Channel*> channelsMap_;

    void update(Channel* channel,int op);
    using ChannelList=std::vector<Channel*>; 
    void fillActiveChannels(int num,ChannelList& activeChannels) const;
public:
    Epoller(Eventloop* loop);
    ~Epoller();

    Timestamp poll(int timeoutMs,ChannelList& activeChannels);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel) const;
};


