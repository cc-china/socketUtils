cmake_minimum_required(VERSION 3.10.2)
project("socket")
# 指定so生成到libs目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/socket/)
add_library(socket_jni SHARED socket/jni_socket.cpp)
add_library(hyn_open_system_device_jni SHARED JniOpenSystemDevice.cpp)
#2.添加第三方库
add_library(libsocket_service SHARED IMPORTED)
set_target_properties(libsocket_service
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/socket/libs/arm64-v8a/libsocket_service.so)
find_library(log-lib log)
target_link_libraries(socket_jni libsocket_service ${log-lib})
target_link_libraries(hyn_open_system_device_jni ${log-lib})
#add_library(socket_service SHARED socket/server/socketserver.cpp)
#target_link_libraries(socket_service ${log-lib})