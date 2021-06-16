#include "Buffer.h"
#include "Endian.h"
#include <sys/uio.h>
#include "SocketsOps.h"
#include "burger/base/Util.h"

using namespace burger;
using namespace burger::net;

// const char Buffer::kCRLF[] = "\r\n";
// const size_t Buffer::kCheapPrepend;
// const size_t Buffer::kInitialSize;

Buffer::Buffer(size_t initalSize) :
    // https://blog.csdn.net/uestcyms/article/details/103509443
    // 子类的初始化列表，只能对子类的成员变量进行初始化
    IBuffer(initalSize) {
    assert(getReadableBytes() == 0);
    assert(getWritableBytes() == initalSize);
    assert(getPrependableBytes() == kCheapPrepend);
}

// 性能可能有损耗
void Buffer::swap(IBuffer& rhs) {
    // 需要先将IBuffer&转换为Buffer&
    Buffer& that = dynamic_cast<Buffer&>(rhs);
    buffer_.swap(that.buffer_);
    std::swap(readrIndex_, that.readrIndex_);
    std::swap(writerIndex_, that.writerIndex_);
}

void Buffer::swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readrIndex_, rhs.readrIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
}

void Buffer::hasWritten(size_t len) {
    assert(len <= getWritableBytes());
    writerIndex_ += len;
}

const char* Buffer::findCRLF() const {
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? nullptr : crlf;
}

const char* Buffer::findCRLF(const char* start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? nullptr : crlf;
}

const char* Buffer::findEOL() const {
    const void* eol = memchr(peek(), '\n', getReadableBytes());
    return static_cast<const char*>(eol);
}

const char* Buffer::findEOL(const char* start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void* eol = memchr(start, '\n', beginWrite() - start);
    return static_cast<const char*>(eol);
}

void Buffer::retrieve(size_t len) {
    assert(len <= getReadableBytes());
    if(len < getReadableBytes()) {
        readrIndex_ += len;
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveAll() {
    readrIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

void Buffer::retrieveUntil(const char* end) {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}

std::string Buffer::retrieveAsString(size_t len) {
    assert(len <= getReadableBytes());
    std::string res(peek(), len);
    retrieve(len);
    return res;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(getReadableBytes());
}

void Buffer::append(const std::string& str) {
    append(str.data(), str.size());
}

void Buffer::append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data+len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const void* data, size_t len) {
    append(static_cast<const char*>(data), len);
}

void Buffer::prepend(const void* data, size_t len) {
    assert(len <= getPrependableBytes());
    readrIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, begin()+readrIndex_);
}

void Buffer::ensureWritableBytes(size_t len) {
    if(getWritableBytes() < len) {
        makeSpace(len);
    }
    assert(getWritableBytes() >= len);
}

void Buffer::shrink(size_t reserve) {
    Buffer other;
    size_t readableBytes = getReadableBytes();
    other.ensureWritableBytes(readableBytes + reserve);
    other.append(peek(), readableBytes);
    swap(other);
}

int64_t Buffer::peekInt64() const {
    assert(getReadableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof(be64));
    return sockets::networkToHost64(be64);
}

int32_t Buffer::peekInt32() const {
    assert(getReadableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof(be32));
    return sockets::networkToHost32(be32);
}

int16_t Buffer::peekInt16() const {
    assert(getReadableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof(be16));
    return sockets::networkToHost16(be16);
}

int8_t Buffer::peekInt8() const {
    assert(getReadableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
}

int64_t Buffer::readInt64() {
    int64_t res = peekInt64();
    retrieveInt64();
    return res;
}

int32_t Buffer::readInt32() {
    int32_t res = peekInt32();
    retrieveInt32();
    return res;
}

int16_t Buffer::readInt16() {
    int16_t res = peekInt16();
    retrieveInt16();
    return res;
}

int8_t Buffer::readInt8() {
    int8_t res = peekInt8();
    retrieveInt8();
    return res;
}

void Buffer::appendInt64(int64_t x) {
    int64_t be64 = sockets::hostToNetwork64(x);
    append(&be64, sizeof(be64));
}

void Buffer::appendInt32(int32_t x) {
    int32_t be32 = sockets::hostToNetwork32(x);
    append(&be32, sizeof(be32));
}

void Buffer::appendInt16(int16_t x) {
    int16_t be16 = sockets::hostToNetwork16(x);
    append(&be16, sizeof(be16));
}

void Buffer::appendInt8(int8_t x)  {
    append(&x, sizeof(x));
}

// 结合栈上的空间，避免内存使用过大，提高内存使用率
// 如果有5K个链接，每个连接都分配64K的（发生/接收）缓冲区的话，将独占640M内存
// 而大多数时候，缓冲区的使用率都很低
// https://www.cnblogs.com/solstice/archive/2011/04/17/2018801.html
ssize_t Buffer::readFd(int fd, int& savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writableBytes = getWritableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writableBytes;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    // 这样只用调用一次readv
    const int iovcnt = (writableBytes < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = sockets::readv(fd, vec, iovcnt);
    if(n < 0) {
        savedErrno = errno;
    } else if(implicit_cast<size_t>(n) <= writableBytes) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writableBytes);  // append里有writerIndex_的移动
    }
    return n;
}

void Buffer::makeSpace(size_t len) {
    if(getWritableBytes() + getPrependableBytes() < len + kCheapPrepend) {
        buffer_.resize(writerIndex_+len);
    } else {
        assert(kCheapPrepend < readrIndex_);
        size_t readableBytes = getReadableBytes();
        std::copy(begin() + readrIndex_,
                begin() + writerIndex_,
                begin() + kCheapPrepend);
        readrIndex_ = kCheapPrepend;
        writerIndex_ = readrIndex_ + readableBytes;
        assert(readableBytes == getReadableBytes());
    }
}
















