
#include "Utils.hpp"
std::string packMessage(std::string input, int type) noexcept
{
    std::string msg;
    int len = input.length() + 5;
    msg.push_back(len / 256 / 256);
    msg.push_back((len / 256) % 256);
    msg.push_back(len % 256);

    msg.push_back(type);
    char check = 0;
    for (auto &x : input)
        check ^= x;
    msg.push_back(check);
    msg = msg + input;
    return msg;
}
std::string heartbeat()
{
    std::string msg;
    msg.push_back(0);
    msg.push_back(0);
    msg.push_back(5);
    msg.push_back(0);
    msg.push_back(0);
    return msg;
}
message parseMessage(std::string Package) noexcept
{
    message msg;
    if (Package.length() < 5)
    {
        spdlog::error("too short message = {}, len = {}", Package, Package.length());
        return msg;
    }
    msg.length = 256 * 256 * (unsigned char)Package[0] + 256 * (unsigned char)Package[1] + (unsigned char)Package[2];
    if (Package.length() != msg.length)
    {
        spdlog::error("the length isn't right!");
        spdlog::error("package length = {}, msg.length = {}", Package.length(), msg.length);
    }
    msg.type = Package[3];
    if (msg.type < 0 or msg.type > 1)
    {
        spdlog::error("message type is invaild = {}, length = {}", msg.type, msg.length);
    }
    msg.context = std::move(Package.substr(5));
    char check = 0;
    for (auto &x : msg.context)
        check ^= x;
    if (Package[4] != check)
    {
        spdlog::error("wrong message");
        spdlog::error(msg.context);
        return message();
    }
    msg.checkcode = Package[4];
    return msg;
}
message readNonblock(int socketfd, std::shared_ptr<Buffer> Buffer) noexcept
{
    if (Buffer->isWriteable() == false)
        return {};
    if (Buffer->writableBytes() == 0)
        Buffer->expandTo(Buffer->size() * 2);
    int n = recv(socketfd, Buffer->beginWrite(), Buffer->writableBytes(), MSG_DONTWAIT | MSG_NOSIGNAL);
    if (n > 0)
    {
        Buffer->writerIndex += n;
    }
    if (Buffer->readableBytes() >= 5)
    {
        int len = Buffer->getLength();
        spdlog::trace("package len = {}", len);
        if (len <= Buffer->readableBytes())
        {
            return parseMessage(std::move(Buffer->retrieve(len)));
        }
        return {};
    }
    return {};
}
int writeNonblock(int socketfd, std::shared_ptr<Buffer> Buffer) noexcept
{
    if (Buffer->isReadable() == false)
        return 0;
    int n = send(socketfd, Buffer->beginRead(), Buffer->readableBytes(), MSG_DONTWAIT | MSG_NOSIGNAL);
    if (Buffer->isWriteable() == false and Buffer->readableBytes() == 0)
        return -2;
    if (n > 0)
    {
        Buffer->readerIndex += n;
        return n;
    }
    return n;
}
