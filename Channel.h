#pragma once
#include<functional>
#include"Timestamp.h"
#include<memory>
#include<atomic>
#include"Eventloop.h"
#include <sys/epoll.h>
#include"Logger.h"
class Eventloop;
class Channel
{
private:
    int fd_;
    using EventCallback=std::function<void()>;
    using ReadEventCallback=std::function<void(Timestamp)>;
    Eventloop* loop_;
    int events_;
    int revents_; 
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    std::atomic_bool tied_;
    std::weak_ptr<void> tie_;
    

public:
    Channel(Eventloop* loop,int fd);
    ~Channel()=default;
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }
    //delete channel from epoller
    void remove();
    //so tie is required to prevent channel from being deleted while still executing callback
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    int revents() const {return revents_;}
    void set_revents(int revt) { revents_ = revt; }

    static const int kReadEvent=EPOLLIN|EPOLLPRI;
    static const int kWriteEvent=EPOLLOUT;
    static const int kNoneEvent=0;
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    void update();

    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    void handleEvent(Timestamp receiveTime);
    Eventloop* ownerLoop() { return loop_; }
    void handleEventWithGuard(Timestamp receiveTime);
};

