#include <MutexLocker.h>
#include <Mutex.h>

using moxie::Mutex;
using moxie::MutexLocker;

MutexLocker::MutexLocker(Mutex& mutex):mutex_(mutex){
    mutex_.lock();
}

MutexLocker::~MutexLocker(){
    mutex_.unlock();
}
