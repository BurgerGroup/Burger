
add_executable(Reactor_test01 Reactor_test01.cc)
target_link_libraries(Reactor_test01 burger_net)

add_executable(Reactor_test02 Reactor_test02.cc)
target_link_libraries(Reactor_test02 burger_net)

add_executable(timerfd_test timerfd_test.cc)
target_link_libraries(timerfd_test burger_net)

add_executable(timer_test timer_test.cc)
target_link_libraries(timer_test burger_net)

add_executable(EventLoopThread_test EventLoopThread_test.cc)
target_link_libraries(EventLoopThread_test burger_net)

add_executable(InetAddress_unittest InetAddress_unittest.cc)
target_link_libraries(InetAddress_unittest burger_net)
add_test(NAME InetAddress_unittest COMMAND InetAddress_unittest)

# add_executable(Socket_unittest Socket_unittest.cc)
# target_link_libraries(Socket_unittest burger_net)

add_executable( Acceptor_test Acceptor_test.cc)
target_link_libraries(Acceptor_test burger_net)



