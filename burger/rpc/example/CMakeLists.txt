add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/user.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/user.pb.h
    COMMAND protoc 
    ARGS --cpp_out ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/proto/user.proto -I${CMAKE_CURRENT_SOURCE_DIR}/proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/proto/user.proto 
    COMMENT "Generating user.pb.cc and user.pb.h"
    VERBATIM
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/friend.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/friend.pb.h
    COMMAND protoc 
    ARGS --cpp_out ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/proto/friend.proto -I${CMAKE_CURRENT_SOURCE_DIR}/proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/proto/friend.proto 
    COMMENT "Generating friend.pb.cc and friend.pb.h"
    VERBATIM
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(user.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion")
set_source_files_properties(friend.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion")

add_executable(provider ./callee/userService.cc user.pb.cc)
target_link_libraries(provider burger_rpc)

add_executable(consumer ./caller/callUserService.cc user.pb.cc)
target_link_libraries(consumer burger_rpc)

# friend
add_executable(friendProvider ./callee/friendService.cc friend.pb.cc)
target_link_libraries(friendProvider burger_rpc)

add_executable(friendConsumer ./caller/callFriendService.cc friend.pb.cc)
target_link_libraries(friendConsumer burger_rpc)
