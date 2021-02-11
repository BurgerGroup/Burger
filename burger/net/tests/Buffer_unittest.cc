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
    const std::string str(200, 'x');
    buf.append(str);
    EXPECT_EQ(buf.getReadableBytes(), str.size());
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - str.size());
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend);

    const std::string str2 = buf.retrieveAsString(50);
    EXPECT_EQ(str2.size(), 50);
    EXPECT_EQ(buf.getReadableBytes(), str.size() - str2.size());
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - str.size());
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend + str2.size());

    buf.append(str);
    EXPECT_EQ(buf.getReadableBytes(), 2*str.size() - str2.size());
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 2*str.size());
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend + str2.size());

    const std::string str3 =  buf.retrieveAllAsString();
    EXPECT_EQ(str3.size(), 350);
    EXPECT_EQ(buf.getReadableBytes(), 0);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend);
    EXPECT_EQ(str3, std::string(350, 'x'));
}

TEST(testBufferGrow, append) {
    Buffer buf;
    buf.append(std::string(400, 'y'));
    EXPECT_EQ(buf.getReadableBytes(), 400);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 400);

    buf.retrieve(50);
    EXPECT_EQ(buf.getReadableBytes(), 350);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 400);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend + 50);

    buf.append(std::string(1000, 'z'));
    EXPECT_EQ(buf.getReadableBytes(), 1350);
    EXPECT_EQ(buf.getWritableBytes(), 0);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend + 50); 

    buf.retrieveAll();
    EXPECT_EQ(buf.getReadableBytes(), 0);
    EXPECT_EQ(buf.getWritableBytes(), 1400); // FIXME
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend);
}


TEST(testBufferInsideGrow, retrieve) {
    Buffer buf;
    buf.append(std::string(800, 'y'));
    EXPECT_EQ(buf.getReadableBytes(), 800);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 800);

    buf.retrieve(500);
    EXPECT_EQ(buf.getReadableBytes(), 300);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 800);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend + 500);

    buf.append(std::string(300, 'z'));
    EXPECT_EQ(buf.getReadableBytes(), 600);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 600);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend);
}

TEST(testBufferShrink, retrieve) {
    Buffer buf;
    buf.append(std::string(2000, 'y'));
    EXPECT_EQ(buf.getReadableBytes(), 2000);
    EXPECT_EQ(buf.getWritableBytes(), 0);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend);

    buf.retrieve(1500);
    EXPECT_EQ(buf.getReadableBytes(), 500);
    EXPECT_EQ(buf.getWritableBytes(), 0);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend + 1500);

    buf.shrink(0);
    EXPECT_EQ(buf.getReadableBytes(), 500);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 500);
    EXPECT_EQ(buf.retrieveAllAsString(), std::string(500, 'y'));
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend);
}


TEST(testBufferPrepend, prePend) {
    Buffer buf;
    buf.append(std::string(200, 'y'));
    EXPECT_EQ(buf.getReadableBytes(), 200);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 200);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend);

    int x = 0;
    buf.prepend(&x, sizeof(x));
    EXPECT_EQ(buf.getReadableBytes(), 204);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize - 200);
    EXPECT_EQ(buf.getPrependableBytes(), Buffer::kCheapPrepend - 4);
}


TEST(testBufferReadInt, read) {
    Buffer buf;
    buf.append("HTTP");

    EXPECT_EQ(buf.getReadableBytes(), 4);
    EXPECT_EQ(buf.peekInt8(), 'H');
    int top16 = buf.peekInt16();
    EXPECT_EQ(top16, 'H'*256 + 'T');
    EXPECT_EQ(buf.peekInt32(), top16*256*256 + 'T'*256 + 'P');

    EXPECT_EQ(buf.readInt8(), 'H');
    EXPECT_EQ(buf.readInt16(), 'T'*256 + 'T');
    EXPECT_EQ(buf.readInt8(), 'P');
    EXPECT_EQ(buf.getReadableBytes(), 0);
    EXPECT_EQ(buf.getWritableBytes(), Buffer::kInitialSize);  // 最后刚好全部取完的话，则全部复位

    buf.appendInt8(-1); // 1
    buf.appendInt16(-2); // 2
    buf.appendInt32(-3); // 4
    EXPECT_EQ(buf.getReadableBytes(), 7);
    EXPECT_EQ(buf.readInt8(), -1);
    EXPECT_EQ(buf.readInt16(), -2);
    EXPECT_EQ(buf.readInt32(), -3);
}

TEST(testBufferFindEOL, findEOL) {
    Buffer buf;
    buf.append(std::string(100000, 'x'));
    const char* null = NULL;
    EXPECT_EQ(buf.findEOL(), null);
    EXPECT_EQ(buf.findEOL(buf.peek()+90000), null);
}

void output(Buffer&& buf, const void* inner) {
    Buffer newbuf(std::move(buf));
    // printf("New Buffer at %p, inner %p\n", &newbuf, newbuf.peek());
    EXPECT_EQ(inner, newbuf.peek());
}

// NOTE: This test fails in g++ 4.4, passes in g++ 4.6.
TEST(testMove, move) {
    Buffer buf;
    buf.append("muduo");
    const void* inner = buf.peek();
    // printf("Buffer at %p, inner %p\n", &buf, inner);
    output(std::move(buf), inner);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}