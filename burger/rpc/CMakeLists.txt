set(rpc_SRCS
    RpcServer.cc
    rpcHeader.pb.cc
    RpcChannel.cc
    RpcController.cc
  )

add_library(burger_rpc ${rpc_SRCS})
target_link_libraries(burger_rpc burger_base burger_net protobuf)

install(TARGETS burger_rpc DESTINATION lib)

# 暴露出去的头文件
file(GLOB HEADERS "*.h")

install(FILES ${HEADERS} DESTINATION include/burger/rpc)

if(BUILD_TYPE STREQUAL "DEBUG")
    add_subdirectory(tests)
    add_subdirectory(example)
endif()