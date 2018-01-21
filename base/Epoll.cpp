#include <iostream>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


#include <Epoll.h>
#include <Log.h>
#include <Timestamp.h>
#include <Events.h>

using moxie::Events;
using moxie::Epoll;
using moxie::Timestamp;

Epoll::Epoll() :
    epoll_fd_(-1),
    revents_(),
    maxNum_(20),
    addStep_(15) { 
    //FIXME : EPOLL_CLOEXEC
    epoll_fd_ = ::epoll_create(1);
    if(epoll_fd_ == -1) {
        LOGGER_SYSERR("epoll_create error : " << ::strerror(errno));
    }
    revents_.resize(maxNum_);
}

bool Epoll::eventCtl(int op, int sd, EpollEvent* event) {
    assert(epoll_fd_ != -1);
    int ret = ::epoll_ctl(epoll_fd_, op, sd, event);
    if (ret == -1) {
        LOGGER_SYSERR("epoll_ctl error : " << ::strerror(errno) << " fd : " << sd);
        return false;
    } else {
        return true;
    }
}

bool Epoll::eventAdd(int sd, EpollEvent* event) {
    return eventCtl(EPOLL_CTL_ADD, sd, event);
}

bool Epoll::eventDel(int sd, EpollEvent* event) {
    return eventCtl(EPOLL_CTL_DEL, sd, event);
}

bool Epoll::eventMod(int sd, EpollEvent* event) {
    return eventCtl(EPOLL_CTL_MOD, sd, event);
}

int Epoll::loopWait(EpollEvent* events, int maxevents, int timeout) {
should_continue:
    int ret = ::epoll_wait(epoll_fd_, events, maxevents, timeout);
    if (ret == -1) {
        if (errno == EINTR) {
            LOGGER_SYSERR("epoll_wait error : " << ::strerror(errno));
            goto should_continue;
        }
        LOGGER_SYSERR("epoll_wait error : " << ::strerror(errno));
    }
    return ret;
}

bool Epoll::add(Events* events) {
    EpollEvent event = events->epollEvents();
    return this->eventAdd(event.data.fd, &event);
}

bool Epoll::mod(Events* events) {
    EpollEvent event = events->epollEvents();
    return this->eventMod(event.data.fd, &event);
}

bool Epoll::del(Events* events) {
    EpollEvent event = events->epollEvents();
    return this->eventDel(event.data.fd, &event);
}

Timestamp moxie::Epoll::Loop(std::vector<PollerEvent> &events, int timeout) {
    int ret = this->loopWait(revents_.data(), maxNum_, timeout);
    for(int i = 0; i < ret; i++) {
        PollerEvent event;
        event.fd = revents_[i].data.fd;
        event.event = revents_[i].events;
        events.emplace_back(event);
    }
    //no use!
    revents_.clear();
    if (ret == maxNum_) {
        maxNum_ += addStep_;
        revents_.resize(maxNum_);
    }
    return Timestamp::now();
}

Epoll::~Epoll() {
    ::close(epoll_fd_);
}

