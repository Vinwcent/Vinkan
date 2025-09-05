#include <iostream>
#include <vinkan/pipelines/shader_module_maker.hpp>
#include <vinkan/vinkan.hpp>

#include "m_series_portability.hpp"

const std::vector<const char *> MyAppValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

// Queue
enum class MyAppQueue { COMPUTE_QUEUE };

// Resources
enum class MyAppDescriptorSet { SIMPLE_DESCRIPTOR_SET };
enum class MyAppDescriptorSetLayout { SIMPLE_DESCRIPTOR_SET_LAYOUT };
enum class MyAppDescriptorPool { SIMPLE_DESCRIPTOR_POOL };
enum class MyAppBuffers { SIMPLE_BUFFER };

// Pipeline
enum class MyAppPipeline { COMPUTE_PIPELINE };
enum class MyAppPipelineLayout { COMPUTE_PIP_LAYOUT };

// Command buffers
enum class MyAppCommandBuffer {};
enum class MyAppCommandPool { SINGLE_USE_COMPUTE_POOL };
enum class MyAppFence { COMPUTE_FENCE };
enum class MyAppSemaphore { COMPUTE_SEMAPHORE };

// Push constants
struct MyAppPC {
  uint32_t value;
};

using MyAppResources =
    vinkan::Resources<MyAppBuffers, MyAppDescriptorSet,
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
      .instanceSize = 64 * sizeof(uint32_t),
      .instanceCount = 1,
      .usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      .sharingMode = {.value = VK_SHARING_MODE_EXCLUSIVE},
      .memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };
  resources.create(MyAppBuffers::SIMPLE_BUFFER, bufferInfo);

  // Fill the buffer with value 22 everywhere
  vinkan::Buffer &buffer = resources.get(MyAppBuffers::SIMPLE_BUFFER);
  std::vector<uint32_t> bufferData(64, 10);
  buffer.map();
  buffer.writeToBuffer(bufferData.data(), VK_WHOLE_SIZE, 0);
  buffer.unmap();

  // Create a descriptor set with the buffer at binding 0
  vinkan::VinkanBufferBinding<MyAppBuffers> binding{
      .bindingIndex = 0, .buffer = MyAppBuffers::SIMPLE_BUFFER};
  resources.createSet(MyAppDescriptorSet::SIMPLE_DESCRIPTOR_SET,
                      MyAppDescriptorSetLayout::SIMPLE_DESCRIPTOR_SET_LAYOUT,
                      {binding});

  // Initialize the pipelines
  vinkan::Pipelines<MyAppPipeline, MyAppPipelineLayout> pipelines_(
      device->getHandle());
  // Create a pipeline layout
  pipelines_.createLayout<MyAppPC>(
      MyAppPipelineLayout::COMPUTE_PIP_LAYOUT,
      {resources.get(MyAppDescriptorSetLayout::SIMPLE_DESCRIPTOR_SET_LAYOUT)},
      VK_SHADER_STAGE_COMPUTE_BIT);

  // Create a pipeline with this layout
  vinkan::ShaderFileInfo shaderFileInfo{
      .shaderFilepath =
          std::string(COMPILED_SHADERS_DIR) + "/addition_shader.spv",
      .shaderStage = VK_SHADER_STAGE_COMPUTE_BIT};
  vinkan::ComputePipelineInfo<MyAppPipelineLayout, vinkan::ShaderFileInfo>
      computePipelineInfo{
          .layoutIdentifier = MyAppPipelineLayout::COMPUTE_PIP_LAYOUT,
          .shaderInfo = shaderFileInfo,
      };
  pipelines_.createComputePipeline(MyAppPipeline::COMPUTE_PIPELINE,
                                   computePipelineInfo);

  // Create a fence
  vinkan::SyncMechanisms<MyAppFence, MyAppSemaphore> syncMechanisms(
      device->getHandle());
  syncMechanisms.createFence(MyAppFence::COMPUTE_FENCE);
  auto fence = syncMechanisms.getFence(MyAppFence::COMPUTE_FENCE);

  // Initialize commands
  vinkan::CommandCoordinator<MyAppCommandBuffer, MyAppCommandPool> coordinator(
      device->getHandle());
  // Create a single use pool
  coordinator.createCommandPool(
      MyAppCommandPool::SINGLE_USE_COMPUTE_POOL,
      device->getQueueFamilyIndex(MyAppQueue::COMPUTE_QUEUE), true);

  // Create a single command in this pool
  VkCommandBuffer commandBuffer = coordinator.createSingleUseCommandBuffer(
      MyAppCommandPool::SINGLE_USE_COMPUTE_POOL);

  // Begin and record command
  coordinator.beginCommandBuffer(commandBuffer);
  pipelines_.bindCmdBuffer(commandBuffer, MyAppPipeline::COMPUTE_PIPELINE);
  VkDescriptorSet descSet =
      resources.get(MyAppDescriptorSet::SIMPLE_DESCRIPTOR_SET);
  vkCmdBindDescriptorSets(
      commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
      pipelines_.get(MyAppPipelineLayout::COMPUTE_PIP_LAYOUT), 0, 1, &descSet,
      0, nullptr);
  MyAppPC pushConstants{.value = 20};
  vkCmdPushConstants(
      commandBuffer, pipelines_.get(MyAppPipelineLayout::COMPUTE_PIP_LAYOUT),
      VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(MyAppPC), &pushConstants);
  vkCmdDispatch(commandBuffer, 1, 1, 1);
  coordinator.endCommandBuffer(commandBuffer);

  // Submit the command
  vinkan::SubmitCommandBufferInfo submitInfo{
      .signalFence = fence,
      .queue = device->getQueue(MyAppQueue::COMPUTE_QUEUE, 0)};
  coordinator.submitCommandBuffer(commandBuffer, submitInfo);

  // Wait execution
  vkWaitForFences(device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);

  // Read first element
  buffer.map();
  buffer.readBuffer(bufferData.data());
  buffer.unmap();
  std::cout << "Finished, first element: " << bufferData[0] << std::endl;
}
