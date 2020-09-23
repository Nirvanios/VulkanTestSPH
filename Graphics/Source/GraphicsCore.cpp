//
// Created by Igor Frank on 24.10.19.
//

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 0

#include <vector>
#include <iostream>
#include <set>
#include <utility>
#include <QueueFamilyIndices.h>
#include <SwapchainSupportDetails.h>
#include <UniformBufferObject.h>
#include <chrono>
#include "GraphicsCore.h"
#include "../../misc/Headres/Utilities.h"

GraphicsCore::GraphicsCore(int width, int height, Window &window, Camera &camera, glm::vec3 particleDimensions) :
        window(window),
        camera(&camera),
        particleDimensions(particleDimensions) {
    initVulkan();
}

void GraphicsCore::drawFrame() {
    device.get().waitForFences(1, &inFlightFences[currentFrame].get(), VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    auto result = device.get().acquireNextImageKHR(swapchain.get(), UINT64_MAX,
                                                   imageAvailableSemaphores[currentFrame].get(), {}, &imageIndex);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        //recreateSwapChain();
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(imageIndex);

    vk::PipelineStageFlags waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore waitSemaphores{imageAvailableSemaphores[currentFrame].get()};
    vk::Semaphore signalSemaphores{renderFinishedSemaphores[currentFrame].get()};

    vk::SubmitInfo submitInfo{1, &waitSemaphores, &waitStages, 1, &commandBuffers[imageIndex].get(), 1,
                              &signalSemaphores};

    device.get().resetFences(1, &inFlightFences[currentFrame].get());

    graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame].get());

    vk::PresentInfoKHR presentInfo{1, &signalSemaphores, 1, &swapchain.get(), &imageIndex};

    result = presentQueue.presentKHR(&presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        //recreateSwapChain();
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void GraphicsCore::updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    std::vector<UniformBufferObject> ubo{};
    for (auto &object : objects) {
        ubo.emplace_back();
        auto &item = ubo.back();

        item.model = object.getModelMatrix();

        item.view = camera->GetViewMatrix();

        item.proj = glm::perspective(glm::radians(45.0f),
                                     swapchainExtent.width / (float) swapchainExtent.height,
                                     0.1f,
                                     100.0f);

        item.proj[1][1] *= -1;
        projectionMatrix = item.proj;
    }

    void *data;
    device.get().mapMemory(uniformBuffersMemory[currentImage].get(), 0, ubo.size() * sizeof(UniformBufferObject), {},
                           &data);
    memcpy(data, ubo.data(), ubo.size() * sizeof(UniformBufferObject));
    device.get().unmapMemory(uniformBuffersMemory[currentImage].get());
}

void GraphicsCore::recreateSwapChain() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        auto[w, h] = window.getFramebufferSize();
        width = w;
        height = h;
        glfwWaitEvents();
    }
    device.get().waitIdle();

    //cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createComputePipeline();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createGraphicsDescriptorPool();
    createComputeDescriptorPool();
    createGraphicsDescriptorSets();
    createComputeDescriptorSets();
    createCommandBuffers();
}

void GraphicsCore::initVulkan() {
    //vk::DynamicLoader dl;
    //auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
//    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    //TODO DISPATCH_LOADER_DYNAMIC
    createInstance();
    //VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());
    surface = window.createSurface(instance);
    pickPhysicalDevice();
    createLogicalDevice();
  setupDebugMessenger();
  createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsDescriptorSetLayout();
    createComputeDescriptorSetLayout();
    createGraphicsPipeline();
    createComputePipeline();
    createCommandPool();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    /*createTextureImage();
    createTextureImageView();
    createTextureSampler();*/
    auto loadedObjects = Utilities::loadModel("../Resources/Objects/ball.obj");
    objects.insert(objects.end(), loadedObjects.begin(), loadedObjects.end());
    loadedObjects = Utilities::loadModel("../Resources/Objects/Plane.obj");
    objects.insert(objects.end(), loadedObjects.begin(), loadedObjects.end());
    createVertexBuffer();
    createIndexBuffer();
    createShaderStorageBuffer();
    createUniformBuffers();
    createGraphicsDescriptorPool();
    createComputeDescriptorPool();
    createGraphicsDescriptorSets();
    createComputeDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

void GraphicsCore::setFramebufferResized(bool framebufferResized) {
    GraphicsCore::framebufferResized = framebufferResized;
}

void GraphicsCore::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<GraphicsCore *>(glfwGetWindowUserPointer(window));
    app->setFramebufferResized(true);
}

bool GraphicsCore::checkValidationLayerSupport() {
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    std::vector<std::string> layerNames(validationLayers.size());
    std::transform(validationLayers.begin(), validationLayers.end(), layerNames.begin(),
                   [](const char *const c) { return std::string{c}; });

    return std::any_of(availableLayers.begin(), availableLayers.end(), [layerNames](const auto &layerProperties) {
        return Utilities::isIn(std::string(layerProperties.layerName), layerNames);
    });
}

