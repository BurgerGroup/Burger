set(base_SRCS
  Timestamp.cc
  Exception.cc
  CountDownLatch.cc
  ThreadPool.cc
  Log.cc
  Util.cc
)


add_library(burger_base ${base_SRCS})
target_link_libraries(burger_base ${GTEST_BOTH_LIBRARIES} pthread rt)
# set_target_properties(burger_base PROPERTIES COMPILE_FLAGS "-std=c++11")
# include_directories(
#     ${CMAKE_SOURCE_DIR}/burger
# )

install(TARGETS burger_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/burger/base)

if(BURGER_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()
