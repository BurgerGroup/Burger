// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>
#include <endian.h>

/**
 * @brief 封装了字节序转换函数
*/
namespace burger {
namespace net {
namespace sockets {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

inline uint64_t hostToNetwork64(uint64_t host64) {
    return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32) {
    return htobe32(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16) {
    return htobe16(host16);
}

inline uint64_t networkToHost64(uint64_t net64) {
    return be64toh(net64);
}

inline uint32_t networkToHost32(uint32_t net32) {
    return be32toh(net32);
}

inline uint16_t networkToHost16(uint16_t net16) {
    return be16toh(net16);
}

#pragma GCC diagnostic pop
} // namespace socket 

} // namespace net

} // namespace burger



#endif // ENDIAN_H