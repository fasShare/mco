#ifndef MOXIE_MCOSTACK_H
#define MOXIE_MCOSTACK_H
#include <set>
#include <map>

#include <Log.h>
#include <PoolInThreads.hpp>

namespace moxie {

class McoRoutine;

class stackmem {
public:
    stackmem(size_t size = 1024 * 1024):
        stack_(nullptr),
        size_(size),
        occupy_(nullptr) {
        if (size_ > 0) {
            stack_ = new char[size_];
        } else {
            assert(false);
        }
    }

    size_t size() {
        return size_;
    }

    char *stack() {
        return stack_;
    }

    void occupy(McoRoutine *co) {
        occupy_ = co;
    }
    McoRoutine *occupy() {
        return occupy_;
    }
private:
    char *stack_;
    size_t size_;
    McoRoutine *occupy_;
};

class CommonStack {
public:
    CommonStack(size_t num = 2, size_t size = 1024 * 1024) :
        index_(0),
        num_(num),
        size_(size),
        vaild_(num_ > 0 && size_ > 0),
        mempools_() {
        if (vaild_) {
            for (size_t i = 0; i < num_; ++i) {
                mempools_.emplace_back(new stackmem(size_));
            }
            assert(mempools_.size() == num_);
        }
    }
    
    bool vaild() {
        return vaild_;
    }

    stackmem *common() {
        if (index_ >= mempools_.size()) {
            index_ = 0;
        }
        assert(num_ == mempools_.size());
        return mempools_[index_++];
    }

    static CommonStack* Stack(long tid) {
        return PoolInThreads<CommonStack *>::Item(tid); 
    }
    
    static CommonStack* Stack() {
        return PoolInThreads<CommonStack *>::Item(); 
    }
private:
    size_t index_;
    size_t num_;
    size_t size_;
    bool vaild_;
    std::vector<stackmem *> mempools_;
};

class McoStack {
public:
    McoStack(long tid, size_t size = 1024 * 1024, bool pri = false) :
        stack_(nullptr),
        common_(nullptr),
        size_(size),
        sbp_(nullptr),
        ssp_(nullptr),
        stmp_(nullptr),
        vaild_(false),
        private_(pri),
        tid_(tid),
        rsize_(0) {
        if (private_) {
            if (size_ & 0xFFF) {
                size_ &= ~0xFFF;
                size_ += 0x1000;
            }
            stack_ = new char[size_];
            if (stack_ && size_ > 0) {
                sbp_ = stack_ + size_;
                vaild_ = true;
            }
            LOGGER_TRACE("create private stack!");
        } else {
            auto cstack = CommonStack::Stack(tid);
            common_ = cstack->common();
            size_ = common_->size();
            stack_ = common_->stack();
            sbp_ = stack_ + size_;
            vaild_ = true;
            LOGGER_TRACE("create common stack!");
        }   
    }

    ~McoStack() {
        if (private_) {
            delete[] stack_;
        } else {
            delete[] stmp_;
        }
    }
    
    void ssp(char *sp) {
        ssp_ = sp;
    }

    size_t size() {
        return size_;
    }

    char *stack() {
        return stack_;
    }

    void occupy(McoRoutine *co) {
        if (private_) {
            assert(common_ == nullptr);
        } else {
            common_->occupy(co);
        }
    }

    McoRoutine *occupy() {
        if (private_) {
            assert(common_ == nullptr);
            return nullptr;
        } else {
            return common_->occupy();
        }
    }

    bool vaild() {
        return vaild_;
    }

    long tid() {
        return tid_;
    }

    void tid(long tid) {
        tid_ = tid;
    }

    static void StoreStack(McoStack *stack) {
        if (!stack || !stack->vaild()) {
            return;
        }
        stack->rsize_ = 0;
        if (stack->stmp_) {
            delete[] stack->stmp_;
            stack->stmp_ = nullptr;
        }
        stack->rsize_ = stack->sbp_ - stack->ssp_;
        assert(stack->rsize_ > 0);
        stack->stmp_ = new char[stack->rsize_];
        LOGGER_TRACE("stack:" << (unsigned long)stack);
        LOGGER_TRACE("Store->bottom:" << (unsigned long)stack->ssp_);
        LOGGER_TRACE("Store->top:" << (unsigned long)stack->sbp_);
        LOGGER_TRACE("Store->restore_size:" << stack->rsize_);
        memcpy(stack->stmp_, stack->ssp_, stack->rsize_);
    }

    static void RecoverStack(McoStack *stack) {
        if (!stack || !stack->vaild()) {
            return;
        }
        assert(stack->rsize_ >= 0);
        LOGGER_TRACE("stack:" << (unsigned long)stack);
        LOGGER_TRACE("Recover->bottom:" << (unsigned long)stack->ssp_);
        LOGGER_TRACE("Recover->top:" << (unsigned long)stack->sbp_);
        LOGGER_TRACE("Recover->restore_size:" << stack->rsize_);
        memcpy(stack->ssp_, stack->stmp_, stack->rsize_);
    }
private:
    char *stack_;
    stackmem *common_;
    size_t size_;
    char *sbp_;
    char *ssp_;
    char *stmp_;
    bool vaild_;
    bool private_;
    long tid_;
    size_t rsize_;
};

}
#endif //MOXIE_MCOSTACK_H
