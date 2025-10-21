add_library(core_mqtt_config INTERFACE)

target_include_directories(core_mqtt_config INTERFACE ${CMAKE_CURRENT_LIST_DIR})

set(CORE_MQTT_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PARENT_SCOPE)