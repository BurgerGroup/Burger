set_source_files_properties(./usageTest/test.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion")
add_executable(protoUseTest ./usageTest/test_proto.cc ./usageTest/test.pb.cc)
target_link_libraries(protoUseTest protobuf)