set(base_SRCS
  Timestamp.cc
  Exception.cc
  CountDownLatch.cc
  ThreadPool.cc
  Log.cc
  Util.cc
  Type.cc
  StringUtil.cc 
  #Config.cc
  Env.cc
)


add_library(burger_base ${base_SRCS})
target_link_libraries(burger_base ${GTEST_BOTH_LIBRARIES} pthread yaml-cpp rt)

install(TARGETS burger_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/burger/base)

# if(BURGER_BUILD_EXAMPLES)
#   add_subdirectory(tests)
# endif()

if(BUILD_TYPE STREQUAL "DEBUG")
    add_subdirectory(tests)
endif()