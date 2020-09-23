//
// Created by Igor Frank on 25.10.19.
//

#ifndef VULKANTEST_QUEUEFAMILYINDICES_H
#define VULKANTEST_QUEUEFAMILYINDICES_H


#include <optional>
#include <vulkan/vulkan.hpp>

class QueueFamilyIndices {
private:
    std::optional<uint32_t> graphicsFamily;

    std::optional<uint32_t> presentFamily;

    std::optional<uint32_t> computeFamily;

private:

    void findQueueFamilies(const vk::PhysicalDevice& device, const  vk::UniqueSurfaceKHR& surface);

public:
    explicit QueueFamilyIndices(const vk::PhysicalDevice& device, const vk::UniqueSurfaceKHR& surface);

    bool isComplete();

    bool isGrpahicsPresentEqual();

    [[nodiscard]] const uint32_t &getComputeFamily() const;

    [[nodiscard]] const uint32_t & getGraphicsFamily() const;

    [[nodiscard]] const uint32_t & getPresentFamily() const;
};


#endif //VULKANTEST_QUEUEFAMILYINDICES_H