void GraphicsCore::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{"SPH", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0),
                                VK_API_VERSION_1_0};

    std::vector<const char *> extensions = getRequiredExtensions();

    auto validationLayerSize = 0;
    auto validationLayerData = validationLayers.data();
    vk::DebugUtilsMessengerCreateInfoEXT *pNext = nullptr;
    if (enableValidationLayers) {
        validationLayerSize = validationLayers.size();
        auto createInfo = generateDebugMessengerCreateInfo();
        pNext = &createInfo;
    }

    auto createInfo = vk::InstanceCreateInfo{{}, &appInfo, 1,
                                             validationLayerData,
                                             static_cast<uint32_t>(extensions.size()), extensions.data()};
    //createInfo.setPNext(pNext);

    instance = vk::createInstanceUnique(createInfo);
}

vk::DebugUtilsMessengerCreateInfoEXT GraphicsCore::generateDebugMessengerCreateInfo() {
    return vk::DebugUtilsMessengerCreateInfoEXT({}, vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                                                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                                                (PFN_vkDebugUtilsMessengerCallbackEXT) GraphicsCore::debugCallback,
                                                nullptr);
}

std::vector<const char *> GraphicsCore::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void GraphicsCore::setupDebugMessenger() {
    loaderDynamic.init(instance.get(), device.get());
//    vk::DynamicLoader dl;
//     auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
//    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
//    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

    if (enableValidationLayers) {
        auto createInfo = generateDebugMessengerCreateInfo();
        debugMessenger = instance.get().createDebugUtilsMessengerEXTUnique(createInfo, nullptr, loaderDynamic);
    }
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL GraphicsCore::debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagBitsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) {

    std::cerr << "[" << vk::to_string(messageSeverity) << "][" << vk::to_string(messageType) << "]: "
              << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void GraphicsCore::pickPhysicalDevice() {
    auto devices = instance->enumeratePhysicalDevices();

    if (devices.size() == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    for (const auto &device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            //msaaSamples = getMaxUsableSampleCount(); //TODO MSAA sampler
            break;
        }
    }

    if (physicalDevice == vk::PhysicalDevice{}) { //TODO check
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool GraphicsCore::isDeviceSuitable(const vk::PhysicalDevice &device) {
    auto deviceProperties = device.getProperties();
    auto deviceFeatures = device.getFeatures();

    QueueFamilyIndices indices(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport(device, surface);
        swapChainAdequate = swapchainSupport.isSwapchainAdequate();
    }

    vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();

    return deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
           deviceFeatures.geometryShader &&
           indices.isComplete() &&
           extensionsSupported &&
           swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

bool GraphicsCore::checkDeviceExtensionSupport(vk::PhysicalDevice device) {

    auto availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void GraphicsCore::createLogicalDevice() {
    QueueFamilyIndices indices(physicalDevice, surface);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.getGraphicsFamily(), indices.getPresentFamily(),
                                              indices.getComputeFamily()};

    const float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {

        queueCreateInfos.emplace_back();
        auto &queueCreateInfo = queueCreateInfos.back();
        queueCreateInfo.setQueueFamilyIndex(queueFamily);
        queueCreateInfo.setQueueCount(1u);
        queueCreateInfo.setPQueuePriorities(&queuePriority);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.setSamplerAnisotropy(VK_TRUE);
    deviceFeatures.setVertexPipelineStoresAndAtomics(VK_TRUE);
    deviceFeatures.setWideLines(VK_TRUE);
    deviceFeatures.setFillModeNonSolid(VK_TRUE);

    vk::DeviceCreateInfo createInfo{{}, static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(), 0,
                                    nullptr, static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(),
                                    &deviceFeatures};

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    device = physicalDevice.createDeviceUnique(createInfo);

    graphicsQueue = device.get().getQueue(indices.getGraphicsFamily(), 0);
    presentQueue = device.get().getQueue(indices.getPresentFamily(), 0);
    computeQueue = device.get().getQueue(indices.getComputeFamily(), 0);
}

void GraphicsCore::createSwapChain() {
    SwapchainSupportDetails swapChainSupport(physicalDevice, surface);

    auto capabilities = swapChainSupport.getCapabilities();

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.getFormats());
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.getPresentModes());
    vk::Extent2D extent = chooseSwapExtent(capabilities);

    uint32_t imageCount = swapChainSupport.getCapabilities().minImageCount + 1;

    if (capabilities.maxImageCount > 0 &&
        imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    imageCount = 2; //TODO storagebuffer

    vk::SwapchainCreateInfoKHR createInfo{{}, surface.get(), imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
                                          extent, 1, vk::ImageUsageFlagBits::eColorAttachment,
                                          vk::SharingMode::eExclusive, 0,
                                          nullptr, capabilities.currentTransform,
                                          vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, VK_TRUE,
                                          nullptr};

    QueueFamilyIndices indices(physicalDevice, surface);
    std::vector<uint32_t> queueFamilyIndices{indices.getGraphicsFamily(), indices.getPresentFamily()};

    if (!indices.isGrpahicsPresentEqual()) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        createInfo.setQueueFamilyIndexCount(2);
        createInfo.setPQueueFamilyIndices(queueFamilyIndices.data());
    }

    swapchain = device.get().createSwapchainKHRUnique(createInfo);
    swapchainImages = device.get().getSwapchainImagesKHR(swapchain.get());

    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;
}

vk::SurfaceFormatKHR GraphicsCore::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR GraphicsCore::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D GraphicsCore::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        auto[width, height] = window.getFramebufferSize();

        vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

void GraphicsCore::createImageViews() {
    swapChainImageViews.resize(swapchainImages.size());

    for (uint32_t i = 0; i < swapchainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapchainImages[i], swapchainImageFormat,
                                                 vk::ImageAspectFlagBits::eColor, 1);
    }
}

vk::UniqueImageView
GraphicsCore::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags,
                              uint32_t mipLevels) {
    vk::ImageViewCreateInfo viewInfo{{}, image, vk::ImageViewType::e2D, format, {}, {aspectFlags, 0, mipLevels, 0, 1}};

    return device.get().createImageViewUnique(viewInfo);
}

void GraphicsCore::createRenderPass() {
    vk::AttachmentDescription colorAttachment{{}, swapchainImageFormat, vk::SampleCountFlagBits::e2,
                                              vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                              vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eUndefined,
                                              vk::ImageLayout::eColorAttachmentOptimal};//TODO msaa

    vk::AttachmentReference colorAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription depthAttachment{{}, findDepthFormat(), vk::SampleCountFlagBits::e2,
                                              vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                                              vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eUndefined,
                                              vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal};

    vk::AttachmentReference depthAttachmentRef = {1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::AttachmentDescription colorAttachmentResolve{{}, swapchainImageFormat, vk::SampleCountFlagBits::e1,
                                                     vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eStore,
                                                     vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eStore,
                                                     vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR};

    vk::AttachmentReference colorAttachmentResolveRef = {2, vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass = {{}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colorAttachmentRef,
                                      &colorAttachmentResolveRef, &depthAttachmentRef};

    vk::SubpassDependency dependency{VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                     vk::PipelineStageFlagBits::eColorAttachmentOutput, {},
                                     vk::AccessFlagBits::eColorAttachmentRead |
                                     vk::AccessFlagBits::eColorAttachmentWrite};

    std::vector<vk::AttachmentDescription> attachments{colorAttachment, depthAttachment, colorAttachmentResolve};
    vk::RenderPassCreateInfo renderPassInfo{{}, static_cast<uint32_t>(attachments.size()), attachments.data(), 1,
                                            &subpass, 1, &dependency};

    renderPass = device.get().createRenderPassUnique(renderPassInfo);
}


vk::Format GraphicsCore::findDepthFormat() {
    return findSupportedFormat(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format GraphicsCore::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                             const vk::FormatFeatureFlags &features) {
    for (auto format : candidates) {
        auto properties = physicalDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }

        throw std::runtime_error("failed to find supported format!");
    }
}

void GraphicsCore::createGraphicsDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding{0, vk::DescriptorType::eUniformBufferDynamic, 1,
                                                    vk::ShaderStageFlagBits::eVertex};

    vk::DescriptorSetLayoutBinding ssboLayoutBinding{2, vk::DescriptorType::eStorageBuffer, 1,
                                                     vk::ShaderStageFlagBits::eVertex};

    vk::DescriptorSetLayoutBinding ssboFloorLayoutBinding{3, vk::DescriptorType::eStorageBuffer, 1,
                                                          vk::ShaderStageFlagBits::eVertex};

    vk::DescriptorSetLayoutBinding samplerLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1,
                                                        vk::ShaderStageFlagBits::eFragment};


    std::vector<vk::DescriptorSetLayoutBinding> bindings{uboLayoutBinding, ssboLayoutBinding, ssboFloorLayoutBinding,
                                                         samplerLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {{}, static_cast<uint32_t >(bindings.size()), bindings.data()};

    graphicsDescriptorSetLayout = device.get().createDescriptorSetLayoutUnique(layoutInfo);

}

void GraphicsCore::createComputeDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding ssboLayoutBinding{0, vk::DescriptorType::eStorageBuffer, 1,
                                                     vk::ShaderStageFlagBits::eCompute};

    vk::DescriptorSetLayoutBinding ssboFloorLayoutBinding{1, vk::DescriptorType::eStorageBuffer, 1,
                                                     vk::ShaderStageFlagBits::eCompute};

    std::vector<vk::DescriptorSetLayoutBinding> bindings{ssboLayoutBinding, ssboFloorLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {{}, static_cast<uint32_t >(bindings.size()), bindings.data()};

    computeDescriptorSetLayout = device.get().createDescriptorSetLayoutUnique(layoutInfo);

}

void GraphicsCore::createComputePipeline() {
    auto compSharedCode = Utilities::readFile("shaders/comp.spv");

    auto computeShaderModule = createShaderModule(compSharedCode);

    vk::PipelineShaderStageCreateInfo computeShader{{}, vk::ShaderStageFlagBits::eCompute, computeShaderModule.get(),
                                                    "main"};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, 1, &computeDescriptorSetLayout.get()};

    computePipelineLayout = device.get().createPipelineLayoutUnique(pipelineLayoutInfo);

    vk::ComputePipelineCreateInfo createInfo{{}, computeShader, computePipelineLayout.get()};

    computePipeline = device.get().createComputePipelineUnique({}, createInfo);

}

void GraphicsCore::createGraphicsPipeline() {
    auto vertShaderCode = Utilities::readFile("shaders/particle.spv");
    auto fragShaderCode = Utilities::readFile("shaders/frag.spv");

    auto vertShaderModule = createShaderModule(vertShaderCode);
    auto fragShaderModule = createShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule.get(),
                                                          "main"};

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{{}, vk::ShaderStageFlagBits::eFragment,
                                                          fragShaderModule.get(), "main"};

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Objects::VertexData::getBindingDescription();
    auto attributeDescriptions = Objects::VertexData::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{{}, 1, &bindingDescription,
                                                           static_cast<uint32_t>(attributeDescriptions.size()),
                                                           attributeDescriptions.data()};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyTriangle{{}, vk::PrimitiveTopology::eTriangleList, VK_FALSE};
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyLine{{}, vk::PrimitiveTopology::eTriangleList, VK_FALSE};

    vk::Viewport viewport{0.0f, 0.0f, (float) swapchainExtent.width, (float) swapchainExtent.height, 0.0f, 1.0f};

    vk::Rect2D scissor{{0, 0}, swapchainExtent};

    vk::PipelineViewportStateCreateInfo viewportState{{}, 1, &viewport, 1, &scissor};

    //TODO Cullface
    vk::PipelineRasterizationStateCreateInfo rasterizerFill = {{}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill,
                                                           vk::CullModeFlagBits::eBack,
                                                           vk::FrontFace::eCounterClockwise};
    vk::PipelineRasterizationStateCreateInfo rasterizerLine = {{}, VK_FALSE, VK_FALSE, vk::PolygonMode::eLine,
                                                           vk::CullModeFlagBits::eBack,
                                                           vk::FrontFace::eCounterClockwise};
    rasterizerLine.setLineWidth(1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling{{}, vk::SampleCountFlagBits::e2, VK_FALSE}; //TODO msaa

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{}; //TODO Blend
    colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR |
                                           vk::ColorComponentFlagBits::eG |
                                           vk::ColorComponentFlagBits::eB |
                                           vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendStateCreateInfo colorBlending{{}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorBlendAttachment};


    std::vector<vk::DynamicState> dynamicStates{
            vk::DynamicState::eViewport,
            vk::DynamicState::eLineWidth
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{{}, 2, dynamicStates.data()};

    vk::PipelineDepthStencilStateCreateInfo depthStencil{{}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE,
                                                         VK_FALSE};

    vk::PushConstantRange pushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof(int)};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, 1, &graphicsDescriptorSetLayout.get(), 1, &pushConstantRange};

    graphicsPipelineLayout = device.get().createPipelineLayoutUnique(pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo pipelineInfoTriangle{{}, 2, shaderStages.data(), &vertexInputInfo, &inputAssemblyTriangle, nullptr,
                                                        &viewportState, &rasterizerFill, &multisampling, &depthStencil,
                                                        &colorBlending,
                                                        &dynamicState, graphicsPipelineLayout.get(), renderPass.get(), 0};
    vk::GraphicsPipelineCreateInfo pipelineInfoLine{{}, 2, shaderStages.data(), &vertexInputInfo, &inputAssemblyLine, nullptr,
                                                    &viewportState, &rasterizerLine, &multisampling, &depthStencil,
                                                    &colorBlending,
                                                    &dynamicState, graphicsPipelineLayout.get(), renderPass.get(), 0};

    graphicsPipelineTriangle = device.get().createGraphicsPipelineUnique({}, pipelineInfoTriangle);
    graphicsPipelineLine = device.get().createGraphicsPipelineUnique({}, pipelineInfoLine);

}

vk::UniqueShaderModule GraphicsCore::createShaderModule(const std::string &code) {
    vk::ShaderModuleCreateInfo createInfo{{}, code.size(), reinterpret_cast<const uint32_t *>(code.data())};

    return device.get().createShaderModuleUnique(createInfo);
}

void GraphicsCore::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices(physicalDevice, surface);

    vk::CommandPoolCreateInfo poolInfo{{}, queueFamilyIndices.getGraphicsFamily()};

    commandPool = device.get().createCommandPoolUnique(poolInfo);
}

