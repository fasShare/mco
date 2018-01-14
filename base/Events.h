#ifndef MOXIE_EVENTS_H
#define MOXIE_EVENTS_H
#include <iostream>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <dlfcn.h>
#include <string.h>

#include <Log.h>

#include <boost/static_assert.hpp>

using close_hook = int (*)(int fd);
static close_hook uni_close = (close_hook)dlsym(RTLD_NEXT, "close");

namespace moxie {

#define EVENT_TYPE_START        0
#define EVENT_TYPE_TCPCON       1
#define EVENT_TYPE_TIMER        2
#define EVENT_TYPE_TCPSER       3
#define EVENT_TYPE_EVENT        4

BOOST_STATIC_ASSERT_MSG(EPOLLIN == POLLIN, "EPOLLIN != POLLIN");
BOOST_STATIC_ASSERT_MSG(EPOLLOUT == POLLOUT, "EPOLLOUT != POLLOUT");
BOOST_STATIC_ASSERT_MSG(EPOLLPRI == POLLPRI, "EPOLLPRI != POLLPRI");
BOOST_STATIC_ASSERT_MSG(EPOLLERR == POLLERR, "EPOLLERR != POLLERR");
BOOST_STATIC_ASSERT_MSG(EPOLLRDHUP == POLLRDHUP, "EPOLLRDHUP != POLLRDHUP");

const uint32_t kNoneEvent = 0;
const uint32_t kReadEvent = POLLIN | POLLPRI;
const uint32_t kWriteEvent = POLLOUT;

#define    NEW 1
#define    MOD 2
#define    DEL 3
#define    BAD 4

class Events {
public:
    Events(const int fd, uint32_t events) :
        fd_(fd),
        tid_(gettid()),
        state_(NEW),
        eorigin_(events),
        mutable_(kNoneEvent) {
    }

    Events(const struct epoll_event& events) :
        Events(events.data.fd, events.events){
    }

    Events(moxie::Events&& that) {
        *this = std::move(that);
    }

    Events& operator=(moxie::Events&& that) {
        if (&that == this) {
            return *this;
        }
        this->fd_ = that.fd_;
        this->tid_ = that.tid_;
        this->state_ = that.state_;
        this->eorigin_ = that.eorigin_;
        this->mutable_ = that.mutable_;
        that.reset();
        return *this;
    }

    ~Events() {
        ::uni_close(fd_);
        LOGGER_TRACE("Events destroyed.");
    }

    bool reset() {
        fd_ = -1;
        state_ = BAD;
        eorigin_ = kNoneEvent;
        mutable_ = kNoneEvent;
        return true;
    }

    bool invaild() const {
        return state_ == BAD;
    }
    bool moder() const {
        return state_ == MOD;
    }
    bool newer() const {
        return state_ == NEW;
    }
    bool deler() const {
        return state_ == DEL;
    }

    int fd() const {
        return fd_;
    }

    void fd(int fd) {
        fd_ = fd;
    }

    long tid() const {
        return tid_;
    }

    void tid(long tid) {
        tid_ = tid;
    }
    
    uint32_t state() const {
        return state_;
    }

    void state(uint32_t sta) {
        state_ = sta;
    }

    uint32_t events() const {
        return eorigin_;
    }
    
    void events(uint32_t event) {
        eorigin_ = event;
        state_ = MOD;
    }

    uint32_t emutable() const {
        return mutable_;
    }

    void emutable(uint32_t event) {
        mutable_ = event;
    }

    bool isRead() {
        return contains(mutable_, POLLIN | POLLPRI | POLLRDHUP);
    }
    bool isWrite() {
        return contains(mutable_, POLLOUT);
    }
    bool isError() {
        return contains(mutable_, POLLERR | POLLNVAL);
    }
    bool isClose() {
        return contains(mutable_, POLLHUP) && !contains(mutable_, POLLIN);
    }

    bool contains(uint32_t emutable, uint32_t events) const {
        return ((emutable & events) != 0);
    }

    struct epoll_event epollEvents() {
        struct epoll_event events;
        bzero(&events, sizeof(struct epoll_event));
        events.data.fd = fd_;
        events.events = eorigin_;
        return events;
    }
    
    bool originRead() {
        return contains(eorigin_, POLLIN | POLLPRI | POLLRDHUP);
    }

    bool originWrite() {
        return contains(eorigin_, POLLOUT);
    }
private:
    Events& operator=(const Events& that);
    Events(const Events& that);

    int fd_;
    long tid_;
    uint32_t state_;
    uint32_t eorigin_;
    uint32_t mutable_;
};

}
#endif // MOXIE_EVENTS_H
