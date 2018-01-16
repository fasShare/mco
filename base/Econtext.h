#ifndef MOXIE_ECONTEXT_H
#define MOXIE_ECONTEXT_H
#include <memory>

#include <McoRoutine.h>

namespace moxie {

class EventLoop;

class Econtext {
public:
	EventLoop *loop() {
		return loop_;
	}

	void loop(EventLoop *loop) {
		loop_ = loop;
	}

    int fd() const {
        return fd_;
    }

    void fd(int fd) {
        fd_ = fd;
    }

    std::shared_ptr<Events> event() const {
        return event_;
    }

    void event(std::shared_ptr<Events> event) {
        event_ = event;
    }

    std::shared_ptr<McoRoutine> mco() const {
        LOGGER_TRACE("co_ [" << (unsigned long)(co_.get()) << "] use_count:" << co_.use_count());
        return co_;
    }

    void mco(std::shared_ptr<McoRoutine> co) {
        co_ = co;
    }

    bool invaild() const {
        if (event_) {
            return event_->invaild();
        }
        return true;
    }   
    bool moder() const {
        if (event_) {
            return event_->moder();
        }
        return false;
    }   
    bool newer() const {
        if (event_) {
            return event_->newer();
        }
        return false;
    }   
    bool deler() const {
        if (event_) {
            return event_->deler();
        }
        return false;
    }
private:
    int fd_;
	EventLoop *loop_;
    std::shared_ptr<Events> event_;
    std::shared_ptr<McoRoutine> co_;
};

}
#endif
