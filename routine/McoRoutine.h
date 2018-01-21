#ifndef MOXIE_MCOROUTINE_H
#define MOXIE_MCOROUTINE_H
#include <string.h>
#include <syscall.h>
#include <iostream>
#include <functional>
#include <assert.h>

#include <Mcontext.h>
#include <McoStack.h>
#include <McoCallStack.h>
#include <Log.h>

extern "C"
{
    extern void mcontext_swap(Mcontext *, Mcontext *) asm("mco_swap");
};

namespace moxie {

using Routine = std::function<void ()>;

class McoRoutine {
public:
    McoRoutine(Routine run, bool use_private = false) :
        sink_(nullptr),
        stack_(nullptr),
        callstack_(nullptr),
        ctx_(new Mcontext),
        corun_(run),
        priStack_(use_private),
        main_(false),
        called_(false),
        shouldClose_(false),
        start_(false),
        running_(false),
        done_(false),
        dyield_(false),
        store_(false),
        sinked_(false) {
        }
    ~McoRoutine() {
        delete stack_;
        stack_ = nullptr;
        LOGGER_TRACE("McoRoutine [" << (unsigned long)this << "]will be destroyed.");
        //		std::cout << gettid() << " McoRoutine [" << (unsigned long)this << "]will be destroyed." << std::endl;
    }

    inline void isMain(bool m) {
        main_ = m;
    }

    inline bool isMain() {
        return main_;
    }

    inline bool sinked() {
        return sinked_;
    }

    inline void sinked(bool sink) {
        sinked_ = sink;
    }

    inline void resume() {
        if (running_ || (done_ && dyield_)) {
            return;
        }
        assert(callstack_ && callstack_->vaild());
        auto& index = callstack_->index();
        LOGGER_TRACE("callstack index:" << index);
        if (0 >= index) { 
            auto stack = new McoStack(gettid(), 1024 * 1024, true);
            assert(stack && stack->vaild());
            auto mco = new McoRoutine(Empty, true);
            mco->stack(stack);
            mco->callStack(callstack_);
            mco->initMcontext();
            McontextMake(mco->ctx_, (cofunc)McoRoutine::RunRoutine, mco);
            mco->main_ = true;
            mco->start_ = true;
            mco->running_ = true;
            mco->called_ = true;
            (*callstack_)[index] = mco;
            index++;
        }
        assert(index > 0);
        auto cur = (*callstack_)[index - 1];
        if (cur->sink_ != this) {
            if (!start_) {
                start_ = true;
                initMcontext();
                McontextMake(ctx_, (cofunc)McoRoutine::RunRoutine, this);
            }
            running_ = true;
            called_ = true;
            sink_ = cur;
            (*callstack_)[index] = this;
            ++index;
            cur->running_ = false;
            //std::cout << gettid() << " resume this=" << (unsigned long)this << " cur:" << (unsigned long)cur << std::endl;
            //LOGGER_TRACE("resume cur=" << (unsigned long)cur);
            //LOGGER_TRACE("resume this=" << (unsigned long)this);
            swap(cur, this);
        } else {
            assert(index >= 2);
            cur->yield();
        }
    }

    inline void yield() {
        if (!running_ && (done_ && dyield_)) {
            return;
        }
        assert(!main_);
        if (done_) {
            dyield_ = true; // routine is done and do the last yield.
            shouldClose_ = true;
        }
        assert(callstack_ && callstack_->vaild());
        auto& index = callstack_->index();
        assert(index >= 2);
        assert(this == (*callstack_)[index - 1]);
        auto sink = (*callstack_)[index - 2];
        assert(sink_ == sink);
        sink_ = nullptr;
        sink->running_ = true;
        running_ = false;
        called_ = false; // remove from callstack
        (*callstack_)[index - 1] = nullptr;
        //std::cout << gettid() << " yield this=" << (unsigned long)this << " sink=" << (unsigned long)sink << std::endl;
        --index;
        swap(this, sink);
    }

