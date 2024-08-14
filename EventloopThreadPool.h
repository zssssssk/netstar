#pragma once
#include "noncopyable.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include"Eventloop.h"
#include"EventloopThread.h"
class EventloopThreadPool:noncopyable
{
private:
    Eventloop* baseloop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventloopThread>> threads_;
    std::vector<Eventloop*> loops_;

public:
    EventloopThreadPool(Eventloop* baseloop,const std::string& nameArg);
    ~EventloopThreadPool()=default;
    void setThreadNum(int n){numThreads_=n;}
    using ThreadInitCallback=std::function<void(Eventloop*)>;
    void start(const ThreadInitCallback& cb=ThreadInitCallback());
    Eventloop* getNextloop();
    std::vector<Eventloop*> getAllLoop();
    bool started() const{
        return started_;
    }
    const std::string name() const{
        return name_;
    }
};

