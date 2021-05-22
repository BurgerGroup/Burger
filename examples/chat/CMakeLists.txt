add_executable(chat_server server.cc)
target_link_libraries(chat_server burger_net)

add_executable(chat_client client.cc)
target_link_libraries(chat_client burger_net)