void GraphicsCore::createColorResources() {
    vk::Format colorFormat = swapchainImageFormat;

    auto[image, imageMemory] = createImage(swapchainExtent.width,
                                           swapchainExtent.height,
                                           1,
                                           vk::SampleCountFlagBits::e2, //TODO msaa
                                           colorFormat,
                                           vk::ImageTiling::eOptimal,
                                           vk::ImageUsageFlagBits::eTransientAttachment |
                                           vk::ImageUsageFlagBits::eColorAttachment,
                                           vk::MemoryPropertyFlagBits::eDeviceLocal);
    colorImage.swap(image);
    colorImageMemory.swap(imageMemory);

    colorImageView = createImageView(colorImage.get(), colorFormat, vk::ImageAspectFlagBits::eColor, 1);

    transitionImageLayout(colorImage,
                          colorFormat,
                          vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eColorAttachmentOptimal,
                          1);
}

void GraphicsCore::createDepthResources() {
    vk::Format depthFormat = findDepthFormat();
    auto[image, imageMemory] = createImage(swapchainExtent.width,
                                           swapchainExtent.height,
                                           1,
                                           vk::SampleCountFlagBits::e2,
                                           depthFormat,
                                           vk::ImageTiling::eOptimal,
                                           vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                           vk::MemoryPropertyFlagBits::eDeviceLocal);
    depthImage.swap(image);
    depthImageMemory.swap(imageMemory);

    depthImageView = createImageView(depthImage.get(), depthFormat, vk::ImageAspectFlagBits::eDepth, 1);

    transitionImageLayout(depthImage,
                          depthFormat,
                          vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);

}

