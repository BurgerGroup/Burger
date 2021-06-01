#include "RingBuffer.h"
#include "burger/net/Endian.h"
#include <sys/uio.h>
#include "SocketsOps.h"
#include "burger/base/Util.h"
#include <iostream>

using namespace burger;
using namespace burger::net;

// const char IBuffer::kCRLF[] = "\r\n";
// const char RingBuffer::kCRLF[] = "\r\n";
// const size_t RingBuffer::kCheapPrepend;
// const size_t RingBuffer::kInitialSize;

RingBuffer::RingBuffer(size_t initalSize) :
    IBuffer(initalSize),
    capacity_(initalSize),
    totalSize_(kCheapPrepend + initalSize),
    hasData_(false)             {
    assert(getReadableBytes() == 0);
    assert(getWritableBytes() == initalSize);
    assert(getPrependableBytes() == kCheapPrepend);
    assert(hasData_ == false);
}

void RingBuffer::swap(IBuffer& rhs) {
    RingBuffer& that = dynamic_cast<RingBuffer&>(rhs);
    buffer_.swap(that.buffer_);
    std::swap(readrIndex_, that.readrIndex_);
    std::swap(writerIndex_, that.writerIndex_);
    std::swap(capacity_, that.capacity_);
    std::swap(hasData_, that.hasData_);
    std::swap(totalSize_, that.totalSize_);
}

void RingBuffer::swap(RingBuffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readrIndex_, rhs.readrIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
    std::swap(capacity_, rhs.capacity_);
    std::swap(hasData_, rhs.hasData_);
    std::swap(totalSize_, rhs.totalSize_);
}

size_t RingBuffer::getReadableBytes() const {
    if(!hasData_) return 0;

    if(writerIndex_ > readrIndex_) {
        return writerIndex_ - readrIndex_;
    }
    else {   
        return capacity_ - (readrIndex_ - writerIndex_);
    }
}

size_t RingBuffer::getWritableBytes() const {
    if(!hasData_) return capacity_;

    if(writerIndex_ > readrIndex_) {
        size_t writable = totalSize_ - writerIndex_; 
        if(readrIndex_ > 2 * kCheapPrepend) {
            writable += readrIndex_ - 2 * kCheapPrepend;  // 在readrIndex前必须有kCheapPrepend
        }
        return writable;
    }
    else {
        if(readrIndex_ - writerIndex_ >= kCheapPrepend) {
            return readrIndex_ - writerIndex_ - kCheapPrepend;
        }
        else {
            return 0;
        }
    }
}

size_t RingBuffer::getPrependableBytes() const {
    if(!hasData_) {
        return kCheapPrepend;
    }
    
    if(writerIndex_ > readrIndex_ || writerIndex_ == kCheapPrepend) {  // 当刚好写满时，writerIndex为8，此时前面还没有数据，依然可以prepend
        return readrIndex_;
    }
    else {
        return readrIndex_ - writerIndex_;
    }
}

// size_t RingBuffer::getLenToBufferEnd() const {
//     return totalSize_ -  readrIndex_;
// }

void RingBuffer::hasWritten(size_t len) {
    assert(len <= getWritableBytes());
    if(len > 0) hasData_ = true;

    writerIndex_ += len;
    writerIndex_ %= capacity_;
}

const char* RingBuffer::findCRLF() const {
    if(!hasData_) return nullptr;

    const char* crlf = nullptr;
    if(writerIndex_ > readrIndex_) {
        crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
    }
    else {
        crlf = std::search(peek(), end(), kCRLF, kCRLF+2);
        if(crlf == end()) {
            if(*(end() - 1) == kCRLF[0] && *(continueRead()) == kCRLF[1]) {  // 有可能CRLF被末尾隔断
                crlf = end() - 1;
            }
            else {
                crlf = std::search(continueRead(), beginWrite(), kCRLF, kCRLF+2);
            }
        }
    }
    return crlf == beginWrite() ? nullptr : crlf;
}

