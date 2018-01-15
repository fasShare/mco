#include <McoRoutine.h>
#include <McoStack.h>
#include <McoCallStack.h>
#include <SigIgnore.h>
#include <EventLoop.h>
#include <Socket.h>
#include <functional>
#include <NetAddress.h>
#include <EventLoopPool.h>

using namespace moxie;

SigIgnore signor;

void accept_call(std::shared_ptr<Socket> acp) {
	while (true) {
		NetAddress addr;
		auto client = acp->accept(addr);
		if (client > 0) {
			char buf[1024];
			int ret = read(client, buf, 1023);
			if (ret > 0) {
				buf[ret] = 0;
				LOGGER_TRACE("recv:" << buf);
			}
			close(client);
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

	auto aco = std::make_shared<McoRoutine>(std::bind(accept_call, acp));
	auto stack = new McoStack(mainloop->tid());
	auto callstack = McoCallStack::CallStack(mainloop->tid());
	aco->stack(stack);
	aco->callStack(callstack);

	auto econtext = std::make_shared<Econtext>();
	econtext->fd(acp->getSocket());
	econtext->mco(aco);
	econtext->event(aev);

	mainloop->put(econtext);
	mainloop->loop();
	return 0;
}
