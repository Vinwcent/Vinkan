cmake_minimum_required(VERSION 3.14)
project(Vinkan)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

if(DEVELOPMENT_BUILD)
    find_package(spdlog REQUIRED)
endif()

# magic_enum
include(FetchContent)
FetchContent_Declare(
	magic_enum
	GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
	GIT_TAG        v0.9.7
)
FetchContent_MakeAvailable(magic_enum)

option(VINKAN_WITH_GLFW "Build with GLFW support" OFF)
if(VINKAN_WITH_GLFW)
    find_package(glfw3 REQUIRED)
endif()

include(sources.cmake)

set(VINKAN_LIBRARY_NAME Vinkan)
add_library(${VINKAN_LIBRARY_NAME} ${VINKAN_FILES})
add_library(Vinkan::Vinkan ALIAS ${VINKAN_LIBRARY_NAME})

target_link_libraries(${VINKAN_LIBRARY_NAME} PUBLIC 
    ${Vulkan_LIBRARIES} 
		magic_enum::magic_enum
)
if(DEVELOPMENT_BUILD)
    target_link_libraries(${VINKAN_LIBRARY_NAME} PUBLIC spdlog::spdlog)
endif()
if(VINKAN_WITH_GLFW)
    target_link_libraries(${VINKAN_LIBRARY_NAME} PUBLIC glfw)
    target_compile_definitions(${VINKAN_LIBRARY_NAME} PUBLIC VINKAN_HAS_GLFW)
endif()

target_include_directories(${VINKAN_LIBRARY_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
)

set(VINKAN_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${Vulkan_INCLUDE_DIRS} 
)
set(VINKAN_INCLUDE_DIRS ${VINKAN_INCLUDE_DIRS} PARENT_SCOPE)

# Logs
set(VINKAN_LOG_LEVEL "OFF" CACHE STRING "Log level (OFF/TRACE/DEBUG/INFO/WARN/ERROR/CRITICAL)")
set_property(CACHE VINKAN_LOG_LEVEL PROPERTY STRINGS "OFF" "TRACE" "DEBUG" "INFO" "WARN" "ERROR" "CRITICAL")

set(SPDLOG_LEVEL_MAP_OFF "SPDLOG_LEVEL_OFF")
set(SPDLOG_LEVEL_MAP_TRACE "SPDLOG_LEVEL_TRACE")
set(SPDLOG_LEVEL_MAP_DEBUG "SPDLOG_LEVEL_DEBUG") 
set(SPDLOG_LEVEL_MAP_INFO "SPDLOG_LEVEL_INFO")
set(SPDLOG_LEVEL_MAP_WARN "SPDLOG_LEVEL_WARN")
set(SPDLOG_LEVEL_MAP_ERROR "SPDLOG_LEVEL_ERROR")
set(SPDLOG_LEVEL_MAP_CRITICAL "SPDLOG_LEVEL_CRITICAL")

if(DEVELOPMENT_BUILD)
    target_compile_definitions(${VINKAN_LIBRARY_NAME} PUBLIC 
	SPDLOG_ACTIVE_LEVEL=${SPDLOG_LEVEL_MAP_${VINKAN_LOG_LEVEL}}
    )
endif()

# Tests
option(VINKAN_BUILD_EXAMPLES "Build examples" OFF)
if(VINKAN_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