const char* RingBuffer::findCRLF(const char* start) const {
    if(!hasData_) return nullptr;

    const char* crlf = nullptr;
    if(writerIndex_ > readrIndex_ || start <= beginWrite()) {     // 正常情况
        crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
    }
    else {  
        crlf = std::search(start, end(), kCRLF, kCRLF+2);
        if(crlf == end()) {
            if(*(end() - 1) == kCRLF[0] && *(continueRead()) == kCRLF[1]) {  // 有可能CRLF被末尾隔断
                crlf = end() - 1;
            }
            else {
                crlf = std::search(continueRead(), beginWrite(), kCRLF, kCRLF+2);
            }
        }
    }
    return crlf == beginWrite() ? nullptr : crlf;
}

const char* RingBuffer::findEOL() const {
    if(!hasData_) return nullptr;

    const void* eol = nullptr;
    if(writerIndex_ > readrIndex_) {
        eol = memchr(peek(), '\n', getReadableBytes());
    }
    else {
        eol = memchr(peek(), '\n', end() - peek());
        if(!eol) {
            eol = memchr(continueRead(), '\n', beginWrite() - continueRead());
        }
    }
    return static_cast<const char*>(eol);
}

const char* RingBuffer::findEOL(const char* start) const {
    if(!hasData_) return nullptr;

    const void* eol = nullptr;
    if(writerIndex_ > readrIndex_ || start <= beginWrite()) {
        eol = memchr(start, '\n', beginWrite() - start);
    }
    else {
        eol = memchr(start, '\n', end() - start);
    }
    return static_cast<const char*>(eol);
}

void RingBuffer::retrieve(size_t len) {
    assert(len <= getReadableBytes());
    if(len < getReadableBytes()) {
        readrIndex_ += len;
        readrIndex_ %= capacity_;
    } else {
        retrieveAll();
    }
}

void RingBuffer::retrieveAll() {
    readrIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
    hasData_ = false;
}

void RingBuffer::retrieveUntil(const char* rawptr) {
    if(!hasData_) return;

    if(peek() <= rawptr) {
        retrieve(rawptr - peek());
    }
    else {
        assert(rawptr <= beginWrite());
        retrieve(end() - beginRead());
        retrieve(rawptr - continueRead());
    }
}

std::string RingBuffer::retrieveAsString(size_t len) {
    assert(len <= getReadableBytes());

    std::string res;
    if(writerIndex_ > readrIndex_ || readrIndex_ + len <= totalSize_) {
        res.append(peek(), len);
    }
    else {
        size_t fisrtPart = totalSize_ - readrIndex_;
        res.append(peek(), fisrtPart);
        res.append(continueRead(), len - fisrtPart);
    }
    retrieve(len);

    if(getReadableBytes() == 0) 
        hasData_ = false;

    return res;
}

std::string RingBuffer::retrieveAllAsString() {
    return retrieveAsString(getReadableBytes());
}

void RingBuffer::append(const std::string& str) {
    append(str.data(), str.size());
}

void RingBuffer::append(const char* data, size_t len) {
    ensureWritableBytes(len);

    if(writerIndex_ > readrIndex_ && writerIndex_ + len > totalSize_) {
        size_t fisrtPart = totalSize_ - writerIndex_;
        std::copy(data, data + fisrtPart, beginWrite());
        std::copy(data + fisrtPart, data + len, continueRead());
    }
    else {
        std::copy(data, data + len, beginWrite());
    }
    hasWritten(len);
}

void RingBuffer::append(const void* data, size_t len) {
    append(static_cast<const char*>(data), len);
}

void RingBuffer::prepend(const void* data, size_t len) {
    assert(len <= getPrependableBytes());
    readrIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, begin()+readrIndex_);
}

void RingBuffer::ensureWritableBytes(size_t len) {
    if(getWritableBytes() < len) {
        makeSpace(len);
    }
    assert(getWritableBytes() >= len);
}

void RingBuffer::shrink(size_t reserve) {
    RingBuffer other;
    size_t readableBytes = getReadableBytes();
    other.ensureWritableBytes(readableBytes + reserve);
    other.append(retrieveAllAsString());
    swap(other);
}

