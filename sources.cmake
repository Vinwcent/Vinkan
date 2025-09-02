set(VINKAN_SOURCES)
set(VINKAN_HEADERS)

list(APPEND VINKAN_SOURCES
    src/vinkan/wrappers/instance.cpp
    src/vinkan/wrappers/physical_device.cpp
    src/vinkan/wrappers/swapchain.cpp
		src/vinkan/wrappers/buffer.cpp

		src/vinkan/wrappers/descriptors/descriptor_pool.cpp
		src/vinkan/wrappers/descriptors/descriptor_set_layout.cpp
		src/vinkan/wrappers/descriptors/descriptor_set.cpp
)
list(APPEND VINKAN_HEADERS
    src/vinkan/wrappers/instance.hpp
    src/vinkan/wrappers/device.hpp
    src/vinkan/wrappers/swapchain.hpp
    src/vinkan/wrappers/render_pass.hpp
		src/vinkan/wrappers/buffer.hpp
		src/vinkan/resources/resources.hpp
		src/vinkan/resources/resources_binder.hpp

		src/vinkan/wrappers/descriptors/descriptor_pool.hpp
		src/vinkan/wrappers/descriptors/descriptor_set_layout.hpp
		src/vinkan/wrappers/descriptors/descriptor_set.hpp

		src/vinkan/pipelines.hpp
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

