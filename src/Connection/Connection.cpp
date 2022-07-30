#include "Connection.hpp"
Connection::Connection(int sendfd, int recvfd, int id, EventLoop *Loop)
{
    spdlog::debug("Connection {} created", id);
    sendfd_ = sendfd, recvfd_ = recvfd, id_ = id, Loop_ = Loop;
    sendBuffer.reset(new Buffer);
    recvBuffer.reset(new Buffer);
}
Connection::~Connection()
{
    //和Connection有关的资源在析构时自动回收
    spdlog::debug("Connection {} released", id_);
}
bool Connection::isClosed() const noexcept
{
    return isClosed_;
}

void Connection::forceClose() noexcept
{
    sendBuffer->close(), recvBuffer->close(); //avoid send & recv on the closed socket
    Loop_->logoutEvent(id_);                  //logout the events on epoll
    close(sendfd_);                           //close the socket directly
    close(recvfd_);
    Loop_->eraseConn(id_); //release itself
    return;
}
void Connection::close(bool force) noexcept
{
    {
        std::lock_guard<std::mutex> mlock(lock);
        if (isClosed_)
            return;
        isClosed_ = 1;
    }

    if (force) //triggered while the connection is lost or under user's requirement
    {
        forceClose();
        return;
    }
    recvBuffer->close(); // Stop the IO read operation.
    sendBuffer->closeWrite();
    //call forceClose while the send buffer is empty
}

void Connection::sendMessage(int msgType, const void *payload, int len) noexcept
{
    if (isClosed_) //discard data from `sendMessage` after close
        return;
    sendBuffer->append(payload, len);
    writeNonblock(sendfd_, sendBuffer);
}