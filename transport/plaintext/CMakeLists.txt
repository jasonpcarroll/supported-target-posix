add_library(plaintext_transport STATIC
            ${CMAKE_CURRENT_LIST_DIR}/plaintext_posix.c)
target_include_directories(plaintext_transport PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(plaintext_transport PUBLIC sockets_wrapper logging_freertos)
