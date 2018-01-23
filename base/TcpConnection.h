#ifndef MOXIE_TCPCONNECTION_H
#define MOXIE_TCPCONNECTION_H
#include <memory>

#include <EventLoop.h>
#include <Econtext.h>
#include <Buffer.h>


namespace moxie {

class TcpConnection {
public:
    using TcpConnShreadPtr = boost::shared_ptr<TcpConnection>;
    using WriteDone = boost::function<void (TcpConnShreadPtr, Timestamp)>;
    using HasData = boost::function<void (TcpConnShreadPtr, Timestamp)>;
    using WillBeClose = boost::function<void (TcpConnShreadPtr, Timestamp)>;

    TcpConnection(EventLoop *loop, std::shared_ptr<Econtext> ect);
    
    void loop(EventLoop *loop);

    std::shared_ptr<Buffer> readbuf();
    std::shared_ptr<Buffer> writebuf();

    std::shared_ptr<Econtext> ect();
    void ect(std::shared_ptr<Econtext> ect);

    void writeDone(WriteDone writeDone);
    void hasData(HasData hasData);
    void willBeClose(WillBeClose beClose);
    WriteDone writeDone();
    HasData hasData();
    WillBeClose willBeClose();

    void shutdown();
private:
    EventLoop *loop_;
    std::shared_ptr<Econtext> ect_;
    std::shared_ptr<Buffer> readbuf_;
    std::shared_ptr<Buffer> writebuf_;
    bool closing_;

    WriteDone writeDone_;
    HasData hasData_;
    WillBeClose willBeClose_;
};

}
#endif // MOXIE_TCPCONNECTION_H
