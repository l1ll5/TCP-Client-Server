#pragma once
#include <map>
#include <mutex>
#include <vector>
#include <unistd.h>
#include <sys/param.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "spdlog/spdlog.h"
#include "../Timer/Timer.hpp"
#include "../Utils/Utils.hpp"
#include "../Epoll/Epoll.hpp"
#include "../Connection/Connection.hpp"
class TcpServerSpi;
class Connection;
class EventLoop
{
public:
    EventLoop();
    ~EventLoop();
    void loop() noexcept;
    void quit();

    void addTask(std::function<void()> callBack) noexcept;
    void setSpi(TcpServerSpi *spi) { spi_ = spi; }

    std::pair<int, int> insertClient(int socketfd, bool isClient = true) noexcept;

    std::shared_ptr<Buffer> getRecv(int socketfd) noexcept;
    std::shared_ptr<Buffer> getSend(int socketfd) noexcept;
    std::shared_ptr<Connection> getConn(int socketfd) noexcept;

    void registerReadCallback(std::function<int(int)> func);
    void registerWriteCallback(std::function<int(int)> func);
    void registerCloseCallback(std::function<int(int)> func);
    void registerTimerTask(int sec, int usec, std::function<int(int)> func) noexcept;
    int eventRegister(int socketfd, int eventType);

    void logoutEvent(int id) noexcept;
    void eraseConn(int id);

private:
    TcpServerSpi *spi_;
    bool running_;
    static const int EPOLL_EVENTS = 1024;

    Timer timer;
    Epoll poll_;

    int checkConn(int id) noexcept;
    void runTasks() noexcept;

    std::vector<std::function<void()>> tasks;

    std::map<int, std::shared_ptr<Connection>> conn;
    std::map<int, int> fdToConn, timerId;
    int connCount;

    std::mutex lock;
    int heartbeatCount;
};