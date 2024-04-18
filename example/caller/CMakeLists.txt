set(SRC_LIST callfriendservice.cc ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})
target_link_libraries(consumer mprpc)

# 我新加的
find_package(Protobuf REQUIRED)
# 如果找到 Protobuf 库，将其链接到您的目标
if(PROTOBUF_FOUND)
    target_link_libraries(consumer ${PROTOBUF_LIBRARIES})
else()
    message(FATAL_ERROR "Protobuf library not found")
endif()