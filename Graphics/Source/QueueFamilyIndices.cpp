//
// Created by Igor Frank on 25.10.19.
//

#include "QueueFamilyIndices.h"

bool QueueFamilyIndices::isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
}

QueueFamilyIndices::QueueFamilyIndices(const vk::PhysicalDevice &device, const vk::UniqueSurfaceKHR &surface) {
    findQueueFamilies(device, surface);
}

void QueueFamilyIndices::findQueueFamilies(const vk::PhysicalDevice &device, const vk::UniqueSurfaceKHR &surface) {

    auto queueFamilies = device.getQueueFamilyProperties();

    int queueFamilyIndex = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily = queueFamilyIndex;
        }

        auto presentSupport = device.getSurfaceSupportKHR(queueFamilyIndex, surface.get());

        if (queueFamily.queueCount > 0 && presentSupport) {
            presentFamily = queueFamilyIndex;
        }

        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eCompute){
            computeFamily = queueFamilyIndex;
        }

        if (isComplete()) {
            break;
        }

        queueFamilyIndex++;
    }
}

const uint32_t &QueueFamilyIndices::getGraphicsFamily() const {
    return graphicsFamily.value();
}

const uint32_t &QueueFamilyIndices::getPresentFamily() const {
    return presentFamily.value();
}

bool QueueFamilyIndices::isGrpahicsPresentEqual() {
    return graphicsFamily == presentFamily;
}

const uint32_t &QueueFamilyIndices::getComputeFamily() const {
    return computeFamily.value();
}


