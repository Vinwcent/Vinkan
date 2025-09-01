#include <vinkan/vinkan.hpp>

#include "m_series_portability.hpp"

const std::vector<const char *> MyAppValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

enum class MyAppQueue { COMPUTE_QUEUE };
enum class MyAppDescriptorSet { SIMPLE_DESCRIPTOR_SET };
enum class MyAppDescriptorSetLayout { SIMPLE_DESCRIPTOR_SET_LAYOUT };
enum class MyAppDescriptorPool { SIMPLE_DESCRIPTOR_POOL };

using ResourceBinder =
    vinkan::ResourcesBinder<MyAppDescriptorSet, MyAppDescriptorSetLayout,
                            MyAppDescriptorPool>;

int main() {
  // INSTANCE CREATION
  std::vector<const char *> extraExtensions{};
  vinkan::InstanceInfo instanceInfo{
      .appName = "Example compute app",
      .appVersion = vinkan::SemanticVersion{.major = 0, .minor = 0, .patch = 1},
      .engineName = "Example compute engine",
      .engineVersion =
          vinkan::SemanticVersion{.major = 0, .minor = 0, .patch = 1},
      .apiVersion = VK_API_VERSION_1_2,
      .validationLayers = MyAppValidationLayers,
      .includePortabilityExtensions = NEED_PORTABILITY_EXTENSIONS,
      .extraVkExtensions = extraExtensions};

  vinkan::Instance instance(instanceInfo);
  // PHYSICAL DEVICE CREATION WITH SUPPORT FOR COMPUTE QUEUE
  vinkan::PhysicalDeviceInfo physicalDeviceInfo{
      .requestedQueueFlags = {VK_QUEUE_COMPUTE_BIT},
      .surfaceSupportRequested = std::nullopt,
      .extensions = DEVICE_EXTENSIONS};
  vinkan::PhysicalDevice physicalDevice(physicalDeviceInfo,
                                        instance.getHandle());

  // DEVICE CREATION WITH ONE COMPUTE QUEUE
  vinkan::Device<MyAppQueue>::Builder deviceBuilder(physicalDevice.getHandle(),
                                                    physicalDevice.getQueues());
  deviceBuilder.addExtensions(DEVICE_EXTENSIONS);
  vinkan::QueueFamilyRequest<MyAppQueue> queueRequest{
      .queueFamilyIdentifier = MyAppQueue::COMPUTE_QUEUE,
      .flagsRequested = VK_QUEUE_COMPUTE_BIT,
      .surfacePresentationSupport = std::nullopt,
      .nQueues = 1,
      .queuePriorities = {1.0}};
  bool success = false;
  deviceBuilder.addQueue(queueRequest, true, success);
  assert(success);
  auto device = deviceBuilder.build();

  ResourceBinder resourceBinder(device->getHandle());
  // SET LAYOUT
  vinkan::SetLayoutInfo layoutInfo{
      .nSets = 1,
      .bindings =
          {
              vinkan::DescriptorSetLayoutBinding{
                  .bindingIndex = 0,
                  .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                  .shaderStageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                  .count = 1,
              },
          },
  };
  resourceBinder.createSetLayout(
      MyAppDescriptorSetLayout::SIMPLE_DESCRIPTOR_SET_LAYOUT, layoutInfo);

  // POOL FROM THE SET LAYOUT
  resourceBinder.createPool(
      MyAppDescriptorPool::SIMPLE_DESCRIPTOR_POOL,
      {MyAppDescriptorSetLayout::SIMPLE_DESCRIPTOR_SET_LAYOUT});

  // BUFFER CREATION
  vinkan::BufferInfo bufferInfo{
      .instanceSize = 256,
      .instanceCount = 1,
      .usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      .sharingMode = {.value = VK_SHARING_MODE_EXCLUSIVE},
      .memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };
  vinkan::Buffer buffer(device->getHandle(),
                        physicalDevice.getMemoryProperties(), bufferInfo);

  // DESCRIPTOR SET CREATION (Write directly)
  vinkan::ResourceDescriptorInfo resourceInfo{
      .bindingIndex = 0,
      .vkBufferInfo = {buffer.descriptorInfo()},
  };
  resourceBinder.createSet(
      MyAppDescriptorSet::SIMPLE_DESCRIPTOR_SET,
      MyAppDescriptorSetLayout::SIMPLE_DESCRIPTOR_SET_LAYOUT, {resourceInfo});
}
