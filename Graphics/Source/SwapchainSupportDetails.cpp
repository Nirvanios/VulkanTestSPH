//
// Created by Igor Frank on 25.10.19.
//

#include "SwapchainSupportDetails.h"

SwapchainSupportDetails::SwapchainSupportDetails(const vk::PhysicalDevice& device, const vk::UniqueSurfaceKHR& surface) {

    capabilities = device.getSurfaceCapabilitiesKHR(surface.get());

    formats = device.getSurfaceFormatsKHR(surface.get());

    presentModes = device.getSurfacePresentModesKHR(surface.get());
}

bool SwapchainSupportDetails::isSwapchainAdequate() {
    return !formats.empty() && !presentModes.empty();
}

const vk::SurfaceCapabilitiesKHR &SwapchainSupportDetails::getCapabilities() const {
    return capabilities;
}

const std::vector<vk::SurfaceFormatKHR> &SwapchainSupportDetails::getFormats() const {
    return formats;
}

const std::vector<vk::PresentModeKHR> &SwapchainSupportDetails::getPresentModes() const {
    return presentModes;
}
