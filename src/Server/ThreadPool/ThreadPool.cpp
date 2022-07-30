#include "ThreadPool.hpp"
ThreadPool::ThreadPool()
{
}

void ThreadPool::start() noexcept
{
    std::function<void(EventLoop *, TcpServerSpi *)> createChild = [](EventLoop *subThread, TcpServerSpi *spi_)
    {
        spdlog::trace("create child");
        std::function<int(int)> readCallback = [subThread, spi_](int socketfd)
        {
            while (true)
            {
                message msg = std::move(readNonblock(socketfd, subThread->getRecv(socketfd)));
                if (msg.length > 0)
                {
                    if (msg.type == 0)
                    {
                        //heartbeat
                        auto conn = subThread->getConn(socketfd);
                        conn->heartbeat += 1;
                        std::string packed = heartbeat();
                        conn->sendMessage(0, packed.c_str(), packed.length());
                    }
                    else if (msg.length >= 5)
                    {
                        spdlog::trace("Server reveived {} from socket {}, length = {}", msg.context, socketfd, msg.length);
                        spi_->onMessage(subThread->getConn(socketfd), 0, msg.context.data(), msg.context.length());
                    }
                }
                else
                    break;
            }
            return 0;
        };
        std::function<int(int)> closeCallback = [subThread, spi_](int socketfd)
        {
            spi_->onDisconnected(subThread->getConn(socketfd), 2, "unexpected close of socketfd");
            return 0;
        };
        std::function<int(int)> writeCallback = [subThread, spi_](int socketfd)
        {
            while (true)
            {
                int p = writeNonblock(socketfd, subThread->getSend(socketfd));
                if (p == -2)
                {
                    //close
                    auto conn = subThread->getConn(socketfd);
                    conn->forceClose();
                    spi_->onDisconnected(conn, 0, "closed safely");
                }
                if (p <= 0)
                    break;
            }
            return 0;
        };

        subThread->registerReadCallback(readCallback);
        subThread->registerWriteCallback(writeCallback);

        subThread->loop();
    };

    std::vector<std::thread> ioThreads;
    for (int i = 0; i < threadNum; i += 1)
    {
        EventLoop *subThread = new EventLoop;

        subThread->setSpi(spi_);
        subLoop.emplace_back(subThread);
        ioThreads.emplace_back(std::move(std::thread(createChild, subThread, spi_)));
    }
    mainLoop.loop();
}
void ThreadPool::registerMainLoop(int socketfd, int eventType, std::function<int(int)> callBackFunc) noexcept
{
    mainLoop.registerReadCallback(callBackFunc);
    mainLoop.eventRegister(socketfd, EPOLLIN);
}

int ThreadPool::receiveFd(int socketfd) noexcept
{
    EventLoop *ioThread = subLoop[nextThread];
    //receive connection from the main process
    subLoop[nextThread]->addTask([this, ioThread, socketfd]()
                                 { ioThread->insertClient(socketfd); });
    (nextThread += 1) %= threadNum;
}