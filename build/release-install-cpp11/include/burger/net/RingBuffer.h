#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "burger/net/IBuffer.h"
#include <assert.h>
#include <string>
#include <algorithm>
#include <vector>
#include <memory>
namespace burger {
namespace net {

/// Normal conditon:
/// @code
/// +-------------+----------------------+-------------------------------------+
/// | prependable |      blank part      |  readable bytes  |  writable bytes  |
/// |    bytes    |(CONTENT is read away)|     (CONTENT)    |                  |
/// +-------------+----------------------+------------------+------------------+
/// |                                    |                  |                  |
/// 0            <=                 readerIndex   <=   writerIndex    <=     size
/// @endcode

/// Ring conditon:
/// @code
/// +-------------+------------------+------------------+-----------------+------------------+
/// | prependable |  readable bytes  |  writable  bytes |   min_prepend   |  readable bytes  |
/// |    bytes    |    (CONTENT)     |                  | (kCheapPrepend) |    (CONTENT)     |
/// |             |                  |          prependable bytes         |                  |
/// +-------------+------------------+------------------------------------+------------------+
/// |             |                  |                                    |                  |
/// 0            <=             writerIndex          <=              readerIndex   <=       size
/// @endcode

class RingBuffer : public burger::net::IBuffer {
public:
    // static const size_t kCheapPrepend = 8;
    // static const size_t kInitialSize = 1024;
    using ptr = std::shared_ptr<RingBuffer>;
    explicit RingBuffer(size_t initalSize = kInitialSize);
    ~RingBuffer() = default;
    
    void swap(IBuffer& rhs);
    void swap(RingBuffer& rhs);

    size_t getReadableBytes() const;
    size_t getWritableBytes() const;
    size_t getPrependableBytes() const;

    /* to send the first part*/
    // size_t getLenToBufferEnd() const;

    char* beginRead() { return begin() + readrIndex_; }
    char* beginWrite() { return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }

    /* for test*/
    // size_t getWriterIndex() const { return writerIndex_; }
    // size_t getReaderIndex() const { return readrIndex_; }
    // size_t getTotalSize() const { return totalSize_; }
    
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

    char* end() { return &*buffer_.end(); }
    const char* end() const { return &*buffer_.end(); }

    char* continueRead() { return begin() + kCheapPrepend; }
    const char* continueRead() const { return begin() + kCheapPrepend; }

    void makeSpace(size_t len);

private:
    // std::vector<char> buffer_;
    // size_t readrIndex_;
    // size_t writerIndex_;
    size_t capacity_; //除去预留的空间外，实际可用的空间
    size_t totalSize_;
    bool hasData_;
};

}  // net
}  // burger

#endif // RINGBUFFER_H