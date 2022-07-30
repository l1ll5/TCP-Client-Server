#include "Buffer.hpp"

Buffer::Buffer(size_t size)
{
    buffer.resize(size);
    readerIndex = 0, writerIndex = 0;
}
Buffer::~Buffer()
{
}

size_t Buffer::readableBytes() const
{
    assert(writerIndex >= readerIndex);
    return writerIndex - readerIndex;
}
size_t Buffer::writableBytes() const
{
    return buffer.size() - writerIndex;
}
size_t Buffer::frontBytes() const
{
    return readerIndex;
}
char *Buffer::beginRead()
{
    return begin() + readerIndex;
}
const char *Buffer::beginRead() const
{
    return begin() + readerIndex;
}
char *Buffer::beginWrite()
{
    return begin() + writerIndex;
}
const char *Buffer::beginWrite() const
{
    return begin() + writerIndex;
}

void Buffer::append(const std::string &str) noexcept
{
    append(str.data(), str.length());
}
void Buffer::append(const char *data, size_t len) noexcept
{
    while (writableBytes() < len)
        expandTo(2 * size());
    std::copy(data, data + len, beginWrite());
    writerIndex += len;
}
void Buffer::append(const void *data, size_t len) noexcept
{
    append(static_cast<const char *>(data), len);
}

void Buffer::expandTo(size_t len) noexcept
{
    if (writableBytes() + frontBytes() < len)
    {
        buffer.resize(writerIndex + len);
    }
    else
    {
        size_t containBytes = readableBytes();
        std::copy(begin() + readerIndex, begin() + writerIndex, begin());
        readerIndex = 0, writerIndex = containBytes;
        if (containBytes < initSize)
            buffer.resize(initSize);
    }
}

std::string Buffer::retrieve(size_t len) noexcept
{
    if (len > readableBytes())
    {
        spdlog::warn("retrieved longer string than the buffer has, buffer has {}, but ask {}.", readableBytes(), len);
        len = readableBytes();
    }
    std::string ret = std::move(std::string(beginRead(), len));
    readerIndex += len;
    return ret;
}
std::string Buffer::retrieveAll() noexcept
{
    size_t len = readableBytes();
    std::string ret = std::move(std::string(beginRead(), len));
    readerIndex = writerIndex = 0;
    return ret;
}

char *Buffer::begin()
{
    return &*buffer.begin();
}
const char *Buffer::begin() const
{
    return &*buffer.begin();
}
size_t Buffer::size() const
{
    return buffer.size();
}

size_t Buffer::getLength()
{
    if (readableBytes() < 3)
        return 65536;
    return 256 * 256 * (unsigned char)buffer[readerIndex] + 256 * (unsigned char)buffer[readerIndex + 1] + (unsigned char)buffer[readerIndex + 2];
}

void Buffer::closeRead()
{
    readable = 0;
}
void Buffer::closeWrite()
{
    writeable = 0;
}
void Buffer::close()
{
    closeRead(), closeWrite();
}
bool Buffer::isReadable()
{
    return readable;
}
bool Buffer::isWriteable()
{
    return writeable;
}