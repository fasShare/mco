#ifndef MOXIE_MCOCALLSTACK_H
#define MOXIE_MCOCALLSTACK_H
#include <map>

#include <PoolInThreads.hpp>

namespace moxie {

class McoRoutine;

class McoCallStack {
public:
    McoCallStack() :
        callStack_(),
        index_(0) {
    }

    inline size_t& index() { return index_; }

    inline McoRoutine*& operator[](const size_t index) {
        return callStack_[index];
    }

    inline McoRoutine* cur() {
        if (index_ > 0) {
            return callStack_[index_ - 1];
        }
        return nullptr;
    }
    
    inline bool vaild() {
        return true;
    }

    inline size_t size() {
        return index_;
    }

    inline bool empty() const {
        return index_ == 0;
    }

    inline static McoCallStack *CallStack() {
        return moxie::PoolInThreads<McoCallStack *>::Item();
    }
    
    inline static McoCallStack *CallStack(long tid) {
        return moxie::PoolInThreads<McoCallStack *>::Item(tid);
    }
private:
    std::map<size_t, McoRoutine*> callStack_;
    size_t index_;
};

}
#endif //MCOCALLSTACK_H
