add_library(tls_transport STATIC
            ${CMAKE_CURRENT_LIST_DIR}/plaintext_posix.c)
target_include_directories(tls_transport PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(tls_transport PUBLIC sockets_wrapper logging_freertos)
