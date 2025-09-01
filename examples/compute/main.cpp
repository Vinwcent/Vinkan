#include <vinkan/vinkan.hpp>

#include "m_series_portability.hpp"

const std::vector<const char *> MyAppValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

enum class MyAppQueue { COMPUTE_QUEUE };
enum class MyAppDescriptorSet { SIMPLE_DESCRIPTOR_SET };
enum class MyAppDescriptorSetLayout { SIMPLE_DESCRIPTOR_SET_LAYOUT };
enum class MyAppDescriptorPool { SIMPLE_DESCRIPTOR_POOL };
enum class MyAppBuffers { SIMPLE_BUFFER };

using MyAppResources =
    vinkan::VinkanResources<MyAppBuffers, MyAppDescriptorSet,
                            MyAppDescriptorSetLayout, MyAppDescriptorPool>;

int main() {
  // Create the instance
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

  // Create the physical device
  vinkan::PhysicalDeviceInfo physicalDeviceInfo{
      .requestedQueueFlags = {VK_QUEUE_COMPUTE_BIT},
      .surfaceSupportRequested = std::nullopt,
      .extensions = DEVICE_EXTENSIONS};
  vinkan::PhysicalDevice physicalDevice(physicalDeviceInfo,
                                        instance.getHandle());

  // Create the device (with a one compute queue on one queue family)
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
  bool mustBeInNewQueueFamily = true;
  deviceBuilder.addQueue(queueRequest, mustBeInNewQueueFamily, success);
  assert(success);
  auto device = deviceBuilder.build();

  // Initialize the resources
  MyAppResources resources(device->getHandle(),
                           physicalDevice.getMemoryProperties());
  // Create a set layout with one storage buffer on binding 0
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
  resources.createSetLayout(
      MyAppDescriptorSetLayout::SIMPLE_DESCRIPTOR_SET_LAYOUT, layoutInfo);

  // Create a pool from this layout info
  resources.createPool(
      MyAppDescriptorPool::SIMPLE_DESCRIPTOR_POOL,
      {MyAppDescriptorSetLayout::SIMPLE_DESCRIPTOR_SET_LAYOUT});

  // Create a buffer
  vinkan::BufferInfo bufferInfo{
      .instanceSize = 256,
      .instanceCount = 1,
      .usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      .sharingMode = {.value = VK_SHARING_MODE_EXCLUSIVE},
      .memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };
  resources.create(MyAppBuffers::SIMPLE_BUFFER, bufferInfo);

  // Create a descriptor set with the buffer at binding 0
  vinkan::VinkanBufferBinding<MyAppBuffers> binding{
      .bindingIndex = 0, .buffer = MyAppBuffers::SIMPLE_BUFFER};
  resources.createSet(MyAppDescriptorSet::SIMPLE_DESCRIPTOR_SET,
                      MyAppDescriptorSetLayout::SIMPLE_DESCRIPTOR_SET_LAYOUT,
                      {binding});
}
