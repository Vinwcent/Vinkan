#include "sampler.hpp"

#include "vinkan/logging/logger.hpp"

// std
#include <stdexcept>

namespace vinkan {

Sampler::Sampler(VkDevice device, SamplerInfo samplerInfo)
    : device_(device),
      magFilter{samplerInfo.magFilter},
      minFilter{samplerInfo.minFilter},
      mipmapMode{samplerInfo.mipmapMode},
      addressModeU{samplerInfo.addressModeU},
      addressModeV{samplerInfo.addressModeV},
      addressModeW{samplerInfo.addressModeW},
      mipLodBias{samplerInfo.mipLodBias},
      anisotropyEnable{samplerInfo.anisotropyEnable},
      maxAnisotropy{samplerInfo.maxAnisotropy},
      compareEnable{samplerInfo.compareEnable},
      compareOp{samplerInfo.compareOp},
      minLod{samplerInfo.minLod},
      maxLod{samplerInfo.maxLod},
      borderColor{samplerInfo.borderColor},
      unnormalizedCoordinates{samplerInfo.unnormalizedCoordinates} {
  VkSamplerCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  createInfo.magFilter = samplerInfo.magFilter;
  createInfo.minFilter = samplerInfo.minFilter;
  createInfo.mipmapMode = samplerInfo.mipmapMode;
  createInfo.addressModeU = samplerInfo.addressModeU;
  createInfo.addressModeV = samplerInfo.addressModeV;
  createInfo.addressModeW = samplerInfo.addressModeW;
  createInfo.mipLodBias = samplerInfo.mipLodBias;
  createInfo.anisotropyEnable = samplerInfo.anisotropyEnable;
  createInfo.maxAnisotropy = samplerInfo.maxAnisotropy;
  createInfo.compareEnable = samplerInfo.compareEnable;
  createInfo.compareOp = samplerInfo.compareOp;
  createInfo.minLod = samplerInfo.minLod;
  createInfo.maxLod = samplerInfo.maxLod;
  createInfo.borderColor = samplerInfo.borderColor;
  createInfo.unnormalizedCoordinates = samplerInfo.unnormalizedCoordinates;

  if (vkCreateSampler(device_, &createInfo, nullptr, &handle_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create sampler!");
  }

  SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Sampler created");
}

Sampler::~Sampler() {
  if (isHandleValid()) {
    vkDestroySampler(device_, handle_, nullptr);
  }
}

}  // namespace vinkan
