#ifndef MOXIE_TCPCONNECTION_H
#define MOXIE_TCPCONNECTION_H
#include <memory>

#include <EventLoop.h>
#include <Econtext.h>
#include <Buffer.h>


namespace moxie {

class TcpConnection {
public:
    TcpConnection(EventLoop *loop, std::shared_ptr<Econtext> ect);
    
    void loop(EventLoop *loop);

    std::shared_ptr<Buffer> readbuf();
    std::shared_ptr<Buffer> writebuf();

    std::shared_ptr<Econtext> ect();
    void ect(std::shared_ptr<Econtext> ect);
private:
    EventLoop *loop_;
    std::shared_ptr<Econtext> ect_;
    std::shared_ptr<Buffer> readbuf_;
    std::shared_ptr<Buffer> writebuf_;
};

}
#endif // MOXIE_TCPCONNECTION_H
