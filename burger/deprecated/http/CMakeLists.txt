set(parser_SRCS
  http11_parser.cc
  httpclient_parser.cc)

# https://www.coder.work/article/6663009

set_source_files_properties(
  ${parser_SRCS}
  PROPERTIES
  COMPILE_FLAGS "-w"
)

set(http_SRCS
  HttpRequest.cc
  HttpMethod.cc
  HttpStatus.cc
  Version.cc
  HttpResponse.cc
  http11_parser.cc
  httpclient_parser.cc
  HttpRequestParser.cc
  HttpResponseParser.cc
  )

add_library(burger_http ${http_SRCS})
target_link_libraries(burger_http burger_net)

install(TARGETS burger_http DESTINATION lib)

# 暴露出去的头文件
file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/burger/http)

# if(NOT CMAKE_BUILD_NO_EXAMPLES)
#  add_subdirectory(tests)
# endif()

if(BUILD_TYPE STREQUAL "DEBUG")
    add_subdirectory(tests)
endif()