# add_executable(thread_test thread_test.cc)

# add_executable(thread_emplace_back thread_emplace_back.cc)

# add_executable(thread_local thread_local.cc)

add_executable(gtest_test gtest_test.cc)
target_link_libraries(gtest_test gtest)
add_test(NAME gtest_test COMMAND gtest_test)

# add_executable(lower_bound lower_bound.cc)

# add_executable(json_test json_test.cc)

add_executable(shareFromThis shareFromThis.cc)

add_executable(chrono_test chrono_test.cc)

add_executable(ini_test ini_test.cc)

