#ifndef VINKAN_PIPELINE_INFO_HPP
#define VINKAN_PIPELINE_INFO_HPP
#include "vinkan/generics/concepts.hpp"
namespace vinkan {

template <EnumType PipelineLayoutT, ValidShaderInfo ShaderInfoT>
struct ComputePipelineInfo {
  PipelineLayoutT layoutIdentifier;
  ShaderInfoT shaderInfo;
};

}  // namespace vinkan
#endif
