// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

#ifndef BUFFER_H
#define BUFFER_H

#include "burger/base/copyable.h"
#include "burger/net/IBuffer.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <memory>

namespace burger {
namespace net {

/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

// 非线程安全
class Buffer : public burger::net::IBuffer {
public:
    // static const size_t kCheapPrepend = 8;
    // static const size_t kInitialSize = 1024;
    using ptr = std::shared_ptr<Buffer>;
    explicit Buffer(size_t initalSize = kInitialSize);
    ~Buffer() = default;

    void swap(IBuffer& rhs);
    void swap(Buffer& rhs);
    size_t getReadableBytes() const { return writerIndex_ - readrIndex_; }
    size_t getWritableBytes() const { return buffer_.size() - writerIndex_; }
    size_t getPrependableBytes() const { return readrIndex_; }

    char* beginRead() { return begin() + readrIndex_; }
    char* beginWrite() { return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }
    void hasWritten(size_t len);
    // 获取可读数据的指针地址，类似vector中的data()
    const char* peek() const { return begin() + readrIndex_; }
    // 从起始位置开始查找 \r\n
    const char* findCRLF() const;
    // 从start位置开始查找 \r\n
    const char* findCRLF(const char* start) const;
    // 查找换行符
    const char* findEOL() const;
    const char* findEOL(const char* start) const;
    
    void retrieve(size_t len);
    void retrieveAll();
    void retrieveUntil(const char* end);
    void retrieveInt64() { retrieve(sizeof(int64_t)); }
    void retrieveInt32() { retrieve(sizeof(int32_t)); }
    void retrieveInt16() { retrieve(sizeof(int16_t)); }
    void retrieveInt8() { retrieve(sizeof(int8_t)); }
    std::string retrieveAsString(size_t len);
    std::string retrieveAllAsString();

    void append(const std::string& str);
    void append(const char* data, size_t len);
    void append(const void* data, size_t len);

    void prepend(const void* data, size_t len);

    void ensureWritableBytes(size_t len);
    
    void shrink(size_t reserve);

    // Peek int from network endian，但不从buffer中删除
    int64_t peekInt64() const;
    int32_t peekInt32() const;
    int16_t peekInt16() const;
    int8_t peekInt8() const;
    // Read int from network endian, 从buffer里读出后删除
    int64_t readInt64();
    int32_t readInt32();
    int16_t readInt16();
    int8_t readInt8();
    // 使用网络字节序写入一个整数
    void appendInt64(int64_t x);
    void appendInt32(int32_t x);
    void appendInt16(int16_t x);
    void appendInt8(int8_t x);
    
    ssize_t readFd(int fd, int& savedErrno);

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    void makeSpace(size_t len);

// private:
//     std::vector<char> buffer_;
//     // to know whu use size_t other than char*
//     size_t readrIndex_;
//     size_t writerIndex_;

//     static const char kCRLF[];  // 
};

} // namespace net

} // namespace burger



#endif // BUFFER_H