    inline static void swap(McoRoutine *sink, McoRoutine *co) {
        char c;
        //LOGGER_TRACE("begin swap addr_c:" << (unsigned long)&c);
        //std::cout << gettid() << " begin swap addr_c:" << (unsigned long)&c << std::endl;
        if (!(sink->done_ && sink->dyield_)) {
            sink->stack_->ssp(&c);
            sink->sinked(true);
            if (!sink->priStack_ && !sink->store_) {
                sink->store_ = true;
                auto ocpy = sink->stack_->occupy();
                assert(ocpy == sink);
                sink->stack_->occupy(nullptr);
                //std::cout << gettid() << " store sink:" << (unsigned long)sink << std::endl;
                McoStack::StoreStack(sink->stack_);
            }
        } else {
            auto ocp = sink->stack_->occupy();
            if (ocp == sink) {
                sink->stack_->occupy(nullptr);
            }
        }
        if (!co->priStack_) {
            auto occupy = co->stack_->occupy();
            if (occupy) {
                assert(occupy->sinked());
            }
            co->sinked(false);
            co->stack_->occupy(co);
            if (occupy && occupy != co
                    && !(occupy->done_ && occupy->dyield_)
                    && occupy->stack_) {
                assert(occupy->stack_);
                assert(occupy->stack_->ssp());
                occupy->store_ = true;
                //LOGGER_TRACE("occupy:" << (unsigned long)occupy);
                //std::cout << gettid() << " store occupy:" << (unsigned long)occupy << std::endl;
                McoStack::StoreStack(occupy->stack_);
            }
        }

        //LOGGER_TRACE("before mcontext_swap");
        mcontext_swap(sink->ctx_, co->ctx_);
        //LOGGER_TRACE("after mcontext_swap");
        auto callstack = McoCallStack::CallStack();
        auto cur = callstack->cur();
        auto cpy = cur->stack_->occupy();
        if (cpy == cur && !cur->priStack_) {
            cur->stack_->occupy(nullptr);
        }
        //LOGGER_TRACE("cur co=" << (unsigned long)cur);
        //LOGGER_TRACE("callstack index=" << (unsigned long)callstack->index());
        //LOGGER_TRACE("callstack size=" << (unsigned long)callstack->size());
        if (cur && cur->store_
                && cur->stack_
                && !cur->priStack_) {
            cur->store_ = false;
            assert(cur->callstack_ == callstack);
            cur->stack_->occupy(cur);
            McoStack::RecoverStack(cur->stack_);
        }
        //LOGGER_TRACE("end swap");
        //std::cout << gettid() << " end swap, cur_co=" << (unsigned long)cur << std::endl;
    }

    inline bool callStack(McoCallStack *callstack) {
        if (callstack
                && callstack->vaild()) {
            callstack_ = callstack;
            return true;
        }
        return false;
    }

    inline bool stack(McoStack *stack) {
        if (stack
                && stack->vaild()) {
            stack_ = stack;
            return true;
        }
        return false;
    }

    inline McoStack *stack() {
        return stack_;
    }

    inline static void Empty() {
        assert(false);
    }

    inline static void RunRoutine(McoRoutine *co) {
        if (co && co->running_) {
            try {
                co->corun_();
            } catch (...) {
                LOGGER_WARN("Some exception happened.");            
            }
            co->done_ = true;
            co->yield();
        }
    }
private:
    inline void initMcontext() {
        McontextInit(ctx_);
        ctx_->ss_sp = stack_->stack();
        ctx_->ss_size = stack_->size();
    }
private:
    McoRoutine* sink_;
    McoStack* stack_;
    McoCallStack* callstack_;
    Mcontext* ctx_;
    Routine corun_;
    bool priStack_;
    bool main_;
    bool called_;
    bool shouldClose_;
    bool start_;
    bool running_;
    bool done_;
    bool dyield_;
    bool store_;
    bool sinked_;
};

}
#endif
