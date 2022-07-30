#pragma once
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <functional>
#include <thread>
#include "spdlog/spdlog.h"
#include "../../Utils/Utils.hpp"
#include "../../EventLoop/EventLoop.hpp"
#include "../../Connection/Connection.hpp"

class ThreadPool
{
public:
    ThreadPool();
    void start() noexcept;
    void setThreadNum(int num) { threadNum = num; }
    void registerMainLoop(int socketfd, int eventType, std::function<int(int)> callBackFunc) noexcept;
    int receiveFd(int socketfd) noexcept;
    void setSpi(TcpServerSpi *spi) { spi_ = spi; }

private:
    std::vector<int> children;
    EventLoop mainLoop;
    int threadNum = 0;
    int nextThread = 0;
    TcpServerSpi *spi_;
    std::vector<EventLoop *> subLoop;
};
