#include <McoRoutine.h>
#include <McoStack.h>
#include <McoCallStack.h>
#include <define.h>

#include <iostream>

using namespace moxie;

McoRoutine *co = nullptr, *co1 = nullptr, *co2 = nullptr, *co3 = nullptr;
int count = 100000000;

void func3() {
    char str[100];
    (void)str;
    while(true) {
        std::cout << "Begin func3" << std::endl;
        count--;
        co3->yield();
    }
}

void func2() {
    char str[200];
    (void)str;
    while(true) {
        std::cout << "Begin func2" << std::endl;
        co3->resume();
        std::cout << "End func2" << std::endl;
        co2->yield();
    }
}

void func1() {
    char str[300];
    (void)str;
    while (true) {
        std::cout << "Begin func1" << std::endl;
        co2->resume();
        std::cout << "End func1" << std::endl;
        co1->yield();
    }
}

void func() {
    char str[400];
    (void)str;
    while (count >= 0) {
        std::cout << "Begin func" << std::endl;
        co1->resume();
        std::cout << "End func" << std::endl;
    }
}

int main () {
    long tid = gettid();
    co = new McoRoutine(func);
    co1 = new McoRoutine(func1);
    co2 = new McoRoutine(func2);
    co3 = new McoRoutine(func3);
    std::cout << "co:" << (unsigned long)co << std::endl;    
    std::cout << "co1:" << (unsigned long)co1 << std::endl;    
    std::cout << "co2:" << (unsigned long)co2 << std::endl;    
    std::cout << "co3:" << (unsigned long)co3 << std::endl;    
    auto stack = new McoStack(tid);
    auto stack1 = new McoStack(tid);
    auto stack2 = new McoStack(tid);
    auto stack3 = new McoStack(tid);
    auto callstack = McoCallStack::CallStack();
    
    co->callStack(callstack);
    co->stack(stack);
    
    co1->callStack(callstack);
    co1->stack(stack1);

    co2->callStack(callstack);
    co2->stack(stack2);
    
    co3->callStack(callstack);
    co3->stack(stack3);
    
    co->resume();    
}
