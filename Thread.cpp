#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

void Thread::setDefaultName()
{
    int num=++numCreated_;
    if(name_.empty()){
        char buf[32]={0};
        snprintf(buf,sizeof buf,"Thread%d",num);
        name_=buf;
    }
}

Thread::Thread(F f, const std::string &name)
    : started_(false), joined_(false), tid_(0), f_(std::move(f)), name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_&&!joined_){
        thread_->detach();
    }
}

void Thread::start()
{
    started_=true;
    sem_t sem;
    sem_init(&sem,0,0);
    thread_=std::make_shared<std::thread>([&](){
        tid_=CurrentThread::tid();
        sem_post(&sem);
        f_();
    });
    sem_wait(&sem);
}

void Thread::join()
{
    joined_=true;
    thread_->join();
}
