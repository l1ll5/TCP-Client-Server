#include "EventLoop.hpp"

EventLoop::EventLoop() {}
EventLoop::~EventLoop() {}

void EventLoop::runTasks() noexcept
{
    if (tasks.size())
    {
        std::vector<std::function<void()>> task;
        {
            std::lock_guard<std::mutex> mlock(lock);
            task.swap(tasks);
        }
        for (auto &it : task)
            it();
    }
}

void EventLoop::loop() noexcept
{
    running_ = true;
    struct epoll_event epollEvents[EPOLL_EVENTS];
    while (running_)
    {
        runTasks();
        timer.getShortest();
        poll_.run(epollEvents, EPOLL_EVENTS);
    }
}
void EventLoop::quit()
{
    running_ = false;
}

void EventLoop::logoutEvent(int id) noexcept
{
    auto it = conn.find(id);
    poll_.eventDelete((*it).second->sendfd_);
    poll_.eventDelete((*it).second->recvfd_);
}

void EventLoop::eraseConn(int id)
{
    conn.erase(conn.find(id));
}

int EventLoop::checkConn(int id) noexcept
{
    /* check the interval of last heartbeat */
    spdlog::trace("checkConn {}", id);
    auto it = conn.find(id);
    (*it).second->heartbeat -= 1;
    spdlog::debug("heartbeat = {}", (*it).second->heartbeat);
    if ((*it).second->heartbeat == -3)
    {
        spdlog::info("timeout, close {}", id);
        spi_->onDisconnected((*it).second, 1, "no reply of heartbeat, connection lost");
        (*it).second->close(true);
        eraseConn(id);
        return 1;
    }
    return 0;
}

std::pair<int, int> EventLoop::insertClient(int socketfd, bool isClient) noexcept
{
    /*
        sub thread receive a client from the main thread
        register the IO callback functions and setup the connection
        isClient = true : main thread add a client to the IO sub threads
    */
    if (isClient)
        if (poll_.eventRegister(socketfd, EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLOUT) < 0)
            spdlog::error("register failed! fd = {}", socketfd);
    std::pair<int, int> ret(socketfd, 0);
    connCount += 1;
    fdToConn[socketfd] = connCount;
    socketfd = dup(socketfd);
    ret.second = socketfd;
    fdToConn[socketfd] = connCount;
    if (isClient)
        if (poll_.eventRegister(socketfd, EPOLLOUT) < 0)
            spdlog::error("register failed! fd = {}", socketfd);

    conn[connCount] = std::shared_ptr<Connection>(new Connection(ret.first, ret.second, connCount, this));
    //check if received the heartbeat pack every 5 seconds
    if (isClient)
    {
        timerId[connCount] = timer.insert(5, 0, std::bind(&EventLoop::checkConn, this, std::placeholders::_1), connCount, true);
        spi_->onAccepted(conn[connCount]);
    }
    return ret;
}

void EventLoop::addTask(std::function<void()> callBack) noexcept
{
    std::lock_guard<std::mutex> mlock(lock);
    tasks.emplace_back(callBack);
    return;
}
void EventLoop::registerTimerTask(int sec, int usec, std::function<int(int)> func) noexcept
{
    timer.insert(sec, usec, func, 0, true);
}

std::shared_ptr<Buffer> EventLoop::getRecv(int socketfd) noexcept
{
    if (fdToConn.count(socketfd) == 0)
    {
        spdlog::error("getBuff error");
    }
    return conn[fdToConn[socketfd]]->recvBuffer;
}
std::shared_ptr<Buffer> EventLoop::getSend(int socketfd) noexcept
{
    if (fdToConn.count(socketfd) == 0)
    {
        spdlog::error("getBuff error");
    }
    return conn[fdToConn[socketfd]]->sendBuffer;
}
std::shared_ptr<Connection> EventLoop::getConn(int socketfd) noexcept
{
    int p = fdToConn[socketfd];
    if (conn.count(p) == 0)
    {
        spdlog::error("getConn error");
    }
    return conn[p];
}
void EventLoop::registerReadCallback(std::function<int(int)> func)
{
    poll_.registerReadCallback(func);
}
void EventLoop::registerWriteCallback(std::function<int(int)> func)
{
    poll_.registerWriteCallback(func);
}
void EventLoop::registerCloseCallback(std::function<int(int)> func)
{
    poll_.registerCloseCallback(func);
}
int EventLoop::eventRegister(int socketfd, int eventType)
{
    return poll_.eventRegister(socketfd, eventType);
}