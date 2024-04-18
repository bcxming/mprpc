set(SRC_LIST friendservice.cc ../friend.pb.cc)

add_executable(provider ${SRC_LIST})
target_link_libraries(provider mprpc)

# 我新加的
find_package(Protobuf REQUIRED)
# 如果找到 Protobuf 库，将其链接到您的目标
if(PROTOBUF_FOUND)
    target_link_libraries(provider ${PROTOBUF_LIBRARIES})
else()
    message(FATAL_ERROR "Protobuf library not found")
endif()