std::pair<vk::UniqueImage, vk::UniqueDeviceMemory>
GraphicsCore::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples,
                          vk::Format format,
                          vk::ImageTiling tiling, const vk::ImageUsageFlags &usage,
                          const vk::MemoryPropertyFlags &properties) {
    vk::ImageCreateInfo imageInfo{{}, vk::ImageType::e2D, format, {width, height, 1}, mipLevels, 1, numSamples, tiling,
                                  usage, vk::SharingMode::eExclusive, 0,
                                  nullptr, vk::ImageLayout::eUndefined};

    auto image = device.get().createImageUnique(imageInfo);


    auto memRequirements = device.get().getImageMemoryRequirements(image.get());


    vk::MemoryAllocateInfo allocInfo{memRequirements.size, findMemoryType(memRequirements.memoryTypeBits, properties)};
    auto imageMemory = device.get().allocateMemoryUnique(allocInfo);
    device.get().bindImageMemory(image.get(), imageMemory.get(), 0);

    return std::make_pair(std::move(image), std::move(imageMemory));
}

uint32_t GraphicsCore::findMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags &properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void GraphicsCore::transitionImageLayout(const vk::UniqueImage &image, vk::Format format, vk::ImageLayout oldLayout,
                                         vk::ImageLayout newLayout,
                                         uint32_t mipLevels) {
    auto commandBuffer = beginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier = {{}, vk::AccessFlagBits::eTransferWrite, oldLayout, newLayout,
                                      VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image.get(),
                                      {{}, 0, mipLevels, 0, 1}};

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);

        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    }

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (oldLayout == vk::ImageLayout::eUndefined &&
               newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.dstAccessMask =
                vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer[0].get().pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

std::vector<vk::UniqueCommandBuffer> GraphicsCore::beginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo allocInfo{commandPool.get(), vk::CommandBufferLevel::ePrimary, 1};

    auto commandBuffer = device.get().allocateCommandBuffersUnique(allocInfo);

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

    commandBuffer[0].get().begin(beginInfo);

    return std::move(commandBuffer);
}

