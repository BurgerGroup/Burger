
# add_executable(timestamp_unittest Timestamp_unittest.cc)
# target_link_libraries(timestamp_unittest burger_base)

add_executable(atomic_unittest Atomic_unittest.cc)

# add_executable(exception_unittest Exception_unittest.cc)
# target_link_libraries(exception_unittest burger_base)

# add_executable(mutex_test mutex_test.cc)
# target_link_libraries(mutex_test burger_base)

add_executable(ThreadPool_test ThreadPool_test.cc)
target_link_libraries(ThreadPool_test burger_base)

# add_executable(Singleton_test Singleton_test.cc)

# add_executable(SingletonThreadLocal_test SingletonThreadLocal_test.cc)
# target_link_libraries(SingletonThreadLocal_test burger_base)

# add_executable(spdlog_test spdlog_test.cc)

# add_executable(log_test log_test.cc)
# target_link_libraries(log_test burger_base)

# add_executable(util_test util_test.cc)
# target_link_libraries(util_test burger_base)

add_executable(Config_test Config_test.cc)
target_link_libraries(Config_test burger_base)

# add_executable(env_test env_test.cc)
# target_link_libraries(env_test burger_base)

add_executable(Coroutine_test01 Coroutine_test01.cc)
target_link_libraries(Coroutine_test01 burger_base)

add_executable(Coroutine_test02 Coroutine_test02.cc)
target_link_libraries(Coroutine_test02 burger_base)

add_executable(Coroutine_test03 Coroutine_test03.cc)
target_link_libraries(Coroutine_test03 burger_base)

add_executable(StringUtil_test StringUtil_test.cc)
target_link_libraries(StringUtil_test burger_base)
