//
// Created by Igor Frank on 28.11.19.
//

#ifndef VULKANTEST_PARTICLERECORD_H
#define VULKANTEST_PARTICLERECORD_H

#include <glm/glm.hpp>

struct ParticleRecord {
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 velocity{0.0f};
    float mass = 0;
    float density = 0;
    float pressure = 0;
    float particleRadius = 1.0;
};


#endif //VULKANTEST_PARTICLERECORD_H
