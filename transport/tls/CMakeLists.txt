find_package(OpenSSL)
add_library(tls_transport STATIC
            ${CMAKE_CURRENT_LIST_DIR}/openssl_posix.c)
target_include_directories(tls_transport PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(tls_transport PUBLIC sockets_wrapper logging_freertos ${OPENSSL_LIBRARIES})
