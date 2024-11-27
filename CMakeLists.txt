cmake_minimum_required(VERSION 3.25)

project(ipcheck)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

include(FetchContent)

find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(json)
endif()

FetchContent_Declare(
    curlpp
    GIT_REPOSITORY https://github.com/jpbarrette/curlpp
    GIT_TAG master
)
FetchContent_MakeAvailable(curlpp)

add_subdirectory(mailclient-cpp)

add_executable(ipcheck
        main.cpp
        src/IPServer.cpp
)

target_link_libraries(
        ipcheck PUBLIC
        curl
        curlpp
        nlohmann_json::nlohmann_json
        mailclient
        pthread
)

target_include_directories(
        ipcheck PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/mailclient-cpp/MAIL"
        "${CMAKE_CURRENT_SOURCE_DIR}/curlpp/include/curlpp"
)
