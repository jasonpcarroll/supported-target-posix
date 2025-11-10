add_library(clock STATIC ${CMAKE_CURRENT_LIST_DIR}/clock.c)
target_include_directories(clock PUBLIC ${CMAKE_CURRENT_LIST_DIR})
