#include "gtest/gtest.h"
#include <iostream>
#include "burger/net/Buffer.h"

using namespace burger;
using namespace burger::net;

TEST(testBufferAppendRetrieve, readWritePrepend) {
    Buffer buf;
    EXPECT_EQ(buf.getReadableBytes(), 0);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend);

}