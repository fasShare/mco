#ifndef MOXIE_MCOCALLSTACK_H
#define MOXIE_MCOCALLSTACK_H
#include <map>

#include <PoolInThreads.hpp>

namespace moxie {

class McoRoutine;

class McoCallStack {
public:
    size_t& index() { return index_; }

    McoRoutine*& operator[](const size_t index) {
        return callStack_[index];
    }

    McoRoutine* cur() {
        if (index_ > 0) {
            return callStack_[index_ - 1];
        }
        return nullptr;
    }
    
    bool vaild() {
        return true;
    }

    size_t size() {
        return callStack_.size();
    }

    bool empty() const {
        return callStack_.size() == 0;
    }

    McoCallStack() :
        callStack_(),
        index_(0) {
    }

    static McoCallStack *CallStack() {
        return moxie::PoolInThreads<McoCallStack *>::Item();
    }
    
    static McoCallStack *CallStack(long tid) {
        return moxie::PoolInThreads<McoCallStack *>::Item(tid);
    }
private:
    std::map<size_t, McoRoutine*> callStack_;
    size_t index_;
};

}
#endif //MCOCALLSTACK_H
