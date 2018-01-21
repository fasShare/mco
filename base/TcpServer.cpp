#include <iostream>
#include <memory>
#include <unistd.h>
#include <new>
#include <functional>

#include <Socket.h>
#include <Log.h>
#include <EventLoop.h>
#include <TcpServer.h>
#include <EventLoopPool.h>
#include <TcpConnection.h>
#include <McoRoutine.h>

using moxie::Econtext;
using moxie::EventLoop;
using moxie::EventLoopPool;
using moxie::Events;
using moxie::McoStack;
using moxie::McoCallStack;
using moxie::McoRoutine;
using moxie::TcpServer;
using moxie::TcpConnection;

TcpServer::TcpServer(const NetAddress& addr, int listenBacklog) :
    loop_(nullptr),
    server_(AF_INET, SOCK_STREAM, 0),
    event_(new Events(server_.getSocket(), kReadEvent)),
    addr_(addr),
    listenBacklog_(listenBacklog) {
    assert(server_.bind(addr_));
    assert(server_.listen(listenBacklog_));
	LOGGER_TRACE("server listen fd = " << server_.getSocket());
}

bool TcpServer::start() { 
	loop_ = EventLoopPool::MainLoop();
	if (!loop_) {
		LOGGER_ERROR("Please check MoxieInit() was called!");
		return false;
	}

    auto econtext = std::make_shared<Econtext>();
    auto stack = new McoStack(loop_->tid());
    auto callstack = McoCallStack::CallStack(loop_->tid());
    auto aco = std::make_shared<McoRoutine>(std::bind(&TcpServer::accept, this));

    // 设置协成运行栈，协程切换栈
    aco->stack(stack);
    aco->callStack(callstack);

    econtext->fd(server_.getSocket());
    econtext->mco(aco);
    econtext->event(event_);
    econtext->loop(loop_);

    // 把TcpServer的监听事件注册到EventLoop中
    loop_->put(econtext);
    return true;
}

bool TcpServer::chain(std::shared_ptr<TcpConnection> conn) {
    //tcp io
    return true;        
}

void TcpServer::proc(std::weak_ptr<TcpConnection> wconn) {
    auto wconnt = std::move(wconn);
    while (true) {
        auto conn = wconnt.lock();
        if (!conn) {
            std::cout << gettid() << " lock nullptr" << std::endl;
            break;
        }

        auto ect = conn->ect();

        // do sth
        if (!chain(conn)) {
            break;
        }

        auto mco = ect->mco();
        if (mco) {
            mco->yield();
        }
    }
}

void TcpServer::accept() {
    LOGGER_TRACE("In TcpServer AcceptCall");
    while (true) {
        int fd = ::accept(server_.getSocket(), nullptr, nullptr);
        if (fd > 0) {
            auto loop = EventLoopPool::NextLoop();
            auto aev = std::make_shared<Events>(fd, kReadEvent);
            long tid = loop->tid();
            auto econtext = std::make_shared<Econtext>();
            auto conn = std::make_shared<TcpConnection>(loop, econtext);

            std::weak_ptr<TcpConnection> wconn = conn;

            auto aco = std::make_shared<McoRoutine>(std::bind(&TcpServer::proc, this, wconn));
            auto stack = new McoStack(tid);
            auto callstack = McoCallStack::CallStack(tid);

            aco->stack(stack);
            aco->callStack(callstack);

            econtext->fd(fd);
            econtext->mco(aco);
            econtext->event(aev);
            econtext->loop(loop);

            loop->put(econtext);
        } else {
            LOGGER_WARN("Accept error.");
        }
    }
}

TcpServer::~TcpServer() {
    loop_->quit();
    LOGGER_TRACE("TcpServer will be destroyed in process " << getpid());
}
