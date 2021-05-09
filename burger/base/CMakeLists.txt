set(base_SRCS
  Timestamp.cc
  Exception.cc
  CountDownLatch.cc
  ThreadPool.cc
  Util.cc 
  Log.cc
  Type.cc
  StringUtil.cc 
  Config.cc
  Env.cc
  Coroutine.cc
  # scheduler.cc
  fcontext/fcontext.S 
  fcontext/jump_x86_64_sysv_elf_gas.S
  fcontext/make_x86_64_sysv_elf_gas.S
)

add_library(burger_base ${base_SRCS})
target_link_libraries(burger_base ${GTEST_BOTH_LIBRARIES} spdlog pthread rt)

install(TARGETS burger_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/burger/base)

# if(BURGER_BUILD_EXAMPLES)
#   add_subdirectory(tests)
# endif()

if(BUILD_TYPE STREQUAL "DEBUG")
    add_subdirectory(tests)
endif()