#ifndef IBUFFER_H
#define IBUFFER_H

#include "burger/base/copyable.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <memory>
#include <algorithm>
namespace burger {
namespace net {

class IBuffer : public burger::copyable {
public:
    using ptr = std::shared_ptr<IBuffer>;
    static const size_t kCheapPrepend;
    static const size_t kInitialSize;

    IBuffer(size_t initalSize) :
    buffer_(kCheapPrepend + initalSize),
    readrIndex_(kCheapPrepend),
    writerIndex_(kCheapPrepend) {}

    virtual ~IBuffer() = default;

    virtual void swap(IBuffer& rhs) = 0;
    virtual size_t getReadableBytes() const = 0;
    virtual size_t getWritableBytes() const = 0;
    virtual size_t getPrependableBytes() const = 0;

    virtual char* beginRead() = 0; 
    virtual char* beginWrite() = 0;
    virtual const char* beginWrite() const= 0;
    virtual void hasWritten(size_t len) = 0;
    // 获取可读数据的指针地址，类似vector中的data()
    virtual const char* peek() const = 0;
    // 从起始位置开始查找 \r\n
    virtual const char* findCRLF() const= 0;
    // 从start位置开始查找 \r\n
    virtual const char* findCRLF(const char* start) const= 0;
    // 查找换行符
    virtual const char* findEOL() const= 0;
    virtual const char* findEOL(const char* start) const= 0;
    
    virtual void retrieve(size_t len) = 0;
    virtual void retrieveAll() = 0;
    virtual void retrieveUntil(const char* end) = 0;
    virtual void retrieveInt64() = 0; 
    virtual void retrieveInt32() = 0; 
    virtual void retrieveInt16() = 0; 
    virtual void retrieveInt8() = 0; 
    virtual std::string retrieveAsString(size_t len) = 0;
    virtual std::string retrieveAllAsString() = 0;

    virtual void append(const std::string& str) = 0;
    virtual void append(const char* data, size_t len) = 0;
    virtual void append(const void* data, size_t len) = 0;

    virtual void prepend(const void* data, size_t len) = 0;

    virtual void ensureWritableBytes(size_t len) = 0;
    
    virtual void shrink(size_t reserve) = 0;

    // Peek int from network endian，但不从buffer中删除
    virtual int64_t peekInt64() const = 0;
    virtual int32_t peekInt32() const = 0;
    virtual int16_t peekInt16() const = 0;
    virtual int8_t peekInt8() const = 0;
    // Read int from network endian, 从buffer里读出后删除
    virtual int64_t readInt64() = 0;
    virtual int32_t readInt32() = 0;
    virtual int16_t readInt16() = 0;
    virtual int8_t readInt8() = 0;
    // 使用网络字节序写入一个整数
    virtual void appendInt64(int64_t x) = 0;
    virtual void appendInt32(int32_t x) = 0;
    virtual void appendInt16(int16_t x) = 0;
    virtual void appendInt8(int8_t x) = 0;
    
    virtual ssize_t readFd(int fd, int& savedErrno) = 0;

protected:
    virtual char* begin() = 0;
    virtual const char* begin() const = 0;
    virtual void makeSpace(size_t len) = 0;

protected:
    std::vector<char> buffer_;
    size_t readrIndex_;
    size_t writerIndex_;

    static const char kCRLF[];

};

// const size_t IBuffer::kCheapPrepend;
// const size_t IBuffer::kInitialSize;


}
}

#endif // IBUFFER_H