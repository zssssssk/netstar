#pragma once
#include"noncopyable.h"
#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <string>
#include <atomic>
class Thread:noncopyable
{
private:
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    using F=std::function<void()>;
    F f_;
    std::string name_;
    static std::atomic_int numCreated_;

    void setDefaultName();
public:
    explicit Thread(F,const std::string& name="");
    ~Thread();

    void start();
    void join();
    bool started() const{
        return started_;
    }
    pid_t tid() const{
        return tid_;
    }
    const std::string& name() const{
        return name_;
    }
    static int numCreated(){
        return numCreated_;
    }
};
