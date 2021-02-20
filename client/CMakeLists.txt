set(client_SRCS
    interface.cc
)

add_library(client ${client_SRCS})
target_link_libraries(client ${CURSES_LIBRARIES} pthread rt)

install(TARGETS client DESTINATION lib)

# file(GLOB HEADERS "*.h")
# install(FILES ${HEADERS} DESTINATION include/burger/base)

add_subdirectory(tests)
