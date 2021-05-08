
# add_executable(EventLoop_test01 EventLoop_test01.cc)
# target_link_libraries(EventLoop_test01 burger_net)

# add_executable(EventLoop_test02 EventLoop_test02.cc)
# target_link_libraries(EventLoop_test02 burger_net)

# add_executable(EventLoop_test03 EventLoop_test03.cc)
# target_link_libraries(EventLoop_test03 burger_net)

# add_executable(timerfd_test timerfd_test.cc)
# target_link_libraries(timerfd_test burger_net)

# add_executable(timer_test timer_test.cc)
# target_link_libraries(timer_test burger_net)

# add_executable(runInLoop_test runInLoop_test.cc)
# target_link_libraries(runInLoop_test burger_net)

# add_executable(EventLoopThread_test EventLoopThread_test.cc)
# target_link_libraries(EventLoopThread_test burger_net)

# add_executable(InetAddress_unittest InetAddress_unittest.cc)
# target_link_libraries(InetAddress_unittest burger_net)
# add_test(NAME InetAddress_unittest COMMAND InetAddress_unittest)

# add_executable(Socket_unittest Socket_unittest.cc)
# target_link_libraries(Socket_unittest burger_net)

# add_executable(Acceptor_test Acceptor_test.cc)
# target_link_libraries(Acceptor_test burger_net)

# add_executable(tcpConnection_test01 tcpConnection_test01.cc)
# target_link_libraries(tcpConnection_test01 burger_net)

# add_executable(tcpConnection_test02 tcpConnection_test02.cc)
# target_link_libraries(tcpConnection_test02 burger_net)

# add_executable(tcpConnection_test03 tcpConnection_test03.cc)
# target_link_libraries(tcpConnection_test03 burger_net)

# add_executable(tcpConnection_test03_client tcpConnection_test03_client.cc)
# target_link_libraries(tcpConnection_test03_client burger_net)

# add_executable(tcpConnection_test04 tcpConnection_test04.cc)
# target_link_libraries(tcpConnection_test04 burger_net)

# add_executable(tcpConnection_test05 tcpConnection_test05.cc)
# target_link_libraries(tcpConnection_test05 burger_net)

# add_executable(EventLoopThreadPool_test EventLoopThreadPool_test.cc)
# target_link_libraries(EventLoopThreadPool_test burger_net)

# add_executable(Buffer_unittest Buffer_unittest.cc)
# target_link_libraries(Buffer_unittest burger_net gtest)
# add_test(NAME Buffer_unittest COMMAND Buffer_unittest)

# add_executable(RingBuffer_unittest RingBuffer_unittest.cc)
# target_link_libraries(RingBuffer_unittest burger_net gtest)
# add_test(NAME RingBuffer_unittest COMMAND RingBuffer_unittest)

# add_executable(TcpClient_test TcpClient_test.cc)
# target_link_libraries(TcpClient_test burger_net)

# add_executable(Hook_test Hook_test.cc)
# target_link_libraries(Hook_test burger_net)

# add_executable(sched_timer_test01 sched_timer_test01.cc)
# target_link_libraries(sched_timer_test01 burger_net)

# add_executable(sched_timer_test02 sched_timer_test02.cc)
# target_link_libraries(sched_timer_test02 burger_net)

# add_executable(sched_timer_test03 sched_timer_test03.cc)
# target_link_libraries(sched_timer_test03 burger_net)


add_executable(coTcpServer_test coTcpServer_test.cc)
target_link_libraries(coTcpServer_test burger_net)

# add_executable(sched_stop_test sched_stop_test.cc)
# target_link_libraries(sched_stop_test burger_net)

# add_executable(idleCoQue_test idleCoQue_test.cc)
# target_link_libraries(idleCoQue_test burger_net)

add_executable(shutdown_test shutdown_test.cc)
target_link_libraries(shutdown_test burger_net)



