#ifndef VINKAN_CONCEPTS_HPP
#define VINKAN_CONCEPTS_HPP

#include <type_traits>
template <typename T>
concept EnumType = std::is_enum_v<T>;

#endif

