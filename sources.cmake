set(VINKAN_SOURCES)
set(VINKAN_HEADERS)

list(APPEND VINKAN_SOURCES
    src/vinkan/wrappers/instance.cpp
    src/vinkan/wrappers/physical_device.cpp
)
list(APPEND VINKAN_HEADERS
    src/vinkan/wrappers/instance.hpp
    src/vinkan/wrappers/device_builder.hpp
    src/vinkan/wrappers/device.hpp
)

if(VINKAN_WITH_GLFW)
    list(APPEND VINKAN_SOURCES
        src/vinkan/glfw/glfw_vk_surface.cpp
    )
    list(APPEND VINKAN_HEADERS
        src/vinkan/glfw/glfw_vk_surface.hpp
    )
endif()

set(VINKAN_FILES ${VINKAN_SOURCES} ${VINKAN_HEADERS})

