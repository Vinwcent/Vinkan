# Vinkan 🌋

> A C++ wrapper around Vulkan with type-safe enums for resource management.

Vinkan organizes Vulkan without hiding it. Use your own enums to identify resources, get RAII everywhere, keep full control when needed.

## 🎯 For Who?

**Primarily built for myself**, but also for Vulkan developers who want:
- Faster prototyping with readable code
- Type safety without losing Vulkan's power
- Less boilerplate, same control

You still need to know Vulkan. Vinkan just makes it cleaner.

## ⚡ What You Get

- **Compute shader**: init to execution in **< 200 lines**
- **Triangle rendering**: complete pipeline in **< 400 lines**  
- **Full Vulkan control** when you need it
- **Type-safe resource management** with your enums
- **Cross-platform** (Windows, Linux, macOS)

## 🚀 Quick Examples

### Compute in ~200 lines (cf. examples)
```cpp
#include <vinkan/vinkan.hpp>

// Your resource types
enum class MyQueue { COMPUTE };
enum class MyBuffer { DATA };
enum class MyPipeline { COMPUTE };

// Standard Vulkan setup
vinkan::Instance instance(instanceInfo);
vinkan::PhysicalDevice physicalDevice(deviceInfo, instance.getHandle());
auto device = vinkan::Device<MyQueue>::Builder(...)
    .addQueue(computeQueueRequest)
    .build();

// Type-safe resources
vinkan::Resources<MyBuffer, ...> resources(device->getHandle(), ...);
resources.create(MyBuffer::DATA, bufferInfo);

// Pipeline and execute
vinkan::Pipelines<MyPipeline, ...> pipelines(device->getHandle());
pipelines.createComputePipeline(MyPipeline::COMPUTE, pipelineInfo);
// ... dispatch and sync
```

### Compute in ~400 lines (cf. examples)
```cpp
// Graphics-specific enums
enum class MyAttachment { SWAPCHAIN };
enum class MyCommand { FRAME_1, FRAME_2 };

// Swapchain + render pass
vinkan::Swapchain swapchain(swapchainInfo);
auto renderPass = vinkan::RenderPass<MyAttachment>::Builder()
    .addAttachment(MyAttachment::SWAPCHAIN, attachmentDesc)
    .build(device->getHandle());

// Render loop
auto cmd = coordinator.beginCommandBuffer(MyCommand::FRAME_1);
renderStage->beginRenderPass(cmd, imageIndex);
pipelines.bindCmdBuffer(cmd, MyPipeline::GRAPHICS);
model.draw(cmd);
```

## 🏗️ Core Components

- **`Device<QueueEnum>`** - Typed queue management
- **`Resources<BufferEnum, DescriptorEnum, ...>`** - Type-safe containers
- **`Pipelines<PipelineEnum, LayoutEnum>`** - Compute + graphics pipelines
- **`CommandCoordinator<CommandEnum, PoolEnum>`** - Command buffer lifecycle
- **`Swapchain`** - Presentation management
- **`RenderPass<AttachmentEnum>`** - Render pass builder

## 📋 What's Implemented

✅ **Full compute pipeline** (buffers, descriptors, dispatch)  
✅ **Graphics rendering** (swapchain, render pass, vertex buffers)  
✅ **Command management** (single-use + long-lived)  
✅ **Synchronization** (fences, semaphores)  
✅ **RAII resource cleanup**  
✅ **Cross-platform support**

PS: Some features are still missing (images can't be created by the resources for example). I'm mainly adding stuff here when it becomes a pain to make it by myself with Vulkan.

## 🔧 Build

```bash
git clone https://github.com/Vinwcent/vinkan.git
cd vinkan && cmake -B build && cmake --build build
```

**Requirements**: C++20, Vulkan SDK 1.2+, CMake 3.15+
**macOS**: Needs MoltenVK

## 📚 Examples

Check out the [examples](examples/) folder:

- **Compute**: Buffer operations with compute shaders
- **Triangle**: Basic graphics pipeline with vertex rendering

Both examples show real working code from init to execution.

---

**Tame Vulkan with less boilerplate** 🌋
