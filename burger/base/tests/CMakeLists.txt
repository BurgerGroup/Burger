
# add_executable(timestamp_unittest Timestamp_unittest.cc)
# target_link_libraries(timestamp_unittest burger_base)

# add_executable(atomic_unittest Atomic_unittest.cc)

# add_executable(exception_unittest Exception_unittest.cc)
# target_link_libraries(exception_unittest burger_base)

# add_executable(mutex_test mutex_test.cc)
# target_link_libraries(mutex_test burger_base)

# add_executable(ThreadPool_test ThreadPool_test.cc)
# target_link_libraries(ThreadPool_test burger_base)

# add_executable(Singleton_test Singleton_test.cc)

# add_executable(SingletonThreadLocal_test SingletonThreadLocal_test.cc)
# target_link_libraries(SingletonThreadLocal_test burger_base)

# add_executable(spdlog_test spdlog_test.cc)

add_executable(log_test log_test.cc)
target_link_libraries(log_test burger_base)

add_executable(util_test util_test.cc)
target_link_libraries(util_test burger_base)

add_executable(yaml_test yaml_test.cc)
target_link_libraries(yaml_test burger_base)
set_source_files_properties(
  yaml_test
  PROPERTIES
  COMPILE_FLAGS "-w"
)

add_executable(env_test env_test.cc)
target_link_libraries(env_test burger_base)