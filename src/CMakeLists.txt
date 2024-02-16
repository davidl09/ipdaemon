set(IPCHECK_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

add_library(ipcheck STATIC IPServer.cpp)
target_include_directories(ipcheck PUBLIC IPCHECK_INCLUDES)
