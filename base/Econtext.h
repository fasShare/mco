#ifndef MOXIE_ECONTEXT_H
#define MOXIE_ECONTEXT_H
#include <memory>

#include <McoRoutine.h>

namespace moxie {

class EventLoop;

class Econtext {
public:
    Econtext() :
        fd_(-1),
        fdflag_(0),
        loop_(nullptr),
        event_(),
        co_(nullptr) {
    }
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

    int fdflag() {
        return fdflag_;
    }

    void fdflag(int fdflag) {
        fdflag_ = fdflag;
    }

    std::shared_ptr<Events> event() const {
        return event_;
    }

    void event(std::shared_ptr<Events> event) {
        event_ = event;
    }

    McoRoutine* mco() const {
        return co_;
    }

    void mco(McoRoutine* co) {
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
    ~Econtext() {
        //std::cout << gettid() << " Econtext will destroyed" << std::endl;
    }
private:
    int fd_;
    int fdflag_;
    EventLoop *loop_;
    std::shared_ptr<Events> event_;
    McoRoutine* co_;
};

}
#endif
