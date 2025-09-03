#ifndef VINKAN_CONCEPTS_HPP
#define VINKAN_CONCEPTS_HPP

#include <type_traits>

#include "vinkan/pipelines/shader_module_maker.hpp"
template <typename T>
concept EnumType = std::is_enum_v<T>;

template <typename T>
concept ValidShaderInfo = std::is_same_v<T, vinkan::ShaderFileInfo> ||
                          std::is_same_v<T, vinkan::ShaderRawInfo>;
#endif

