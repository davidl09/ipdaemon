cmake_minimum_required(VERSION 3.27)

#set(CMAKE_TOOLCHAIN_FILE "/opt/x-tools-armv6-rpi-linux-gnueabihf/x-tools/armv6-rpi-linux-gnueabihf/armv6-rpi-linux-gnueabihf.toolchain.cmake")

project(ipcheck)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(mailclient-cpp)

add_executable(ipcheck
        main.cpp
        src/IPServer.cpp
)

target_link_libraries(
        ipcheck PUBLIC
        curl
        curlpp
        mailclient
)

target_include_directories(
        ipcheck PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/mailclient-cpp/MAIL"
        "${CMAKE_CURRENT_SOURCE_DIR}/curlpp/include/curlpp"
)
