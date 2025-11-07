add_library(sockets_wrapper STATIC ${CMAKE_CURRENT_LIST_DIR}/sockets_posix.c)
target_include_directories(sockets_wrapper PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(sockets_wrapper PUBLIC logging_freertos)
