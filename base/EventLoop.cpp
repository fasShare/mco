#include <assert.h>
#include <sys/eventfd.h>

#include <EventLoop.h>
#include <define.h>
#include <Events.h>
#include <Epoll.h>
#include <Mutex.h>
#include <MutexLocker.h>
#include <Timestamp.h>
#include <Econtext.h>

using moxie::EventLoop;
using moxie::Events;
using moxie::Econtext;

EventLoop::EventLoop() :
    epoll_(new Epoll()),
    mutex_(),
    tid_(gettid()),
    wfd_(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
    quit_(false),
    ecs_() {
    assert(wfd_ >= 0);
    auto wec = std::make_shared<Econtext>();
    wec->event(std::make_shared<Events>(wfd_, kReadEvent));
    wec->fd(wfd_);
    esync(wec);
}

EventLoop::~EventLoop() {
    delete epoll_;
}

bool EventLoop::put(std::shared_ptr<Econtext> ectx) {
    MutexLocker lock(mutex_);
    if (gettid() != tid_) {    
        notify();
    }
    esync(ectx);
    return true;
}

std::shared_ptr<Econtext> EventLoop::econtext(std::shared_ptr<Events> event) {
    MutexLocker lock(mutex_);
    
    auto iter = ecs_.find(event->fd());
    if (iter == ecs_.end()) {
        return nullptr;
    }
    return iter->second;
}

long EventLoop::tid() const {
        return tid_;
    }

void EventLoop::tid(long tid) {
    tid_ = tid;
}

bool EventLoop::notify() {
    uint64_t note = 8;
    ssize_t ret = write(wfd_, &note, sizeof note);
    if (ret != sizeof note) {
        return false;    
    }
    return true;
}

bool EventLoop::quit() {
    MutexLocker lock(mutex_);
    quit_ = true;
    if (gettid() != tid_) {
        notify();
    }
    return true;
}

void EventLoop::loop() {
    assert(gettid() == tid_);
    while(!quit_) {
        LOGGER_TRACE("Start new loop");
        occur_.clear();
        epoll_->Loop(occur_, 20000);
        for(size_t i = 0; i < occur_.size(); ++i) {
            LOGGER_TRACE("Begin handle event");
            //std::cout << gettid() << " begin handle[" << i << "]" << std::endl;
            int fd = occur_[i].fd;
            std::shared_ptr<Econtext> ectx = nullptr;
            {
                MutexLocker lock(mutex_);
                auto iter = ecs_.find(fd);
                if (iter == ecs_.end()) {
                    LOGGER_TRACE("can't find fd:" << fd);
                    continue;
                }
                ectx = iter->second;
                if (!ectx) {
                    continue;
                }
            }
        
            if (fd == wfd_) {
                wait();
                continue;
            }

            auto event = ectx->event();
            if (!event) {
                continue;
            }
            
            event->emutable(occur_[i].event);
            if (!eventHandleAble(event)) {
                continue;
            }

            auto co = ectx->mco();
            //std::cout << gettid() << " co [" << (unsigned long)(co.get())
            //          << "] use_count:" << co.use_count() << std::endl;
            if (co) {
                co->resume();
            }
            //std::cout << gettid() << " co [" << (unsigned long)(co.get())
            //          << "] use_count:" << co.use_count() << std::endl;
        }
    }
}
bool EventLoop::eventHandleAble(std::shared_ptr<Events> origin) {
    if (origin->deler()) {
        return false;
    }   
    if (origin->moder()) {
        if (origin->isRead() && (!origin->originRead())) {
            origin->emutable(origin->emutable() & ~kReadEvent);
        }
        if (origin->isWrite() && (!origin->originWrite())) {
            origin->emutable(origin->emutable() & ~kWriteEvent);
        }
        return origin->emutable() != kNoneEvent;
    }   
    return true;
}

bool EventLoop::wait() {
    uint64_t note = 0;
    ssize_t ret = read(wfd_, &note, sizeof note);
    if (ret != sizeof note) {
        return false;
    }
    return true;
}

bool EventLoop::esync(std::shared_ptr<Econtext> ec) {
    int fd = ec->fd();
    auto event = ec->event();
    assert(event && (event->fd() == fd));
    LOGGER_TRACE("ecs_size:" << ecs_.size());
    if (ec->deler()) {
        LOGGER_TRACE("Delete Econtext");
        //std::cout << gettid() << " Delete Econtext mco:" << (unsigned long)ec->mco().get() << std::endl;;
        int count = ecs_.count(fd);
        if (count != 1) {
            std::cout << gettid() << " count != 1 count_val=[" << count << "]" << std::endl;
            assert(ecs_.count(fd) == 1);
        }
        epoll_->del(event.get());
        ecs_.erase(fd);
    } else if (ec->moder()) {
        LOGGER_TRACE("Modify Econtext");
        assert(ecs_.count(fd) > 0);
        epoll_->mod(event.get());
    } else if (event->newer()) {
        LOGGER_TRACE("Update new Econtext");
        assert(ecs_.count(fd) == 0);
        ecs_[fd] = ec;
        epoll_->add(event.get());
    } else {
        assert(false);
    }
    LOGGER_TRACE("ecs_size:" << ecs_.size());
    return true;
}