void GraphicsCore::endSingleTimeCommands(const std::vector<vk::UniqueCommandBuffer> &commandBuffer) {
    commandBuffer[0].get().end();

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&commandBuffer[0].get());

    graphicsQueue.submit(submitInfo, {});
    graphicsQueue.waitIdle();
}

bool GraphicsCore::hasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void GraphicsCore::createFramebuffers() {
    swapchainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::vector<vk::ImageView> attachments = {
                colorImageView.get(),
                depthImageView.get(),
                swapChainImageViews[i].get()
        };

        vk::FramebufferCreateInfo framebufferInfo{{}, renderPass.get(), static_cast<uint32_t>(attachments.size()),
                                                  attachments.data(), swapchainExtent.width, swapchainExtent.height, 1};

        swapchainFramebuffers[i] = device.get().createFramebufferUnique(framebufferInfo);
    }
}

void GraphicsCore::createVertexBuffer() {
    vk::DeviceSize bufferSize = Utilities::getVertexBufferSize(objects);

    vk::UniqueBuffer stagingBuffer;
    vk::UniqueDeviceMemory stagingBufferMemory;
    auto[buffer, bufferMemory] = createBuffer(bufferSize,
                                              vk::BufferUsageFlagBits::eTransferSrc,
                                              vk::MemoryPropertyFlagBits::eHostVisible |
                                              vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBuffer.swap(buffer);
    stagingBufferMemory.swap(bufferMemory);

    uint32_t offset = 0;
    for (const auto &object : objects) {
        void *data;
        auto objectVertices = object.getVertices();
        uint32_t sizeToMap = objectVertices.size() * sizeof(Objects::VertexData);
        device.get().mapMemory(stagingBufferMemory.get(), offset, sizeToMap, {}, &data);
        memcpy(data, objectVertices.data(), (size_t) sizeToMap);
        device.get().unmapMemory(stagingBufferMemory.get());
        offset += sizeToMap;
    }

    auto[bufferV, bufferMemoryV] = createBuffer(bufferSize,
                                                vk::BufferUsageFlagBits::eTransferDst |
                                                vk::BufferUsageFlagBits::eVertexBuffer,
                                                vk::MemoryPropertyFlagBits::eDeviceLocal);
    vertexBuffer.swap(bufferV);
    vertexBufferMemory.swap(bufferMemoryV);


    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

}

std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>
GraphicsCore::createBuffer(vk::DeviceSize size, const vk::BufferUsageFlags &usage,
                           const vk::MemoryPropertyFlags &properties) {

    vk::BufferCreateInfo bufferInfo{{}, size, usage, vk::SharingMode::eExclusive};

    auto buffer = device.get().createBufferUnique(bufferInfo);

    vk::MemoryRequirements memRequirements = device.get().getBufferMemoryRequirements(buffer.get());

    vk::MemoryAllocateInfo allocInfo{memRequirements.size, findMemoryType(memRequirements.memoryTypeBits, properties)};

    auto bufferMemory = device.get().allocateMemoryUnique(allocInfo);

    device.get().bindBufferMemory(buffer.get(), bufferMemory.get(), 0);

    return std::make_pair(std::move(buffer), std::move(bufferMemory));
}

void
GraphicsCore::copyBuffer(const vk::UniqueBuffer &srcBuffer, const vk::UniqueBuffer &dstBuffer, vk::DeviceSize size) {
    auto commandBuffer = beginSingleTimeCommands();

    vk::BufferCopy copyRegion{0, 0, size};

    commandBuffer[0].get().copyBuffer(srcBuffer.get(), dstBuffer.get(), 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void GraphicsCore::createUniformBuffers() {
    vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();

    size_t minUboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;

    if (minUboAlignment > 0) {
        dynamicAlignmentMat = (dynamicAlignmentMat + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    auto bufferSize = dynamicAlignmentMat * objects.size(); //TODO
    uniformBuffers.resize(bufferSize);
    uniformBuffersMemory.resize(bufferSize);

    for (size_t i = 0; i < swapchainImages.size(); i++) {
        auto[buffer, memoryBuffer] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                                                  vk::MemoryPropertyFlagBits::eHostVisible |
                                                  vk::MemoryPropertyFlagBits::eHostCoherent);
        uniformBuffers[i].swap(buffer);
        uniformBuffersMemory[i].swap(memoryBuffer);
    }
}

void GraphicsCore::createShaderStorageBuffer() {
    const auto waterDensity = 997;
    const auto volume = 0.001;
    const auto mass = 0.02;
    std::vector<ParticleRecord> particles;
    for (int z = 0; z < 15; ++z) {
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                particles.emplace_back();
                auto &particle = particles.back();
                particle.position = glm::vec4{x * 0.02, y * 0.02, z * 0.02, 0.0f};
                particle.mass = mass;
                particle.color = glm::vec4(1.0, 1.0, 0.0, 1.0);
            }
        }
    }

    vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();

    size_t minSSBOAlignment = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;

    if (minSSBOAlignment > 0) {
        dynamicAlignmentParticle = (dynamicAlignmentParticle + minSSBOAlignment - 1) & ~(minSSBOAlignment - 1);
    }

    auto bufferSize = dynamicAlignmentParticle * 5000; //TODO Particle Count
    vk::UniqueBuffer stagingBuffer;
    vk::UniqueDeviceMemory stagingBufferMemory;
    auto[buffer, bufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                              vk::MemoryPropertyFlagBits::eHostVisible |
                                              vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBuffer.swap(buffer);
    stagingBufferMemory.swap(bufferMemory);

    void *data;
    uint32_t sizeToMap = 5000 * dynamicAlignmentParticle;
    device.get().mapMemory(stagingBufferMemory.get(), 0, sizeToMap, {}, &data);
    memcpy(data, particles.data(), (size_t) sizeToMap);
    device.get().unmapMemory(stagingBufferMemory.get());


    auto[bufferS, bufferMemoryS] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst |
                                                            vk::BufferUsageFlagBits::eTransferSrc |
                                                            vk::BufferUsageFlagBits::eStorageBuffer,
                                                vk::MemoryPropertyFlagBits::eDeviceLocal);
    storageBuffer.swap(bufferS);
    storageBufferMemory.swap(bufferMemoryS);

    copyBuffer(stagingBuffer, storageBuffer, bufferSize);


    std::vector<ParticleRecord> floor;
    for (int x = -26; x < 26; x += 2) {
        for (int z = -26; z < 26; z += 2) {
            for (int y = -10; y > -13; --y) {
                floor.emplace_back();
                auto &floorParticle = floor.back();
                floorParticle.mass = mass;
                floorParticle.position = glm::vec4{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z),
                                                   0.0f};
                floorParticle.color = glm::vec4{0.5f};
                //floorParticle.density = waterDensity / 2028.0;
                //floorParticle.pressure = 248 / 2028.0;
            }
        }
    }

    bufferSize = dynamicAlignmentParticle * 2028; //TODO Particle Count
    vk::UniqueBuffer stagingBufferFloor;
    vk::UniqueDeviceMemory stagingBufferMemoryFloor;
    auto[bufferFloor, bufferMemoryFloor] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                        vk::MemoryPropertyFlagBits::eHostVisible |
                                                        vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBufferFloor.swap(bufferFloor);
    stagingBufferMemoryFloor.swap(bufferMemoryFloor);

    sizeToMap = 2028 * dynamicAlignmentParticle;
    device.get().mapMemory(stagingBufferMemoryFloor.get(), 0, sizeToMap, {}, &data);
    memcpy(data, floor.data(), (size_t) sizeToMap);
    device.get().unmapMemory(stagingBufferMemoryFloor.get());


    auto[bufferSFloor, bufferMemorySFloor] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst |
                                                                      vk::BufferUsageFlagBits::eTransferSrc |
                                                                      vk::BufferUsageFlagBits::eStorageBuffer,
                                                          vk::MemoryPropertyFlagBits::eDeviceLocal);
    storageBufferFloor.swap(bufferSFloor);
    storageBufferMemoryFloor.swap(bufferMemorySFloor);

    copyBuffer(stagingBufferFloor, storageBufferFloor, bufferSize);

}

