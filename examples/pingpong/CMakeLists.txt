add_executable(benchMark_pingpong_client client.cc)
target_link_libraries(benchMark_pingpong_client burger_net)

add_executable(benchMark_pingpong_server server.cc)
target_link_libraries(benchMark_pingpong_server burger_net)

# add_executable(pingpong_bench bench.cc)
# target_link_libraries(pingpong_bench burger_net)
