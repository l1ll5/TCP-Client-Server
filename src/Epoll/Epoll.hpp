#include <sys/epoll.h>
#include <unistd.h>
#include "spdlog/spdlog.h"
class Epoll
{
public:
    Epoll();
    ~Epoll();

    void run(epoll_event *Events, int size) noexcept;
    void registerReadCallback(std::function<int(int)> func);
    void registerWriteCallback(std::function<int(int)> func);
    void registerCloseCallback(std::function<int(int)> func);
    int eventRegister(int socketfd, int eventType) noexcept;

    int eventDelete(int socketfd);

private:
    int epollfd_;
    std::function<int(int)> readCallback, writeCallback, closeCallback;
};
