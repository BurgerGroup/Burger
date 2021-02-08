set(net_SRCS
  EventLoop.cc
  Channel.cc 
  Epoll.cc 
  SocketsOps.cc
  Timer.cc
  TimerId.cc
  TimerQueue.cc
  EventLoopThread.cc
  InetAddress.cc
  Socket.cc
  Acceptor.cc
  TcpServer.cc
  TcpConnection.cc
  EventLoopThreadPool.cc
  Buffer.cc
  )

add_library(burger_net ${net_SRCS})
target_link_libraries(burger_net burger_base)

install(TARGETS burger_net DESTINATION lib)

# 暴露出去的头文件
set(HEADERS
  Callbaks.h
  Channel.h
  EventLoop.h
  Timer.h 
  TimerId.h
  TimerQueue.h
  )
#file(GLOB HEADERS "*.h")

install(FILES ${HEADERS} DESTINATION include/burger/net)

#add_subdirectory(http)
#add_subdirectory(inspect)
#
if(NOT CMAKE_BUILD_NO_EXAMPLES)
 add_subdirectory(tests)
endif()
#
#if(PROTOBUF_FOUND)
#  add_subdirectory(protorpc)
#else()
#  add_subdirectory(protorpc EXCLUDE_FROM_ALL)
#endif()