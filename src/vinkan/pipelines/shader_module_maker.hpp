#ifndef VINKAN_SHADER_MODULE_MAKER_HPP
#define VINKAN_SHADER_MODULE_MAKER_HPP
#include <vulkan/vulkan.h>

#include <cstddef>
#include <string>
#include <vector>

namespace vinkan {

struct ShaderRawInfo {
  const unsigned char *shaderData;
  const size_t shaderSize;
  VkShaderStageFlagBits shaderStage;
};

struct ShaderFileInfo {
  std::string shaderFilepath;
  VkShaderStageFlagBits shaderStage;
};

class ShaderModuleMaker {
 public:
  ShaderModuleMaker(VkDevice device);
  ~ShaderModuleMaker();

  VkPipelineShaderStageCreateInfo operator()(ShaderRawInfo shaderInfo);
  VkPipelineShaderStageCreateInfo operator()(ShaderFileInfo shaderInfo);

 private:
  VkDevice device_;
  std::vector<VkShaderModule> shaderModules_{};

  VkShaderModule createShaderModule_(const std::vector<char> &code);
};
}  // namespace vinkan
#endif
