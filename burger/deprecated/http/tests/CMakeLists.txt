add_executable(test_httpMethod test_httpMethod.cc)
target_link_libraries(test_httpMethod burger_http)

add_executable(test_httpStatus test_httpStatus.cc)
target_link_libraries(test_httpStatus burger_http)

add_executable(test_version test_version.cc)
target_link_libraries(test_version burger_http)

add_executable(test_request test_request.cc)
target_link_libraries(test_request burger_http)

add_executable(test_response test_response.cc)
target_link_libraries(test_response burger_http)

add_executable(test_req_parser test_req_parser.cc)
target_link_libraries(test_req_parser burger_http)

add_executable(test_response_parser test_response_parser.cc)
target_link_libraries(test_response_parser burger_http)

add_executable(example example.cc)
target_link_libraries(example burger_http)
