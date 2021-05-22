#ifndef CODEC_H
#define CODEC_H

#include "burger/net/IBuffer.h"
#include "burger/net/Endian.h"
#include "burger/net/CoTcpConnection.h"
#include <boost/noncopyable.hpp>
#include <functional>
#include <string>

using namespace burger;
using namespace burger::net;

class LengthHeaderCodec : boost::noncopyable {
public: 
    using StringMsgCallBack = std::function<void (const CoTcpConnection::ptr& conn, 
                const std::string& msg, Timestamp ts)>;
    explicit LengthHeaderCodec(const StringMsgCallBack& cb) 
        : msgCallback_(cb) 
    {}
    void decode(const CoTcpConnection::ptr& conn, IBuffer::ptr& buf) {
        while(buf->getReadableBytes() >= kHeaderLen) {   // kHeaderLen == 4
            // FIXME: use Buffer::peekInt32()
            const char* data = buf.peek();
            int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
            const int32_t len = sockets::networkToHost32(be32);
            if (len > 65536 || len < 0) {  // 大于4k太长了
                ERROR("Invalid length {}", len);
                conn->shutdown();  
                break;
            } else if (buf.getReadableBytes() >= len + kHeaderLen) {
                buf.retrieve(kHeaderLen);
                std::string message(buf->peek(), len);
                msgCallback_(conn, message);
                buf.retrieve(len);
            } else {
                break;
            }
        }
    }

private:
    StringMsgCallBack msgCallback_;
    const static size_t kHeaderLen = sizeof(int32_t);
};


#endif // CODEC_H