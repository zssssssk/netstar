#pragma once
#include<memory>
#include<vector>
#include<functional>
#include<mutex>
#include<atomic>
#include"CurrentThread.h"
class Channel;
class Epoller;

class Eventloop
{
private:
    int wakeupfd_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::unique_ptr<Epoller> epoller_;

    std::vector<Channel*> activeChannels_;

    using Functor=std::function<void()>;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;
    std::atomic_bool callingPendingFunctor;
    
    std::atomic_bool looping_;
    pid_t threadId_;

    void handleRead();//wakeup
    void doPendingFunctor();
public:
    Eventloop();
    ~Eventloop();

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return threadId_ ==  CurrentThread::tid(); }
    
    void wakeup();

    void loop();
    void quit();
    void runInloop(Functor cb);
    void queueInloop(Functor cb);
};

