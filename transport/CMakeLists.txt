find_package(OpenSSL REQUIRED)
add_library(transport STATIC ${CMAKE_CURRENT_LIST_DIR}/transport.c
                             ${CMAKE_CURRENT_LIST_DIR}/sockets_posix.c)
target_include_directories(transport PUBLIC ${CMAKE_CURRENT_LIST_DIR}
                                            ${OPENSSL_INCLUDE_DIR})
target_link_libraries(transport PUBLIC logging_freertos ${OPENSSL_LIBRARIES})
