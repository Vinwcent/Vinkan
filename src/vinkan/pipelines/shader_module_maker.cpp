#include "shader_module_maker.hpp"

#include "vinkan/utils/file_io.hpp"
namespace vinkan {

ShaderModuleMaker::ShaderModuleMaker(VkDevice device) : device_(device) {}

ShaderModuleMaker::~ShaderModuleMaker() {
  if (!device_) {
    return;
  }
  for (auto shaderModule : shaderModules_) {
    vkDestroyShaderModule(device_, shaderModule, nullptr);
  }
}

VkShaderModule ShaderModuleMaker::createShaderModule_(
    const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }
  SPDLOG_LOGGER_INFO(get_vinkan_logger(), "A shader module has been created");
  shaderModules_.push_back(shaderModule);
  return shaderModule;
}

VkPipelineShaderStageCreateInfo ShaderModuleMaker::operator()(
    ShaderRawInfo shaderInfo) {
  std::vector<char> shaderCode(shaderInfo.shaderData,
                               shaderInfo.shaderData + shaderInfo.shaderSize);
  auto shaderModule = createShaderModule_(shaderCode);

  VkPipelineShaderStageCreateInfo shaderStage;
  shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStage.stage = shaderInfo.shaderStage;
  shaderStage.module = shaderModule;
  shaderStage.pName = "main";
  shaderStage.flags = 0;
  shaderStage.pNext = nullptr;
  shaderStage.pSpecializationInfo = nullptr;

  return shaderStage;
}

VkPipelineShaderStageCreateInfo ShaderModuleMaker::operator()(
    ShaderFileInfo shaderInfo) {
  auto shaderCode = vinkan::readTextFile(shaderInfo.shaderFilepath);
  auto shaderData = reinterpret_cast<const unsigned char *>(shaderCode.data());
  auto shaderSize = shaderCode.size();
  return (*this)(ShaderRawInfo{.shaderData = shaderData,
                               .shaderSize = shaderSize,
                               .shaderStage = shaderInfo.shaderStage});
}

}  // namespace vinkan

