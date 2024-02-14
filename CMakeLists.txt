cmake_minimum_required(VERSION 3.27)

if (DEFINED ${CMAKE_CROSS_COMPILE})
    set(CMAKE_C_COMPILER "/opt/x-tools-armv6-rpi-linux-gnueabihf/x-tools/armv6-rpi-linux-gnueabihf/bin/armv6-rpi-linux-gnueabihf-gcc")
    set(CMAKE_CXX_COMPILER "/opt/x-tools-armv6-rpi-linux-gnueabihf/x-tools/armv6-rpi-linux-gnueabihf/bin/armv6-rpi-linux-gnueabihf-g++")
    set(CMAKE_TOOLCHAIN_FILE "/opt/x-tools-armv6-rpi-linux-gnueabihf/x-tools/armv6-rpi-linux-gnueabihf/armv6-rpi-linux-gnueabihf.toolchain.cmake")
endif(DEFINED ${CMAKE_CROSS_COMPILE})

project(ipcheck)


set(CMAKE_CXX_STANDARD 23)


add_subdirectory(mailclient-cpp)
add_subdirectory(curlpp)

add_executable(ipcheck main.cpp)

target_link_libraries(
        ipcheck PUBLIC
        curl
        curlpp_static
        mailclient
)

target_include_directories(
        ipcheck PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/mailclient-cpp/MAIL"
        "${CMAKE_CURRENT_SOURCE_DIR}/curlpp/include/curlpp"
)
