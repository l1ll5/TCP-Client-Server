#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include "spdlog/spdlog.h"

class Buffer
{
public:
    Buffer(size_t size = initSize);
    ~Buffer();

    size_t readableBytes() const;
    size_t writableBytes() const;

    void append(const std::string &str) noexcept;
    void append(const char *data, size_t len) noexcept;
    void append(const void *data, size_t len) noexcept;

    char *beginRead();
    const char *beginRead() const;
    char *beginWrite();
    const char *beginWrite() const;
    std::string retrieve(size_t len) noexcept;
    std::string retrieveAll() noexcept;
    size_t readerIndex;
    size_t writerIndex;
    size_t size() const;
    void expandTo(size_t len) noexcept;
    size_t getLength();

    void closeRead();
    void closeWrite();
    void close();
    bool isReadable();
    bool isWriteable();

private:
    char *begin(); //get the start address of vector buffer.
    const char *begin() const;
    std::vector<char> buffer;
    static const size_t initSize = 8192;

    size_t frontBytes() const;
    bool readable = 1, writeable = 1;
};