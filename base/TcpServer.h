#ifndef MOXIE_TCPSERVER_H
#define MOXIE_TCPSERVER_H
#include <memory>
#include <map>
#include <vector>
#include <queue>

#include <Socket.h>
#include <Events.h>
#include <EventLoop.h>
#include <NetAddress.h>
#include <SigIgnore.h>
#include <TcpConnection.h>

namespace moxie {

class TcpServer {
public:
    TcpServer(const NetAddress& addr, int listenBacklog = 100);
    ~TcpServer();

    bool start();
    bool chain(std::shared_ptr<TcpConnection>);
    void proc(std::weak_ptr<TcpConnection>);
    void accept();
private:
    EventLoop *loop_;
    Socket server_;
    std::shared_ptr<Events> event_;
    NetAddress addr_;
    const uint listenBacklog_;
};

}
#endif // MOXIE_TCPSERVER_H
