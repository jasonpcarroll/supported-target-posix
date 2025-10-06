set(LOGGING_FREERTOS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/logging_freertos.c)
set(LOGGING_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})

add_library(logging_freertos OBJECT EXCLUDE_FROM_ALL
                                    ${LOGGING_FREERTOS_SOURCES})

target_include_directories(logging_freertos PUBLIC ${LOGGING_INCLUDE_DIRS})
