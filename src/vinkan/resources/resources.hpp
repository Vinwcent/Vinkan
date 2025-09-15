#ifndef VINKAN_RESOURCES_HPP
#define VINKAN_RESOURCES_HPP

#include <vulkan/vulkan.h>

#include <map>
#include <memory>
#include <variant>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/resources/image_resource.hpp"
#include "vinkan/resources/resources_binder.hpp"
#include "vinkan/wrappers/buffer.hpp"
#include "vinkan/wrappers/image.hpp"
#include "vinkan/wrappers/image_view.hpp"
#include "vinkan/wrappers/sampler.hpp"

namespace vinkan {

template <EnumType BufferT>
struct VinkanBufferBinding {
  uint32_t bindingIndex;
  BufferT buffer;
};

template <EnumType ImageResourceT>
struct VinkanImageResourceBinding {
  uint32_t bindingIndex;
  ImageResourceT imageResource;
  VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
};

template <EnumType BufferT, EnumType ImageT, EnumType ImageResourceT,
          EnumType ImageViewT, EnumType SamplerT, EnumType SetT,
          EnumType SetLayoutT, EnumType PoolT>
class Resources {
 public:
  Resources(VkDevice device,
            VkPhysicalDeviceMemoryProperties deviceMemoryProperties)
      : device_(device),
        deviceMemoryProperties_(deviceMemoryProperties),
        resourcesBinder_(device) {}

  void create(BufferT bufferIdentifier, BufferInfo bufferInfo) {
    assert(!buffers_.contains(bufferIdentifier));
    buffers_.emplace(
        bufferIdentifier,
        std::make_unique<Buffer>(device_, deviceMemoryProperties_, bufferInfo));
    SPDLOG_LOGGER_INFO(
        get_vinkan_logger(),
        "Buffer " + std::string(magic_enum::enum_name(bufferIdentifier)) +
            " created.");
  }

  void create(ImageT imageIdentifier, ImageInfo imageInfo) {
    assert(!images_.contains(imageIdentifier));
    images_.emplace(
        imageIdentifier,
        std::make_unique<Image>(device_, deviceMemoryProperties_, imageInfo));
    SPDLOG_LOGGER_INFO(get_vinkan_logger(),
                       "Image " +
                           std::string(magic_enum::enum_name(imageIdentifier)) +
                           " created.");
  }

  void create(ImageViewT imageViewIdentifier, ImageViewInfo imageViewInfo) {
    assert(!imageViews_.contains(imageViewIdentifier));
    imageViews_.emplace(imageViewIdentifier,
                        std::make_unique<ImageView>(device_, imageViewInfo));
    SPDLOG_LOGGER_INFO(
        get_vinkan_logger(),
        "ImageView " + std::string(magic_enum::enum_name(imageViewIdentifier)) +
            " created.");
  }

  void create(SamplerT samplerIdentifier, SamplerInfo samplerInfo) {
    assert(!samplers_.contains(samplerIdentifier));
    samplers_.emplace(samplerIdentifier,
                      std::make_unique<Sampler>(device_, samplerInfo));
    SPDLOG_LOGGER_INFO(
        get_vinkan_logger(),
        "Sampler " + std::string(magic_enum::enum_name(samplerIdentifier)) +
            " created.");
  }

  void create(ImageResourceT imageResourceIdentifier,
              ImageViewT imageViewIdentifier, SamplerT samplerIdentifier) {
    assert(!imageResources_.contains(imageResourceIdentifier));
    assert(imageViews_.contains(imageViewIdentifier) &&
           "ImageView must exist before creating ImageResource");
    assert(samplers_.contains(samplerIdentifier) &&
           "Sampler must exist before creating ImageResource");

    auto& imageView = *imageViews_[imageViewIdentifier];
    auto& sampler = *samplers_[samplerIdentifier];

    imageResources_.emplace(
        imageResourceIdentifier,
        std::make_unique<ImageResource>(imageView, sampler));
    SPDLOG_LOGGER_INFO(
        get_vinkan_logger(),
        "ImageResource " +
            std::string(magic_enum::enum_name(imageResourceIdentifier)) +
            " created.");
  }

