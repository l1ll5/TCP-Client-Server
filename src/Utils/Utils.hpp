
#pragma once
#include <functional>
#include "../Buffer/Buffer.hpp"
#include "spdlog/spdlog.h"
#include <sys/socket.h>
struct message
{
    message() {}
    int length = 0;      // 3 byte
    int type = 0;        // 1 byte
    int checkcode = 0;   // 1 byte
    std::string context; // max 256^3 - 6 byte
};
std::string heartbeat();
message readNonblock(int socketfd, std::shared_ptr<Buffer> Buffer) noexcept;
int writeNonblock(int socketfd, std::shared_ptr<Buffer> Buffer) noexcept;
message parseMessage(std::string Package) noexcept;
std::string packMessage(std::string input, int type) noexcept;