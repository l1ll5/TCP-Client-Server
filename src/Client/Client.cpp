#include "Client.hpp"

TcpClientApi::TcpClientApi()
{
}
TcpClientApi::~TcpClientApi()
{
}

void TcpClientApi::setAutoReconnect(int milliseconds) noexcept { reconnect = milliseconds; }

void TcpClientApi::registerSpi(TcpClientSpi *spi) { spi_ = spi; }

int TcpClientApi::setServerAddress(const char *svrAddr) noexcept
{
    std::string addr(svrAddr);
    for (int i = 0; i < (int)addr.length(); i += 1)
        if (addr[i] == ':')
        {
            ip_ = std::move(addr.substr(0, i));
            port_ = std::move(addr.substr(i + 1, addr.length()));
            return 0;
        }
    spdlog::error("Server Address invalid! received {}", addr);
    return -1;
}
int TcpClientApi::sendMessage(int msgType, const void *data, size_t size) noexcept
{
    if (!connected)
        return -1;
    spdlog::trace("Client send, socketfd = {}", socketfd);
    std::string msg((char *)data, size);
    ClientLoop.addTask([this, msgType, msg]()
                       { ClientLoop.getConn(socketfd)->sendMessage(msgType, msg.data(), msg.length()); });
    return 0;
}

void TcpClientApi::shutdown() noexcept
{
    running = 0;
    ClientLoop.quit();
    close(socketfd);
}

void TcpClientApi::start() noexcept
{
    std::function<int(int)> connect_ = [&](int tmp)
    {
        struct sockaddr_in addr;
        bzero(&addr, sizeof(addr));

        inet_pton(AF_INET, ip_.c_str(), &(addr.sin_addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((unsigned short)atoi(port_.c_str()));

        int ret = connect(socketfd, (struct sockaddr *)&addr, sizeof(addr));
        if (ret < 0)
            spdlog::error("connect failed, return {}", ret);
        else
        {
            spdlog::info("Client connect successful, server ip = {}, port = {}", ip_, port_);
        }
        return (ret == 0);
    };

    std::function<int(int)> run = [&](int socketfd)
    {
        auto [readfd, writefd] = ClientLoop.insertClient(socketfd, false);

        std::function<int(int)> receiveMessage_ = [&](int socketfd)
        {
            while (true)
            {
                message msg = std::move(readNonblock(socketfd, ClientLoop.getRecv(socketfd)));
                if (msg.length > 0)
                {
                    if (msg.type == 0) //heartbeat message
                    {
                        heartbeatCount = 0;
                    }
                    else if (msg.length >= 5)
                    {
                        spdlog::info("Client received {} from socket {}", msg.context, socketfd);
                        spi_->onMessage(0, msg.context.data(), msg.context.length());
                    }
                }
                else
                    break;
            }

            return 0;
        };
        ClientLoop.registerReadCallback(receiveMessage_);
        std::function<int(int)> sendMessage_ = [&](int socketfd)
        {
            while (true)
            {
                if (writeNonblock(socketfd, ClientLoop.getSend(socketfd)) <= 0)
                    break;
            }
            return 0;
        };
        ClientLoop.registerWriteCallback(sendMessage_);
        std::function<int(int)> sendHeartbeat = [&](int tmp)
        {
            std::string packed = heartbeat();
            sendMessage(0, packed.c_str(), packed.length());
            return 0;
        };
        std::function<int(int)> recvHeartbeat = [&](int tmp)
        {
            spdlog::debug("recv heartbeat = {}", heartbeatCount);
            heartbeatCount -= 1;
            if (heartbeatCount == -3)
            {
                if (reconnect == -1) // auto reconnect is disabled
                {
                    spdlog::info("Connection lost, reconnect = -1, shutdown");
                    running = 0;
                    ClientLoop.quit();
                    connected = 0;
                    spi_->onDisconnected(1, "no reply of heartbeat, connection lost");
                    shutdown();
                }
                else
                {
                    spdlog::info("Connection lost, reconnect every {} milliseconds automatically", reconnect);
                    ClientLoop.registerTimerTask(reconnect / 1000, (reconnect % 1000) * 1000, connect_);
                }
            }
            return 0;
        };
        ClientLoop.eventRegister(readfd, EPOLLIN | EPOLLOUT);

        ClientLoop.registerTimerTask(5, 0, sendHeartbeat);
        ClientLoop.registerTimerTask(5, 0, recvHeartbeat);

        connected = 1;
        spi_->onConnected();
        ClientLoop.loop();
        return 0;
    };

    if (running)
    {
        spdlog::error("Client already running!");
        return;
    }
    running = 1;
    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd < 0)
        spdlog::error("create socket failed", socketfd);

    connect_(0);

    client = std::move(std::thread(run, socketfd));
}
