set(http_SRCS
  HttpRequest.cc
  HttpMethod.cc
  )

add_library(burger_http ${http_SRCS})
target_link_libraries(burger_http burger_net)

install(TARGETS burger_http DESTINATION lib)
# set(HEADERS
#   HttpContext.h
#   HttpRequest.h
#   HttpResponse.h
#   HttpServer.h
#   )
install(FILES ${HEADERS} DESTINATION include/burger/http)

# if(BURGER_BUILD_EXAMPLES)
# add_executable(httpserver_test tests/HttpServer_test.cc)
# target_link_libraries(httpserver_test burger_http)

if(BOOSTTEST_LIBRARY)
add_executable(httprequest_unittest tests/HttpRequest_unittest.cc)
target_link_libraries(httprequest_unittest muduo_http boost_unit_test_framework)
endif()

# endif()

add_subdirectory(tests)