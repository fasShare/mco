#include <TcpConnection.h>

using moxie::TcpConnection;
using moxie::EventLoop;
using moxie::Events;
using moxie::McoRoutine;
using moxie::Econtext;
using moxie::Buffer;

TcpConnection::TcpConnection(EventLoop *loop, std::shared_ptr<Econtext> ect) :
    loop_(loop),
    ect_(ect),
    readbuf_(std::make_shared<Buffer>(1024)),
    writebuf_(std::make_shared<Buffer>(1024)) {
}

void TcpConnection::loop(EventLoop *loop) {
    loop_ = loop;
}

std::shared_ptr<Buffer> TcpConnection::readbuf() {
    return readbuf_;
}

std::shared_ptr<Buffer> TcpConnection::writebuf() {
    return writebuf_;
}

std::shared_ptr<Econtext> TcpConnection::ect() {
    return ect_;
}

void TcpConnection::ect(std::shared_ptr<Econtext> ect) {
    ect_ = ect;
}


