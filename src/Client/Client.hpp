#pragma once
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <cstddef>

#include "spdlog/spdlog.h"
#include "../EventLoop/EventLoop.hpp"
#include "../Connection/Connection.hpp"

class TcpClientSpi
{
public:
    virtual void onConnected() = 0;
    virtual void onDisconnected(int reasonCode, const char *reasonMsg) = 0;
    virtual void onMessage(int msgType, const void *payload, size_t size) = 0;
};

class TcpClientApi
{
public:
    TcpClientApi();
    ~TcpClientApi();

    TcpClientApi(const TcpClientApi &) = delete;
    TcpClientApi &operator=(const TcpClientApi &) = delete;

    void setAutoReconnect(int milliseconds = -1) noexcept;
    int setServerAddress(const char *svrAddr) noexcept;
    void registerSpi(TcpClientSpi *spi);
    void start() noexcept;
    void shutdown() noexcept;
    int sendMessage(int msgType, const void *data, size_t size) noexcept;

private:
    std::string ip_, port_;
    int socketfd;
    TcpClientSpi *spi_;
    bool running = 0;
    int reconnect = -1;
    std::thread client;
    EventLoop ClientLoop;
    int connected;
    int readfd_, writefd_;

    int heartbeatCount = 0;
};