#include <McoRoutine.h>
#include <McoStack.h>
#include <McoCallStack.h>
#include <SigIgnore.h>
#include <EventLoop.h>
#include <Socket.h>
#include <functional>
#include <NetAddress.h>
#include <EventLoopPool.h>
#include <thread>

using namespace moxie;

SigIgnore signor;

void client_call(std::weak_ptr<Econtext> wect) {
    auto wectt = std::move(wect);
	while (true) {
		auto ect = wectt.lock();
	    //std::cout << gettid() << " after lock." << std::endl;
		if (!ect) {
			std::cout << gettid() << " lock nullptr" << std::endl;
			break;
		}
		auto mco = ect->mco();
        LOGGER_TRACE("mco[" << (unsigned long)(mco.get()) << "] use_count:" << mco.use_count());
		char buf[1024];
		int ret = read(ect->fd(), buf, 1023);
		LOGGER_TRACE("New socket io event on fd:" << ect->fd());
		if (ret > 0) {
			buf[ret] = 0;
			LOGGER_TRACE("recv:" << buf << " size:" << ret);
		} else {
			auto loop = ect->loop();
			if (loop) {
				auto event = ect->event();
				if (event) {
                    //std::cout << gettid() << " will delete econtext." << std::endl;
					event->state(DEL);
					loop->put(ect);
				} else {
					assert(false);
				}
			} else {
				assert(false);
			}
			break;
		}
		if (mco) {
			mco->yield();
		}
	}
	LOGGER_TRACE("Out of while.");
}

void accept_call(std::shared_ptr<Socket> acp, std::weak_ptr<Econtext> wect) {
	while (true) {
		auto ect = wect.lock();
		if (!ect) {
			break;
		}
		auto mco = ect->mco();
		NetAddress addr;
		auto client = acp->accept(addr);
		if (client > 0) {
			auto loop = EventLoopPool::NextLoop();
			auto aev = std::make_shared<Events>(client, kReadEvent);
			auto econtext = std::make_shared<Econtext>();
			
			std::weak_ptr<Econtext> wec = econtext;
			auto aco = std::make_shared<McoRoutine>(std::bind(client_call, wec));
			
			auto stack = new McoStack(loop->tid());
			auto callstack = McoCallStack::CallStack(loop->tid());
			
			aco->stack(stack);
			aco->callStack(callstack);

			econtext->fd(client);
			econtext->mco(aco);
			econtext->event(aev);
			econtext->loop(loop);

			loop->put(econtext);
		}
		if (mco) {
			mco->yield();
		}
	}
}

void thread_func() {
	auto loop = new EventLoop();
	if (loop) {
		EventLoopPool::AddEventLoop(loop);

		loop->loop();
	}
}

int main() {
	auto mainloop = new EventLoop();
	EventLoopPool::AddMainLoop(mainloop);

	NetAddress addr(AF_INET, 6686, "127.0.0.1");
	auto acp = std::make_shared<Socket>(AF_INET, SOCK_STREAM, 0);
	assert(acp->bind(addr));
	assert(acp->listen(50));
	auto aev = std::make_shared<Events>(acp->getSocket(), kReadEvent);
	auto econtext = std::make_shared<Econtext>();

	auto aco = std::make_shared<McoRoutine>(std::bind(accept_call, acp, econtext));
	auto stack = new McoStack(mainloop->tid());
	auto callstack = McoCallStack::CallStack(mainloop->tid());
	aco->stack(stack);
	aco->callStack(callstack);

	econtext->fd(acp->getSocket());
	econtext->mco(aco);
	econtext->event(aev);
	econtext->loop(mainloop);

	std::thread t1(thread_func);
	std::thread t2(thread_func);
	std::thread t3(thread_func);

	mainloop->put(econtext);
	mainloop->loop();
	return 0;
}
