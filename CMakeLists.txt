cmake_minimum_required(VERSION 3.14)
project(Vinkan)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(spdlog REQUIRED)

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
    spdlog::spdlog
)
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
    PARENT_SCOPE
)
