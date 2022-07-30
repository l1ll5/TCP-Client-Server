#pragma once
#include <memory>
#include <any>
#include "../Buffer/Buffer.hpp"
#include "../Utils/Utils.hpp"
#include "../EventLoop/EventLoop.hpp"
class EventLoop;
class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection(int sendfd, int recvfd, int id, EventLoop *Loop);
    ~Connection();
    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

    std::any context;

    void sendMessage(int msgType, const void *payload, int len) noexcept;
    void close(bool force = false) noexcept;
    bool isClosed() const noexcept;
    std::shared_ptr<Buffer> sendBuffer, recvBuffer;
    int heartbeat;
    int sendfd_;
    int recvfd_;
    void forceClose() noexcept;

private:
    bool isClosed_;
    EventLoop *Loop_;
    int id_;
    std::mutex lock;
};

class TcpServerSpi
{
public:
    virtual void onAccepted(std::shared_ptr<Connection>) = 0;
    virtual void onDisconnected(std::shared_ptr<Connection>, int reason, const char *reasonMsg) = 0;
    virtual void onMessage(std::shared_ptr<Connection>, int msgType, const void *payload, size_t size) = 0;
};