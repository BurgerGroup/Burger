#include "Buffer.h"


using namespace burger;
using namespace burger::net;

const char Buffer::kCRLF[] = "\r\n";
const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

Buffer::Buffer(size_t initalSize) :
    buffer_(kCheapPrepend + initalSize),
    readrIndex_(kCheapPrepend),
    writerIndex_(kCheapPrepend) {
    assert(getReadableBytes() == 0);
    assert(getWritableBytes() == initalSize);
    assert(getPrependableBytes() == kCheapPrepend);
}

void Buffer::swap(Buffer& rhs) {
    // 只用交换三个数据成员
    buffer_.swap(rhs.buffer_);
    std::swap(readrIndex_, rhs.readrIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
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












