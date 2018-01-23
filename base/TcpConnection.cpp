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
    writebuf_(std::make_shared<Buffer>(1024)),
    closing_(false) {
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

void TcpConnection::writeDone(const WriteDone& writeDone) {
    writeDone_ = writeDone
}

void TcpConnection::hasData(const HasData& hasData) {
    hasData_ = hasData;
}

void TcpConnection::willBeClose(const WillBeClose& beClose) {
    willBeClose_ = beClose;
}

TcpConnection::WriteDone TcpConnection::writeDone() {
    return writeDone_;
}

TcpConnection::HasData TcpConnection::hasData() {
    return hasData_;
}

TcpConnection::WillBeClose TcpConnection::willBeClose() {
    return willBeClose_;
}

void TcpConnection::shutdown() {
    closing_ = true;
}

bool TcpConnection::closing() {
    return closing_;
}