void GraphicsCore::createIndexBuffer() {
    vk::DeviceSize bufferSize = Utilities::getIndexBufferSize(objects);

    vk::UniqueBuffer stagingBuffer;
    vk::UniqueDeviceMemory stagingBufferMemory;
    auto[buffer, bufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                              vk::MemoryPropertyFlagBits::eHostVisible |
                                              vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBuffer.swap(buffer);
    stagingBufferMemory.swap(bufferMemory);

    uint32_t offset = 0;
    for (const auto &object : objects) {
        void *data;
        auto objectIndices = object.getIndices();
        uint32_t sizeToMap = objectIndices.size() * sizeof(uint32_t);
        device.get().mapMemory(stagingBufferMemory.get(), offset, sizeToMap, {}, &data);
        memcpy(data, objectIndices.data(), (size_t) sizeToMap);
        device.get().unmapMemory(stagingBufferMemory.get());
        offset += sizeToMap;
    }
    auto[bufferI, bufferMemoryI] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst |
                                                            vk::BufferUsageFlagBits::eIndexBuffer,
                                                vk::MemoryPropertyFlagBits::eDeviceLocal);
    indexBuffer.swap(bufferI);
    indexBufferMemory.swap(bufferMemoryI);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);
}

void GraphicsCore::createGraphicsDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> poolSizes{{vk::DescriptorType::eUniformBufferDynamic, static_cast<uint32_t>(swapchainImages.size())},
                                                  {vk::DescriptorType::eStorageBuffer,        static_cast<uint32_t>(swapchainImages.size())},
                                                  {vk::DescriptorType::eStorageBuffer,        static_cast<uint32_t>(swapchainImages.size())},
                                                  {vk::DescriptorType::eCombinedImageSampler, static_cast<uint32_t>(swapchainImages.size())}};

    vk::DescriptorPoolCreateInfo poolInfo{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                          static_cast<uint32_t>(swapchainImages.size()),
                                          static_cast<uint32_t>(poolSizes.size()), poolSizes.data()};

    graphicsDescriptorPool = device.get().createDescriptorPoolUnique(poolInfo);
}

