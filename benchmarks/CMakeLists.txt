# add_executable(ThreadPool_benchmark ThreadPool/ThreadPool_benchmark.cc)
# target_link_libraries(ThreadPool_benchmark burger_base boost_system boost_thread)

# add_executable(logger_benchmark Logger/logger_benchmark.cc)
# target_link_libraries(logger_benchmark burger_base benchmark boost_log boost_thread boost_system)

# add_executable(spd_test spd_test.cc)
# target_link_libraries(spd_test burger_base benchmark boost_log boost_thread boost_system)

add_executable(pingpong_client Throughput/client.cc)
target_link_libraries(pingpong_client burger_net)

add_executable(pingpong_server Throughput/server.cc)
target_link_libraries(pingpong_server burger_net)

add_executable(pingpong_coServer Throughput/coServer.cc)
target_link_libraries(pingpong_coServer burger_net)

add_executable(RingBuffer_benchmark RingBuffer/RingBuffer_benchmark.cc)
target_link_libraries(RingBuffer_benchmark burger_net benchmark)