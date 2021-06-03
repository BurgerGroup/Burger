// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

#ifndef EPOLL_H
#define EPOLL_H

#include "burger/base/Timestamp.h"
#include <boost/noncopyable.hpp>
#include <vector>
#include <memory>
#include <map>
#include <sys/epoll.h>

namespace burger {
namespace net {
class EventLoop;
class Channel;
class Epoll : boost::noncopyable {
public:
    Epoll(EventLoop* loop);
    ~Epoll();
    // Must called in Loop thread
    Timestamp wait(int timeoutMs, std::vector<Channel*>& activeChannels);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel) const;

    void assertInLoopThread() const;
private:
    void fillActiveChannels(int numEvents, 
                std::vector<Channel*>& activeChannels) const;
    void epollctl(int operation, Channel* channel);
    static std::string operationToString(int op);
private:
    static const int kInitEventListSize = 16;
    std::map<int, Channel*> channelMap_;  // 有序，添加修改效率高
    EventLoop* ownerLoop_;  
    int epollFd_;   
    std::vector<struct epoll_event> eventList_; 
};

} // namespace net

} // namespace burger

#endif // EPOLL_H