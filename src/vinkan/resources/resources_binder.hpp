#ifndef VINKAN_RESOURCES_BINDER_HPP
#define VINKAN_RESOURCES_BINDER_HPP
#include <cassert>
#include <map>
#include <numeric>
#include <vector>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/logging/logger.hpp"
#include "vinkan/structs/descriptors_structs.hpp"
#include "vinkan/wrappers/descriptors/descriptor_pool.hpp"
#include "vinkan/wrappers/descriptors/descriptor_set.hpp"
#include "vinkan/wrappers/descriptors/descriptor_set_layout.hpp"

namespace vinkan {

template <EnumType SetT, EnumType SetLayoutT, EnumType PoolT>
class ResourcesBinder {
 public:
  ResourcesBinder(VkDevice device) : device_(device) {}
  void createSetLayout(SetLayoutT setLayoutIdentifier,
                       SetLayoutInfo layoutInfo) {
    assert(!layoutIdentifierToInfo_.contains(setLayoutIdentifier) &&
           "This set layout is already defined");
    DescriptorSetLayout::Builder builder(device_);
    for (auto &layoutInfo : layoutInfo.bindings) {
      builder.addBinding(layoutInfo);
    }
    layoutIdentifierToInfo_.emplace(setLayoutIdentifier, layoutInfo);
    setLayouts_.emplace(setLayoutIdentifier, builder.build());
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Descriptor set layout created");
  }

  void createPool(PoolT pool,
                  const std::vector<SetLayoutT> &setLayoutIdentifiers) {
    std::vector<SetLayoutInfo> layoutInfos{};
    for (auto setLayoutIdentifier : setLayoutIdentifiers) {
      assert(layoutIdentifierToInfo_.contains(setLayoutIdentifier) &&
             "Cannot create a pool when one of the layout is not defined yet");
      layoutInfos.push_back(layoutIdentifierToInfo_[setLayoutIdentifier]);
      layoutIdentifierToPool_.emplace(setLayoutIdentifier, pool);
    }
    int totalNSets =
        std::accumulate(layoutInfos.begin(), layoutInfos.end(), 0,
                        [](int sum, const SetLayoutInfo &layoutInfo) {
                          return sum + layoutInfo.nSets;
                        });

    std::map<VkDescriptorType, int> setsPerDescriptor{};
    for (auto &layout : layoutInfos) {
      for (auto &binding : layout.bindings) {
        setsPerDescriptor[binding.descriptorType] +=
            layout.nSets * binding.count;
      }
    }
    DescriptorPool::Builder builder(device_);
    builder.setMaxSets(totalNSets);

    for (const auto &alloc : setsPerDescriptor) {
      builder.addPoolSize(alloc.first, alloc.second);
    }
    pools_.emplace(pool, builder.build());
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Descriptor pool created");
  };

  void createSet(
      SetT setIdentifier, SetLayoutT setLayoutIdentifier,
      const std::vector<ResourceDescriptorInfo> &resourceDescriptorInfos) {
    assert(setLayouts_.contains(setLayoutIdentifier));
    auto &setLayout = *setLayouts_[setLayoutIdentifier];
    auto &pool = *pools_[layoutIdentifierToPool_[setLayoutIdentifier]];
    DescriptorSet::Builder builder(device_, setLayout, pool);
    for (auto &resourceDescriptorInfo : resourceDescriptorInfos) {
      builder.setBuffer(resourceDescriptorInfo);
    }
    sets_.emplace(setIdentifier, builder.build());
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Descriptor set created");
  }

  VkDescriptorSet get(SetT setIdentifier) {
    assert(sets_.contains(setIdentifier));
    return sets_[setIdentifier]->getHandle();
  }
  VkDescriptorSetLayout get(SetLayoutT setLayoutIdentifier) {
    assert(setLayouts_.contains(setLayoutIdentifier));
    return setLayouts_[setLayoutIdentifier]->getHandle();
  }
  VkDescriptorPool get(PoolT poolIdentifier) {
    assert(pools_.contains(poolIdentifier));
    return pools_[poolIdentifier]->getHandle();
  }

 private:
  VkDevice device_;
  // We keep this to build the pool when needed
  std::map<SetLayoutT, SetLayoutInfo> layoutIdentifierToInfo_;
  std::map<SetLayoutT, PoolT> layoutIdentifierToPool_;

  std::map<SetT, std::unique_ptr<DescriptorSet>> sets_;
  std::map<SetLayoutT, std::unique_ptr<DescriptorSetLayout>> setLayouts_;
  std::map<PoolT, std::unique_ptr<DescriptorPool>> pools_;
};
}  // namespace vinkan
#endif
