#ifndef VINKAN_DESCRIPTORS_STRUCTS_HPP
#define VINKAN_DESCRIPTORS_STRUCTS_HPP

#include <cstdint>
#include <vector>

#include "vinkan/wrappers/descriptors/descriptor_set_layout.hpp"
namespace vinkan {

struct SetLayoutInfo {
  uint32_t nSets;
  std::vector<DescriptorSetLayoutBinding> bindings;
};

}  // namespace vinkan

#endif

