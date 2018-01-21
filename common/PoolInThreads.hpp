#ifndef TOOLS_POOLINTHREADS_H
#define TOOLS_POOLINTHREADS_H
#include <syscall.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <unordered_map>

#include <MutexLocker.h>
#include <Mutex.h>

#define gettid() (::syscall(SYS_gettid))

namespace moxie {

template <class T>
class PoolInThreads {
public:
    template <class... Args>
    static boost::shared_ptr<T> Item(Args... args) {
        return Instance()->getT(args...);
    }
    template <class... Args>
    static boost::shared_ptr<T> Item(long tid, Args... args) {
        return Instance()->getT(tid, args...);
    }
private:
    template <class... Args>
    boost::shared_ptr<T> getT(Args... args) {
        auto tid = gettid();
        return getT(tid, args...);
    }
    template <class... Args>
    boost::shared_ptr<T> getT(long tid, Args... args) {
        MutexLocker lock(mutex_);
        auto iter = pools_.find(tid);
        if (iter != pools_.end()) {
            return iter->second;
        }
        auto ret = boost::make_shared<T>(args...);
        pools_[tid] = ret;
        return ret;
    }
    static PoolInThreads *Instance() {
        if (!instance_) {
            instance_ = new PoolInThreads;
        }
        return instance_;
    }
    static PoolInThreads *instance_;
    std::unordered_map<long, boost::shared_ptr<T>> pools_;
    Mutex mutex_;
};
template <class T>
PoolInThreads<T> *PoolInThreads<T>::instance_ = nullptr;

// 模板的局部特化
template<class T> 
class PoolInThreads<T*> {
public:
    template <class... Args>
    static T* Item(Args... args) {
        return Instance()->getT(args...);
    }
    template <class... Args>
    static T* Item(long tid, Args... args) {
        return Instance()->getT(tid, args...);
    }
private:
    template <class... Args>
    T* getT(Args... args) {
        auto tid = gettid();
        return getT(tid, args...);
    }
    template <class... Args>
    T* getT(long tid, Args... args) {
        MutexLocker lock(mutex_);
        auto iter = pools_.find(tid);
        if (iter != pools_.end()) {
            return iter->second;
        }
        auto ret = new T(args...);
        pools_[tid] = ret;
        return ret;
    }
    static PoolInThreads *Instance() {
        if (!instance_) {
            instance_ = new PoolInThreads;
        }
        return instance_;
    }
    static PoolInThreads *instance_;
    std::unordered_map<long, T*> pools_;
    Mutex mutex_;
};

template <class T>
PoolInThreads<T*> *PoolInThreads<T*>::instance_ = nullptr;

}

#endif
