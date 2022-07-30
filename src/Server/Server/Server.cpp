#include "Server.hpp"

TcpServerApi::TcpServerApi()
{
}
TcpServerApi::~TcpServerApi()
{
}

void TcpServerApi::bindAddress(const char *sockaddr)
{
    std::string addr(sockaddr);
    for (int i = 0; i < (int)addr.length(); i += 1)
        if (addr[i] == ':')
        {
            isbound = true;
            ip_ = std::move(addr.substr(0, i));
            port_ = std::move(addr.substr(i + 1, addr.length()));
            return;
        }
    spdlog::error("bindAddress failed, the address {} is invaild!", addr);
}
void TcpServerApi::registerSpi(TcpServerSpi *spi) noexcept
{
    spdlog::info("successfully register Spi");
    spi_ = spi;
    isregistered = true;
}
void TcpServerApi::run()
{
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listenfd < 0)
        spdlog::error("failed to build the listenfd, return {}", listenfd);

    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    inet_pton(AF_INET, ip_.c_str(), &(addr.sin_addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)atoi(port_.c_str()));

    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        spdlog::error("bind error");

    if (listen(listenfd, SOMAXCONN) < 0)
        spdlog::error("listen error");

    std::function<int(int)> acceptConn = [&](int socketfd)
    {
        spdlog::trace("accept in progress");
        struct sockaddr_storage addr;
        socklen_t socklen(sizeof(addr));

        int clientfd = accept4(socketfd, (struct sockaddr *)&addr, &socklen, SOCK_NONBLOCK);
        if (clientfd < 0)
            spdlog::error("accept fail, fd is : {}", socketfd);

        spdlog::trace("accept successful, clientfd is : {}", clientfd);
        pool.receiveFd(clientfd);
        return 0;
    };
    pool.registerMainLoop(listenfd, EPOLLIN, acceptConn);

    spdlog::info("server running at port : {}", port_);

    pool.setThreadNum(4);
    pool.setSpi(spi_);
    pool.start();
}