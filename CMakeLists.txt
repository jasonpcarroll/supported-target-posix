cmake_minimum_required(VERSION 3.15.3)

project(TargetSupportPosix C)

# ------------------------------------------------------------------------------
# Targets
# ------------------------------------------------------------------------------

# Logging

add_subdirectory(logging)

# FreeRTOS configuration

add_subdirectory(freertos_config)
