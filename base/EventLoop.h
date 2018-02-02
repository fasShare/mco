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
    EventLoop();
    ~EventLoop();

    bool put(std::shared_ptr<Econtext> ectx);
    std::shared_ptr<Econtext> econtext(std::shared_ptr<Events> ectx);
    std::shared_ptr<Econtext> econtext(int fd);
    long tid() const;
    void tid(long tid);
    bool notify();
    bool quit();
    void loop();
private:
    bool eventHandleAble(std::shared_ptr<Events> origin);
    bool wait();
    bool esync(std::shared_ptr<Econtext> ec);
private:
    Epoll *epoll_;
    Mutex mutex_;
    long tid_;
    int wfd_;
    bool quit_;
    std::vector<std::shared_ptr<Econtext>> food_;
    std::unordered_map<int, std::shared_ptr<Econtext>> ecs_;
    std::vector<PollerEvent> occur_;
};

}
#endif // MOXIE_EVENTLOOP_H
