#pragma once
#include "noncopyable.h"
#include "Thread.h"
#include <functional>
#include <string>
#include"Eventloop.h"
#include<semaphore.h>

class EventloopThread:noncopyable
{
private:
    Eventloop* loop_;
    bool exiting_;
    Thread thread_;
    using ThreadInitCallback=std::function<void(Eventloop*)>;
    ThreadInitCallback cb_;
    sem_t sem_;

    void threadFunc();
public:
    EventloopThread(const ThreadInitCallback& cb,const std::string &name="");
    ~EventloopThread();

    Eventloop* startLoop();
};


