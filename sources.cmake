set(VINKAN_SOURCES)
set(VINKAN_HEADERS)

list(APPEND VINKAN_SOURCES
		src/vinkan/glfw_vk_surface.cpp
)
list(APPEND VINKAN_HEADERS
		src/vinkan/glfw_vk_surface.hpp
)
set(VINKAN_FILES ${VINKAN_SOURCES} ${VINKAN_HEADERS})

