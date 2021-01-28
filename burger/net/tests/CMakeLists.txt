
add_executable(Reactor_test01 Reactor_test01.cc)
target_link_libraries(Reactor_test01 burger_net)

add_executable(Reactor_test02 Reactor_test02.cc)
target_link_libraries(Reactor_test02 burger_net)

add_executable(timerfd_test timerfd_test.cc)
target_link_libraries(timerfd_test burger_net)

add_executable(timer_test timer_test.cc)
target_link_libraries(timer_test burger_net)