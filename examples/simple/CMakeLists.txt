add_executable(simple_chargen chargen/chargen.cc chargen/main.cc)
target_link_libraries(simple_chargen burger_net)

add_executable(simple_discard discard/discard.cc discard/main.cc)
target_link_libraries(simple_discard burger_net)

add_executable(simple_daytime daytime/daytime.cc daytime/main.cc)
target_link_libraries(simple_daytime burger_net)

add_executable(simple_echo echo/echo.cc echo/main.cc)
target_link_libraries(simple_echo burger_net)

# add_executable(simple_send_client send_client/client.cc send_client/main.cc)
# target_link_libraries(simple_send_client burger_net)

add_executable(simple_chargenclient chargenclient/chargenclient.cc)
target_link_libraries(simple_chargenclient burger_net)



