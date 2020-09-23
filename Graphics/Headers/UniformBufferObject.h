//
// Created by Igor Frank on 28.10.19.
//

#ifndef VULKANTEST_UNIFORMBUFFEROBJECT_H
#define VULKANTEST_UNIFORMBUFFEROBJECT_H

#include <glm/glm.hpp>

class UniformBufferObject {
public:
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 dummy = glm::mat4(1.0f);

};


#endif //VULKANTEST_UNIFORMBUFFEROBJECT_H
