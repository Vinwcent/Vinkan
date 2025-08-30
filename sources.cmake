set(VINKAN_SOURCES)
set(VINKAN_HEADERS)

list(APPEND VINKAN_SOURCES
    src/vinkan/instance.cpp
    src/vinkan/physical_device.cpp
)
list(APPEND VINKAN_HEADERS
    src/vinkan/instance.hpp
    src/vinkan/device_builder.hpp
    src/vinkan/device.hpp
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

