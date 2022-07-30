#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <functional>

#include "spdlog/spdlog.h"
#include "../../Utils/Utils.hpp"
#include "../../Buffer/Buffer.hpp"
#include "../ThreadPool/ThreadPool.hpp"
#include "../../Connection/Connection.hpp"

class TcpServerApi
{
public:
    TcpServerApi();
    ~TcpServerApi();

    TcpServerApi(const TcpServerApi &) = delete;
    TcpServerApi &operator=(const TcpServerApi &) = delete;

    void bindAddress(const char *sockaddr);
    void registerSpi(TcpServerSpi *spi) noexcept;
    void run();

private:
    std::string ip_, port_;
    ThreadPool pool;
    TcpServerSpi *spi_;

    bool isbound, isregistered;
};
