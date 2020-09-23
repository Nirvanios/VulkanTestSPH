//
// Created by Igor Frank on 07.10.19.
//

#ifndef UNTITLED_VERTEX_H
#define UNTITLED_VERTEX_H


#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.hpp>
#include <array>

namespace Objects {
    struct VertexData {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
        glm::vec3 normal{0.0f, 0.0f, 0.0f};

        static vk::VertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(VertexData);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
            std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{
                    {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, pos)},
                    {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, color)},
                    {2, 0, vk::Format::eR32G32Sfloat, offsetof(VertexData, texCoord)},
                    {3, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, normal)}
            };
            return attributeDescriptions;
        }

        bool operator==(const VertexData &other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };
}

namespace std {
    template<>
    struct hash<Objects::VertexData> {
        size_t operator()(Objects::VertexData const &vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                     (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
#endif //UNTITLED_VERTEX_H
