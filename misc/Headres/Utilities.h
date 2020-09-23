//
// Created by Igor Frank on 07.10.19.
//

#ifndef VULKANTEST_UTILITIES_H
#define VULKANTEST_UTILITIES_H


#include <stdint-gcc.h>
#include <vector>
#include <Object.h>
#include <vulkan/vulkan.hpp>

class Utilities {
public:
    static uint32_t  getVertexBufferSize(const std::vector<Object> &objects);

    static uint32_t  getIndexBufferSize(const std::vector<Object> &objects);

    static std::vector<Object> loadModel(const std::string &modelPath);

    static std::tuple<int, float>
    pickObject(const std::vector<Object> &objects, const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
               const glm::vec2 &normalizedViewportCoordinates);

    static std::string readFile(const std::string &filename);

    template<typename T, typename Container=std::vector<T>>
    static bool isIn(T value, Container &&container) {
        return std::any_of(container.begin(), container.end(), [value](const auto &a) { return value == a; });
    }


};


#endif //VULKANTEST_UTILITIES_H
