#ifndef MOXIE_MUTEXLOCKER_H
#define MOXIE_MUTEXLOCKER_H
#include <pthread.h>

namespace moxie {

class Mutex;

class MutexLocker {
public:
    MutexLocker(Mutex& mutex);
    ~MutexLocker();
private:
    Mutex& mutex_;
};

}
#endif // MOXIE_MUTEXLOCKER_H
