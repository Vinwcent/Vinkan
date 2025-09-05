find_package(glfw3)
add_executable(hello_triangle main.cpp)
target_link_libraries(hello_triangle PRIVATE Vinkan::Vinkan glfw)
set_target_properties(hello_triangle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/hello_triangle
)

add_definitions(-DCOMPILED_SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(hello_triangle PRIVATE ${VINKAN_INCLUDE_DIRS})
