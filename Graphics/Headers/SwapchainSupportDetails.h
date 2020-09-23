//
// Created by Igor Frank on 25.10.19.
//

#ifndef VULKANTEST_SWAPCHAINSUPPORTDETAILS_H
#define VULKANTEST_SWAPCHAINSUPPORTDETAILS_H

#include <vulkan/vulkan.hpp>

class SwapchainSupportDetails {
private:
    vk::SurfaceCapabilitiesKHR capabilities;
public:
    [[nodiscard]] const vk::SurfaceCapabilitiesKHR &getCapabilities() const;

    [[nodiscard]] const std::vector<vk::SurfaceFormatKHR> &getFormats() const;

    [[nodiscard]] const std::vector<vk::PresentModeKHR> &getPresentModes() const;

private:
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
public:
    SwapchainSupportDetails(const vk::PhysicalDevice &device, const vk::UniqueSurfaceKHR &surface);

    bool isSwapchainAdequate();
};


#endif //VULKANTEST_SWAPCHAINSUPPORTDETAILS_H
