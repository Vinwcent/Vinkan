#ifndef VINKAN_SAMPLER_HPP
#define VINKAN_SAMPLER_HPP

#include <vulkan/vulkan.h>

#include "vinkan/generics/ptr_handle_wrapper.hpp"

namespace vinkan {

struct SamplerInfo {
  VkFilter magFilter = VK_FILTER_LINEAR;
  VkFilter minFilter = VK_FILTER_LINEAR;
  VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  float mipLodBias = 0.0f;
  VkBool32 anisotropyEnable = VK_FALSE;
  float maxAnisotropy = 1.0f;
  VkBool32 compareEnable = VK_FALSE;
  VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
  float minLod = 0.0f;
  float maxLod = VK_LOD_CLAMP_NONE;
  VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  VkBool32 unnormalizedCoordinates = VK_FALSE;
};

class Sampler : public PtrHandleWrapper<VkSampler> {
 public:
  Sampler(VkDevice device, SamplerInfo samplerInfo);
  ~Sampler();

  Sampler(const Sampler&) = delete;
  Sampler& operator=(const Sampler&) = delete;

  // Getters
  VkFilter getMagFilter() const { return magFilter; }
  VkFilter getMinFilter() const { return minFilter; }
  VkSamplerMipmapMode getMipmapMode() const { return mipmapMode; }
  VkSamplerAddressMode getAddressModeU() const { return addressModeU; }
  VkSamplerAddressMode getAddressModeV() const { return addressModeV; }
  VkSamplerAddressMode getAddressModeW() const { return addressModeW; }
  float getMipLodBias() const { return mipLodBias; }
  VkBool32 getAnisotropyEnable() const { return anisotropyEnable; }
  float getMaxAnisotropy() const { return maxAnisotropy; }
  VkBool32 getCompareEnable() const { return compareEnable; }
  VkCompareOp getCompareOp() const { return compareOp; }
  float getMinLod() const { return minLod; }
  float getMaxLod() const { return maxLod; }
  VkBorderColor getBorderColor() const { return borderColor; }
  VkBool32 getUnnormalizedCoordinates() const {
    return unnormalizedCoordinates;
  }

 private:
  VkDevice device_;
  VkFilter magFilter;
  VkFilter minFilter;
  VkSamplerMipmapMode mipmapMode;
  VkSamplerAddressMode addressModeU;
  VkSamplerAddressMode addressModeV;
  VkSamplerAddressMode addressModeW;
  float mipLodBias;
  VkBool32 anisotropyEnable;
  float maxAnisotropy;
  VkBool32 compareEnable;
  VkCompareOp compareOp;
  float minLod;
  float maxLod;
  VkBorderColor borderColor;
  VkBool32 unnormalizedCoordinates;
};

}  // namespace vinkan

#endif
