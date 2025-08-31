set(VINKAN_SOURCES)
set(VINKAN_HEADERS)

list(APPEND VINKAN_SOURCES
    src/vinkan/wrappers/instance.cpp
    src/vinkan/wrappers/physical_device.cpp
    src/vinkan/wrappers/swapchain.cpp
		src/vinkan/wrappers/buffer.cpp
)
list(APPEND VINKAN_HEADERS
    src/vinkan/wrappers/instance.hpp
    src/vinkan/wrappers/device_builder.hpp
    src/vinkan/wrappers/device.hpp
    src/vinkan/wrappers/swapchain.hpp
    src/vinkan/wrappers/render_pass_builder.hpp
    src/vinkan/wrappers/render_pass.hpp
		src/vinkan/wrappers/buffer.hpp
		src/vinkan/vinkan_resources.hpp
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

