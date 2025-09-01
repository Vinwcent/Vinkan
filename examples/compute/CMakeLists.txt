add_executable(compute_example main.cpp)
target_link_libraries(compute_example PRIVATE Vinkan::Vinkan)
set_target_properties(compute_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/compute
)
target_include_directories(compute_example PRIVATE ${VINKAN_INCLUDE_DIRS})
