#include "Epoll.hpp"

Epoll::Epoll()
{
    epollfd_ = epoll_create1(EPOLL_CLOEXEC);
    spdlog::trace("create epoll, fd = {}", epollfd_);
}
Epoll::~Epoll()
{
    close(epollfd_);
}

void Epoll::run(epoll_event *Events, int size) noexcept
{
    int ret = epoll_wait(epollfd_, Events, size, 100);
    if (ret < 0)
        spdlog::error("epoll wait error, return {}", ret);
    else if (ret > 0)
    {
        for (int i = 0; i < ret; i += 1)
        {
            uint32_t events = Events[i].events;
            int fd = Events[i].data.fd;
            if (events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
            {
                readCallback(fd);
            }
            if (events & EPOLLOUT)
            {
                writeCallback(fd);
            }
            if (events & EPOLLHUP)
            {
                closeCallback(fd);
            }
        }
    }
}

void Epoll::registerReadCallback(std::function<int(int)> func)
{
    readCallback = func;
}
void Epoll::registerWriteCallback(std::function<int(int)> func)
{
    writeCallback = func;
}
void Epoll::registerCloseCallback(std::function<int(int)> func)
{
    closeCallback = func;
}
int Epoll::eventRegister(int socketfd, int eventType) noexcept
{
    spdlog::trace("{} register on {}", epollfd_, socketfd);
    struct epoll_event epollEvent;
    epollEvent.events = eventType;
    epollEvent.data.fd = socketfd;
    return epoll_ctl(epollfd_, EPOLL_CTL_ADD, socketfd, &epollEvent);
}
int Epoll::eventDelete(int socketfd)
{
    struct epoll_event epev = {0, {0}};
    return epoll_ctl(epollfd_, EPOLL_CTL_DEL, socketfd, &epev);
}