void GraphicsCore::createComputeDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> poolSizes{{vk::DescriptorType::eStorageBuffer, static_cast<uint32_t>(swapchainImages.size())},
                                                  {vk::DescriptorType::eStorageBuffer, static_cast<uint32_t>(swapchainImages.size())}};

    vk::DescriptorPoolCreateInfo poolInfo{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                          static_cast<uint32_t>(swapchainImages.size()),
                                          static_cast<uint32_t>(poolSizes.size()), poolSizes.data()};

    computeDescriptorPool = device.get().createDescriptorPoolUnique(poolInfo);
}

void GraphicsCore::createGraphicsDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(swapchainImages.size(), graphicsDescriptorSetLayout.get());
    vk::DescriptorSetAllocateInfo allocInfo{graphicsDescriptorPool.get(), static_cast<uint32_t>(swapchainImages.size()),
                                            layouts.data()};

    graphicsDescriptorSets.resize(swapchainImages.size());
    graphicsDescriptorSets = device.get().allocateDescriptorSetsUnique(allocInfo);


    for (size_t i = 0; i < swapchainImages.size(); i++) {
        vk::DescriptorBufferInfo bufferInfo{uniformBuffers[i].get(), 0, dynamicAlignmentMat};
        vk::DescriptorBufferInfo ssboInfo{storageBuffer.get(), 0, dynamicAlignmentParticle * 5000};
        vk::DescriptorBufferInfo ssboFloorInfo{storageBufferFloor.get(), 0, dynamicAlignmentParticle * 2028};
        /* TODO Textures
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;
         */
        std::vector<vk::WriteDescriptorSet> descriptorWrites{{graphicsDescriptorSets[i].get(), 0, 0, 1, vk::DescriptorType::eUniformBufferDynamic, nullptr, &bufferInfo},
                                                             {graphicsDescriptorSets[i].get(), 2, 0, 1, vk::DescriptorType::eStorageBuffer,        nullptr, &ssboInfo},
                                                             {graphicsDescriptorSets[i].get(), 3, 0, 1, vk::DescriptorType::eStorageBuffer,        nullptr, &ssboFloorInfo}};

        //{graphicsDescriptorSets[i].get(), 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageinfo} //TODO textures
        /*
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = graphicsDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;*/

        device.get().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                          nullptr);
    }
}

