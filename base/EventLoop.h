#ifndef MOXIE_EVENTLOOP_H
#define MOXIE_EVENTLOOP_H
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <memory>
#include <assert.h>
#include <sys/eventfd.h>

#include <define.h>
#include <Events.h>
#include <Epoll.h>
#include <Mutex.h>
#include <MutexLocker.h>
#include <Timestamp.h>
#include <Econtext.h>

namespace moxie {

class EventLoop {
public:
    EventLoop() :
        epoll_(new Epoll()),
        mutex_(),
        tid_(gettid()),
        wfd_(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
        quit_(false),
        food_(),
        ecs_() {
        assert(wfd_ >= 0);
        auto wec = std::make_shared<Econtext>();
        wec->event(std::make_shared<Events>(wfd_, kReadEvent));
        wec->fd(wfd_);
        esync(wec);
    }

	~EventLoop() {
        delete epoll_;
    }

    bool put(std::shared_ptr<Econtext> ectx) {
        MutexLocker lock(mutex_);
        if (gettid() != tid_) {    
            food_.push_back(ectx);
            notify();
        } else {
            esync(ectx);
        }
        return true;
    }
    
    long tid() const {
        return tid_;
    }

    void tid(long tid) {
        tid_ = tid;
    }

    bool notify() {
        uint64_t note = 8;
        ssize_t ret = write(wfd_, &note, sizeof note);
        if (ret != sizeof note) {
            return false;    
        }
        return true;
    }

    bool quit() {
        MutexLocker lock(mutex_);
        quit_ = true;
        if (gettid() != tid_) {
            notify();
        }
        return true;
    }

    void loop() {
        assert(gettid() == tid_);
        std::vector<PollerEvent> occur;
        while(!quit_) {
			LOGGER_TRACE("Start new loop");
            eat();
			occur.clear();
            epoll_->Loop(occur, 20000);
            for(size_t i = 0; i < occur.size(); ++i) {
				LOGGER_TRACE("Begin handle event");
                int fd = occur[i].fd;
                auto ec = ecs_.find(fd);
                if (ec == ecs_.end()) {
					LOGGER_TRACE("can't find fd:" << fd);
                    assert(false);
                }
				if (fd == wfd_) {
					wait();
					continue;
				}
                auto co = ec->second->mco();
                LOGGER_TRACE("co [" << (unsigned long)(co.get()) << "] use_count:" << co.use_count());
                if (co) {
                    co->resume();
                }
                LOGGER_TRACE("co [" << (unsigned long)(co.get()) << "] use_count:" << co.use_count());
            }
        }
    }
private:
    bool eventHandleAble(boost::shared_ptr<Events> origin) {
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

    bool wait() {
        uint64_t note = 0;
        ssize_t ret = read(wfd_, &note, sizeof note);
        if (ret != sizeof note) {
            return false;
        }
        return true;
    }

    bool esync(std::shared_ptr<Econtext> ec) {
        assert(gettid() == tid_);
        int fd = ec->fd();
        auto event = ec->event();
        assert(event && (event->fd() == fd));
		LOGGER_TRACE("ecs_size:" << ecs_.size());
        if (ec->deler()) {
			LOGGER_TRACE("Delete Econtext");
            assert(ecs_.count(fd) == 1);
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

    bool eat() {
        MutexLocker lock(mutex_);
        size_t count = 0;
        size_t i = 0;
        for (; i < food_.size(); ++i) {
            if (!esync(food_[i])) {
                ++count;
            }
        }
        food_.clear();
        return (count != 0) && (count != i);
    }
private:
    Epoll *epoll_;
    Mutex mutex_;
    long tid_;
    int wfd_;
    bool quit_;
    std::vector<std::shared_ptr<Econtext>> food_;
    std::unordered_map<int, std::shared_ptr<Econtext>> ecs_;
};

}
#endif // MOXIE_EVENTLOOP_H
