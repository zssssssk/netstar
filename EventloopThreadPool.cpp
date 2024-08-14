#include "EventloopThreadPool.h"
#include"Logger.h"


EventloopThreadPool::EventloopThreadPool(Eventloop * baseloop, const std::string & nameArg)
    :baseloop_(baseloop)
    ,name_(nameArg)
    ,started_(false)
    ,numThreads_(0)
    ,next_(0)
{
}

void EventloopThreadPool::start(const ThreadInitCallback &cb)
{
    started_=true;
    for(int i=0;i<numThreads_;i++){
        std::string tName=name_;
        tName+=std::to_string(i);
        threads_.push_back(std::make_unique<EventloopThread>(cb,tName));
        loops_.emplace_back(threads_.back()->startLoop());
    }
    // 整个服务端只有一个线程，运行着baseloop
    if(numThreads_==0&&cb){
        cb(baseloop_);
    }
}

Eventloop *EventloopThreadPool::getNextloop()
{
    if(numThreads_==0) return baseloop_;
    next_++;
    if(next_==numThreads_) next_=0;
    return loops_[next_];
}

std::vector<Eventloop *> EventloopThreadPool::getAllLoop()
{
    if(loops_.empty()){
        return std::vector<Eventloop*>(1,baseloop_);
    }
    return loops_;
}
