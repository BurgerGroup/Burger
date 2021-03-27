set(db_SRCS
  Mysql.cc
  MysqlTime.cc
  # MysqlUtil.cc
  # MysqlManager.cc
  MysqlRes.cc
  MysqlStmt.cc
  MysqlStmtRes.cc
  # MysqlTransaction.cc
  )

add_library(burger_db ${db_SRCS})
target_link_libraries(burger_db burger_base mysqlclient pthread)

install(TARGETS burger_db DESTINATION lib)

# 暴露出去的头文件
file(GLOB HEADERS "*.h")

install(FILES ${HEADERS} DESTINATION include/burger/db)

# if(NOT CMAKE_BUILD_NO_EXAMPLES)
#  add_subdirectory(tests)
# endif()

if(BUILD_TYPE STREQUAL "DEBUG")
    add_subdirectory(tests)
endif()