int64_t RingBuffer::peekInt64() const {
    assert(getReadableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    size_t len = totalSize_ - readrIndex_;
    if(len >= sizeof(be64)) {
        ::memcpy(&be64, peek(), sizeof(be64));
    }
    else {
        char* tmp = reinterpret_cast<char*>(&be64);
        ::memcpy(tmp, peek(), len);
        ::memcpy(tmp + len, continueRead(), sizeof(be64) - len);
    }
    return sockets::networkToHost64(be64);
}

int32_t RingBuffer::peekInt32() const {
    assert(getReadableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    size_t len = totalSize_ - readrIndex_;
    if(len >= sizeof(be32)) {
        ::memcpy(&be32, peek(), sizeof(be32));
    }
    else {
        char* tmp = reinterpret_cast<char*>(&be32);
        ::memcpy(tmp, peek(), len);
        ::memcpy(tmp + len, continueRead(), sizeof(be32) - len);
    }
    return sockets::networkToHost32(be32);
}

int16_t RingBuffer::peekInt16() const {
    assert(getReadableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    size_t len = totalSize_ - readrIndex_;
    if(len >= sizeof(be16)) {
        ::memcpy(&be16, peek(), sizeof(be16));
    }
    else {
        char* tmp = reinterpret_cast<char*>(&be16);
        ::memcpy(tmp, peek(), len);
        ::memcpy(tmp + len, continueRead(), sizeof(be16) - len);
    }
    return sockets::networkToHost16(be16);
}

int8_t RingBuffer::peekInt8() const {
    assert(getReadableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
}

int64_t RingBuffer::readInt64() {
    int64_t res = peekInt64();
    retrieveInt64();
    return res;
}

int32_t RingBuffer::readInt32() {
    int32_t res = peekInt32();
    retrieveInt32();
    return res;
}

int16_t RingBuffer::readInt16() {
    int16_t res = peekInt16();
    retrieveInt16();
    return res;
}

int8_t RingBuffer::readInt8() {
    int8_t res = peekInt8();
    retrieveInt8();
    return res;
}

void RingBuffer::appendInt64(int64_t x) {
    int64_t be64 = sockets::hostToNetwork64(x);
    append(&be64, sizeof(be64));
}

void RingBuffer::appendInt32(int32_t x) {
    int32_t be32 = sockets::hostToNetwork32(x);
    append(&be32, sizeof(be32));
}

void RingBuffer::appendInt16(int16_t x) {
    int16_t be16 = sockets::hostToNetwork16(x);
    append(&be16, sizeof(be16));
}

void RingBuffer::appendInt8(int8_t x)  {
    append(&x, sizeof(x));
}

// 结合栈上的空间，避免内存使用过大，提高内存使用率
// 如果有5K个链接，每个连接都分配64K的（发生/接收）缓冲区的话，将独占640M内存
// 而大多数时候，缓冲区的使用率都很低
// https://www.cnblogs.com/solstice/archive/2011/04/17/2018801.html
ssize_t RingBuffer::readFd(int fd, int& savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    size_t writableBytes = 0;
    if(!hasData_ || writerIndex_ < readrIndex_) {
        writableBytes = getWritableBytes();
    }
    else {
        writableBytes = totalSize_ - writerIndex_;
    }

    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writableBytes;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    // when there is enough space in this RingBuffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    // 这样只用调用一次readv
    const int iovcnt = /*(writableBytes < sizeof(extrabuf)) ? 2 : 1*/ 2;
    const ssize_t n = sockets::readv(fd, vec, iovcnt);
    if(n < 0) {
        savedErrno = errno;
    } else if(implicit_cast<size_t>(n) <= writableBytes) {
        writerIndex_ += n;
        hasData_ = true;
    } else {
        writerIndex_ += writableBytes;
        hasData_ = true;
        append(extrabuf, n - writableBytes);  // append里有writerIndex_的移动
    }
    return n;
}

void RingBuffer::makeSpace(size_t len) {
    if(!hasData_) {
        buffer_.resize(kCheapPrepend + len);
        readrIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
        totalSize_ = buffer_.size();
        capacity_ = totalSize_ - kCheapPrepend;
    }
    else if(writerIndex_ > readrIndex_) {
        buffer_.resize(writerIndex_ + len);
        totalSize_ = buffer_.size();
        capacity_ = totalSize_ - kCheapPrepend;
    }
    else {
        RingBuffer other;
        size_t readableBytes = getReadableBytes();
        other.ensureWritableBytes(readableBytes + len);
        other.append(retrieveAllAsString());
        swap(other);
    }
}
