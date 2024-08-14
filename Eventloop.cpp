#include"Eventloop.h"
#include"Logger.h"
#include<sys/eventfd.h>
#include"Epoller.h"
#include"Channel.h"
#include"CurrentThread.h"
#include<memory>
__thread Eventloop* thisThread=nullptr;
const int timeoutMs=10000;

int creatEventfd(){
    int wakeupfd=::eventfd(0,EFD_CLOEXEC|EFD_NONBLOCK);
    if(wakeupfd<0){
        LOG_FATAL("wakeupfd create fail!");
    }
    return wakeupfd;
}

Eventloop::Eventloop() : looping_(false),
                         wakeupfd_(creatEventfd()),
                         wakeupChannel_(new Channel(this, wakeupfd_)),
                         epoller_(new Epoller(this)),
                         callingPendingFunctor(false),
                         threadId_(CurrentThread::tid())
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if(thisThread==nullptr){
        thisThread=this;
    }
    else{
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", thisThread, threadId_);
    }

    wakeupChannel_->setReadCallback(std::bind(&Eventloop::handleRead,this));
    wakeupChannel_->enableReading();
}

Eventloop::~Eventloop()
{
}

void Eventloop::handleRead()
{
    uint64_t c='1';
    auto n=read(wakeupfd_,&c,sizeof c);
    if(n!=sizeof c){
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 1", n);
    }
}

void Eventloop::wakeup()
{
    uint64_t c='1';
    auto n=write(wakeupfd_,&c,sizeof c);
    if(n!=sizeof c){
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 1", n);
    }
}

void Eventloop::loop()
{
    looping_=true;
    while(looping_){
        activeChannels_.clear();
        auto time=epoller_->poll(timeoutMs,activeChannels_);
        for(auto& channel:activeChannels_){
            channel->handleEvent(time);
        }
        doPendingFunctor();
    }
}

void Eventloop::quit()
{
    looping_=false;
    if(!isInLoopThread()){
        wakeup();
    }
}

void Eventloop::runInloop(Functor cb)
{
    if(isInLoopThread()){
        cb();
    }
    else{
        queueInloop(cb);
    }
}

void Eventloop::queueInloop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    wakeup();
}

void Eventloop::doPendingFunctor()
{
    std::vector<Functor> t;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        t.swap(pendingFunctors_);
    }
    //不用专门wakeup(),因为外面一定会去wakeup(),唤醒之后自然会执行该方法

    callingPendingFunctor=true;
    for(auto& f:t){
        f();
    }
    callingPendingFunctor=false;
}

void Eventloop::updateChannel(Channel *channel)
{
    epoller_->updateChannel(channel);
}

void Eventloop::removeChannel(Channel *channel)
{
    epoller_->removeChannel(channel);
}

bool Eventloop::hasChannel(Channel *channel)
{
    return epoller_->hasChannel(channel);
}

