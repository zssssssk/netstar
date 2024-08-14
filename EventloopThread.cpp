#include "EventloopThread.h"
void EventloopThread::threadFunc()
{
    Eventloop loop;
    if(cb_){
        cb_(&loop);
    }
    loop_=&loop;
    sem_post(&sem_);
    loop.loop();

    loop_=nullptr;
}

EventloopThread::EventloopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr)
    , exiting_(false)
    , cb_(cb)
    , thread_(std::bind(&EventloopThread::threadFunc,this), name)
{
    sem_init(&sem_,0,0);
}

EventloopThread::~EventloopThread()
{
    exiting_=true;
    if(loop_){
        loop_->quit();
        thread_.join();
    }
    sem_destroy(&sem_);
}

Eventloop *EventloopThread::startLoop()
{
    thread_.start();
    sem_wait(&sem_);
    return loop_;
}
