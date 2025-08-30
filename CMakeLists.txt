cmake_minimum_required(VERSION 3.14)
project(Vinkan)

find_package(Vulkan REQUIRED)
find_package(spdlog REQUIRED)
include(sources.cmake)

set(VINKAN_LIBRARY_NAME Vinkan)
add_library(${VINKAN_LIBRARY_NAME} ${VINKAN_FILES})

add_library(Vinkan::Vinkan ALIAS ${VINKAN_LIBRARY_NAME})

target_link_libraries(${VINKAN_LIBRARY_NAME} PUBLIC 
    ${Vulkan_LIBRARIES} 
    spdlog::spdlog
)

target_include_directories(${VINKAN_LIBRARY_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
)

set(VINKAN_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${Vulkan_INCLUDE_DIRS} 
    PARENT_SCOPE
)
