//
// Created by Igor Frank on 24.10.19.
//

#ifndef VULKANTEST_GRAPHICSCORE_H
#define VULKANTEST_GRAPHICSCORE_H


#include <memory>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include <Object.h>
#include <Camera.h>
#include <ParticleRecord.h>
#include "Window.h"


class GraphicsCore {
public:
    GraphicsCore(int width, int height, Window& window, Camera& camera, glm::vec3 particleDimensions = glm::vec3{4.0f});

    void setFramebufferResized(bool framebufferResized);

    void drawFrame();

    bool windowShouldClose();

    void waitIdle();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

private:
    glm::vec3 particleDimensions;
    const uint32_t MAX_FRAMES_IN_FLIGHT = 1;

    size_t currentFrame = 0;

    std::vector<Object> objects;
    glm::mat4 projectionMatrix;

    Window window;
    Camera* camera;
    vk::UniqueInstance instance;
    vk::DispatchLoaderDynamic loaderDynamic;
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> debugMessenger;
    vk::UniqueSurfaceKHR surface;
    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::Queue computeQueue;
    vk::UniqueSwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;
    vk::Format swapchainImageFormat;
    vk::Extent2D swapchainExtent;
    std::vector<vk::UniqueImageView> swapChainImageViews;
    vk::UniqueDescriptorSetLayout graphicsDescriptorSetLayout;
    vk::UniqueDescriptorSetLayout computeDescriptorSetLayout;
    vk::UniqueRenderPass renderPass;
    vk::UniquePipelineLayout graphicsPipelineLayout;
    vk::UniquePipelineLayout computePipelineLayout;
    vk::UniquePipeline graphicsPipelineTriangle;
    vk::UniquePipeline graphicsPipelineLine;
    vk::UniquePipeline computePipeline;
    vk::UniqueCommandPool commandPool;
    vk::UniqueImage colorImage;
    vk::UniqueDeviceMemory colorImageMemory;
    vk::UniqueImageView colorImageView;
    vk::UniqueImage depthImage;
    vk::UniqueDeviceMemory depthImageMemory;
    vk::UniqueImageView depthImageView;
    std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;
    vk::UniqueBuffer vertexBuffer;
    vk::UniqueDeviceMemory vertexBufferMemory;
    vk::UniqueBuffer indexBuffer;
    vk::UniqueDeviceMemory indexBufferMemory;
    std::vector<vk::UniqueBuffer> uniformBuffers;
    std::vector<vk::UniqueDeviceMemory> uniformBuffersMemory;
    vk::UniqueDescriptorPool graphicsDescriptorPool;
    vk::UniqueDescriptorPool computeDescriptorPool;
    vk::UniqueBuffer storageBuffer;
    vk::UniqueDeviceMemory storageBufferMemory;
    vk::UniqueBuffer storageBufferFloor;
    vk::UniqueDeviceMemory storageBufferMemoryFloor;
    std::vector<vk::UniqueDescriptorSet> graphicsDescriptorSets;
    std::vector<vk::UniqueDescriptorSet> computeDescriptorSets;
    std::vector<vk::UniqueCommandBuffer> commandBuffers;

    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
    std::vector<vk::UniqueFence> inFlightFences;

    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e2;

    size_t dynamicAlignmentMat = sizeof(glm::mat4);
    size_t dynamicAlignmentParticle = sizeof(ParticleRecord);

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_LUNARG_assistant_layer"
    };

    const std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,

    };

    void initVulkan();

    void createInstance();

    bool checkValidationLayerSupport();

    std::vector<const char *> getRequiredExtensions();

    vk::DebugUtilsMessengerCreateInfoEXT generateDebugMessengerCreateInfo();

    void setupDebugMessenger();

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
            vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            vk::DebugUtilsMessageTypeFlagBitsEXT messageType,
            const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);

    void pickPhysicalDevice();

    bool isDeviceSuitable(const vk::PhysicalDevice &device);

    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

    void createLogicalDevice();

    void createSwapChain();

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

    void createImageViews();

    vk::UniqueImageView
    createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags, uint32_t mipLevels);

    void createRenderPass();

    vk::Format findDepthFormat();

    vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                   const vk::FormatFeatureFlags &features);

    void createGraphicsDescriptorSetLayout();

    void createComputeDescriptorSetLayout();

    void createGraphicsPipeline();

    void createComputePipeline();

    vk::UniqueShaderModule createShaderModule(const std::string &code);

    void createCommandPool();

    void createColorResources();

    void createDepthResources();

    std::pair<vk::UniqueImage, vk::UniqueDeviceMemory>
    createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples,
                vk::Format format,
                vk::ImageTiling tiling, const vk::ImageUsageFlags &usage, const vk::MemoryPropertyFlags &properties);

    uint32_t findMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags &properties);

    void transitionImageLayout(const vk::UniqueImage &image, vk::Format format, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               uint32_t mipLevels);

    std::vector<vk::UniqueCommandBuffer> beginSingleTimeCommands();

    void endSingleTimeCommands(const std::vector<vk::UniqueCommandBuffer>& commandBuffer);

    bool hasStencilComponent(vk::Format format);

    void createFramebuffers();

    void createVertexBuffer();

    std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>
    createBuffer(vk::DeviceSize size, const vk::BufferUsageFlags &usage, const vk::MemoryPropertyFlags &properties);

    void copyBuffer(const vk::UniqueBuffer &srcBuffer, const vk::UniqueBuffer &dstBuffer, vk::DeviceSize size);

    void createUniformBuffers();

    void createShaderStorageBuffer();

    void createIndexBuffer();

    void createGraphicsDescriptorPool();

    void createComputeDescriptorPool();

    void createGraphicsDescriptorSets();

    void createComputeDescriptorSets();

    void createCommandBuffers();

    void createSyncObjects();

    void recreateSwapChain();

    void updateUniformBuffer(uint32_t currentImage);

    bool framebufferResized;
};


#endif //VULKANTEST_GRAPHICSCORE_H
