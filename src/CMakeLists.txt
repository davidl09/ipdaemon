set(IPCHECK_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})

add_library(
        ipcheck-lib STATIC 
        IPServer.cpp
)

target_include_directories(ipcheck-lib PUBLIC ${IPCHECK_INCLUDES})

target_link_libraries(
        ipcheck-lib PUBLIC 
        mailclient
        curlpp 
        curl
        nlohmann_json::nlohmann_json
)
