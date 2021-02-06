#ifndef BUFFER_H
#define BUFFER_H

#include "burger/base/copyable.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <algorithm>
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

class Buffer : public burger::copyable {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initalSize = kInitialSize);

    void swap(Buffer& rhs);
    size_t getReadableBytes() const { return writerIndex_ - readrIndex_; }
    size_t getWritableBytes() const { return buffer_.size() - writerIndex_; }
    size_t getPrependableBytes() const { return readrIndex_; }

    char* beginWrite() { return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }
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


private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { &*buffer_.begin(); }



private:
    std::vector<char> buffer_;
    size_t readrIndex_;
    size_t writerIndex_;

    static const char kCRLF[];  // 
};

} // namespace net

} // namespace burger



#endif // BUFFER_H