void GraphicsCore::createComputeDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(swapchainImages.size(), computeDescriptorSetLayout.get());
    vk::DescriptorSetAllocateInfo allocInfo{computeDescriptorPool.get(), static_cast<uint32_t>(swapchainImages.size()),
                                            layouts.data()};

    computeDescriptorSets.resize(swapchainImages.size());
    computeDescriptorSets = device.get().allocateDescriptorSetsUnique(allocInfo);


    for (size_t i = 0; i < swapchainImages.size(); i++) {
        vk::DescriptorBufferInfo ssboInfo{storageBuffer.get(), 0, dynamicAlignmentParticle * 5000};
        vk::DescriptorBufferInfo ssboFloorInfo{storageBufferFloor.get(), 0, dynamicAlignmentParticle * 2028};
        std::vector<vk::WriteDescriptorSet> descriptorWrites{{computeDescriptorSets[i].get(), 0, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &ssboInfo},
                                                             {computeDescriptorSets[i].get(), 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &ssboFloorInfo}};
        device.get().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                          nullptr);
    }
}

void GraphicsCore::createCommandBuffers() {
    std::vector<vk::ClearValue> clearValues(2);
    clearValues[0].setColor({std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f}});
    clearValues[1].setDepthStencil({1.0f, 0});

    vk::CommandBufferAllocateInfo allocInfo{commandPool.get(), vk::CommandBufferLevel::ePrimary,
                                            static_cast<uint32_t>(swapchainFramebuffers.size())};

    commandBuffers = device.get().allocateCommandBuffersUnique(allocInfo);

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        vk::CommandBufferBeginInfo beginInfo{};

        commandBuffers[i].get().begin(beginInfo);

        auto[width, height] = window.getFramebufferSize();
        vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
        commandBuffers[i].get().setViewport(0, 1, &viewport);

        commandBuffers[i].get().bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline.get());
        commandBuffers[i].get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, computePipelineLayout.get(), 0, 1,
                                                   &computeDescriptorSets[i].get(), 0,
                                                   nullptr);
        commandBuffers[i].get().dispatch(5000, 1, 1);

        vk::RenderPassBeginInfo renderPassInfo{renderPass.get(), swapchainFramebuffers[i].get(),
                                               {{0, 0}, swapchainExtent},
                                               static_cast<uint32_t>(clearValues.size()), clearValues.data()};

        commandBuffers[i].get().beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        commandBuffers[i].get().bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipelineTriangle.get());
        vk::Buffer vertexBuffers[] = {vertexBuffer.get()};
        vk::DeviceSize offsets[] = {0};

        commandBuffers[i].get().bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffers[i].get().bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);


        auto vertexOffset = 0;
        auto index = 0;
        for (const auto &object : objects) {
            uint32_t dynamicOffset = index * static_cast<uint32_t>(dynamicAlignmentMat);
            int isParticle = 0;
            int instanceCount = 1;
            if (index == 0) {
                isParticle = 1;
                instanceCount = 5000;
            }
/*            else if(index == 1){
                commandBuffers[i].get().bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipelineLine.get());
                commandBuffers[i].get().setLineWidth(2.0);
            }*/
            commandBuffers[i].get().pushConstants(graphicsPipelineLayout.get(), vk::ShaderStageFlagBits::eVertex, 0,
                                                  sizeof(int), &isParticle);
            commandBuffers[i].get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout.get(),
                                                       0, 1,
                                                       &graphicsDescriptorSets[i].get(), 1, &dynamicOffset);
            commandBuffers[i].get().drawIndexed(object.getIndices().size(), instanceCount, 0, vertexOffset, 0);
            /*if(instanceCount == 5000){
                isParticle = 2;
                commandBuffers[i].get().pushConstants(graphicsPipelineLayout.get(), vk::ShaderStageFlagBits::eVertex, 0,
                                                      sizeof(int), &isParticle);
                commandBuffers[i].get().drawIndexed(object.getIndices().size(), 2028, 0, vertexOffset, 0);
            }*/
            vertexOffset += object.getVertices().size();
            index++;
        }
        commandBuffers[i].get().endRenderPass();
        commandBuffers[i].get().end();
    }
}

void GraphicsCore::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};

    vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageAvailableSemaphores[i] = device.get().createSemaphoreUnique(semaphoreInfo);
        renderFinishedSemaphores[i] = device.get().createSemaphoreUnique(semaphoreInfo);
        inFlightFences[i] = device.get().createFenceUnique(fenceInfo);
    }
}

bool GraphicsCore::windowShouldClose() {
    return window.windowShouldClose();
}

void GraphicsCore::waitIdle() {
    device.get().waitIdle();
}