  void create(ImageResourceT imageResourceIdentifier,
              ImageViewT imageViewIdentifier) {
    assert(!imageResources_.contains(imageResourceIdentifier));
    assert(imageViews_.contains(imageViewIdentifier) &&
           "ImageView must exist before creating ImageResource");

    auto& imageView = *imageViews_[imageViewIdentifier];

    imageResources_.emplace(imageResourceIdentifier,
                            std::make_unique<ImageResource>(imageView));
    SPDLOG_LOGGER_INFO(
        get_vinkan_logger(),
        "ImageResource " +
            std::string(magic_enum::enum_name(imageResourceIdentifier)) +
            " created (storage image).");
  }

  Buffer& get(BufferT bufferIdentifier) {
    assert(buffers_.contains(bufferIdentifier));
    return *buffers_[bufferIdentifier];
  }

  Image& get(ImageT imageIdentifier) {
    assert(images_.contains(imageIdentifier));
    return *images_[imageIdentifier];
  }

  ImageView& get(ImageViewT imageViewIdentifier) {
    assert(imageViews_.contains(imageViewIdentifier));
    return *imageViews_[imageViewIdentifier];
  }

  Sampler& get(SamplerT samplerIdentifier) {
    assert(samplers_.contains(samplerIdentifier));
    return *samplers_[samplerIdentifier];
  }

  ImageResource& get(ImageResourceT imageResourceIdentifier) {
    assert(imageResources_.contains(imageResourceIdentifier));
    return *imageResources_[imageResourceIdentifier];
  }

  VkDescriptorSet get(SetT descriptorSetIdentifier) {
    return resourcesBinder_.get(descriptorSetIdentifier);
  }

  VkDescriptorSetLayout get(SetLayoutT descriptorSetLayoutIdentifier) {
    return resourcesBinder_.get(descriptorSetLayoutIdentifier);
  }

  VkDescriptorPool get(PoolT descriptorPool) {
    return resourcesBinder_.get(descriptorPool);
  }

  void createPool(PoolT pool,
                  const std::vector<SetLayoutT>& setLayoutIdentifiers) {
    resourcesBinder_.createPool(pool, setLayoutIdentifiers);
  }

  void createSetLayout(SetLayoutT setLayout, SetLayoutInfo layoutInfo) {
    resourcesBinder_.createSetLayout(setLayout, layoutInfo);
  }

  void createSet(
      SetT setIdentifier, SetLayoutT setLayoutIdentifier,
      const std::vector<
          std::variant<VinkanBufferBinding<BufferT>,
                       VinkanImageResourceBinding<ImageResourceT>>>& bindings) {
    std::vector<AnyDescriptorInfo> resourceDescriptorInfos{};

    for (const auto& binding : bindings) {
      std::visit(
          [&](const auto& specificBinding) {
            auto descriptorInfo = getResourceDescriptorInfo(specificBinding);
            resourceDescriptorInfos.push_back(descriptorInfo);
          },
          binding);
    }

    resourcesBinder_.createSet(setIdentifier, setLayoutIdentifier,
                               resourceDescriptorInfos);
  }

 private:
  BufferDescriptorInfo getResourceDescriptorInfo(
      const VinkanBufferBinding<BufferT>& binding) {
    assert(buffers_.contains(binding.buffer));
    auto& buffer = buffers_.at(binding.buffer);
    return BufferDescriptorInfo{binding.bindingIndex,
                                {buffer->descriptorInfo()}};
  }

  ImageDescriptorInfo getResourceDescriptorInfo(
      const VinkanImageResourceBinding<ImageResourceT>& binding) {
    assert(imageResources_.contains(binding.imageResource));
    auto& imageResource = imageResources_.at(binding.imageResource);
    return ImageDescriptorInfo{binding.bindingIndex,
                               {imageResource->descriptorInfo(binding.layout)}};
  }

  std::map<BufferT, std::unique_ptr<Buffer>> buffers_;
  std::map<ImageT, std::unique_ptr<Image>> images_;
  std::map<ImageViewT, std::unique_ptr<ImageView>> imageViews_;
  std::map<SamplerT, std::unique_ptr<Sampler>> samplers_;
  std::map<ImageResourceT, std::unique_ptr<ImageResource>> imageResources_;

  VkDevice device_;
  VkPhysicalDeviceMemoryProperties deviceMemoryProperties_;
  ResourcesBinder<SetT, SetLayoutT, PoolT> resourcesBinder_;
};

}  // namespace vinkan

#endif
