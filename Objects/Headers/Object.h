//
// Created by Igor Frank on 07.10.19.
//

#ifndef UNTITLED_OBJECT_H
#define UNTITLED_OBJECT_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <VertexData.h>
#include <BoundingSphere.h>

class Object {
public:
    [[nodiscard]] const std::string &getName() const;

    void setName(const std::string &name);

    [[nodiscard]] const std::vector<Objects::VertexData> &getVertices() const;

    void appendVertex(Objects::VertexData vertex);

    [[nodiscard]] const std::vector<uint32_t> &getIndices() const;

    void appendIndex(const uint32_t &index);

    void addNormalAtIndex(const uint32_t &index, const glm::vec3 &normal);

    void computeBoundigSphere();

private:
    std::string name = "Undefined";
    std::vector<Objects::VertexData> vertices;
    std::vector<uint32_t> indices;
    BoundingSphere boundingSphere;
    glm::mat4 modelMatrix = glm::mat4(1.0f);
public:
    [[nodiscard]] const glm::mat4 &getModelMatrix() const;

    void setModelMatrix(const glm::mat4 &modelMatrix);

    [[nodiscard]] const BoundingSphere &getBoundingSphere() const;

    void setBoundingSphere(const BoundingSphere &boundingSphere);

    void appendBS();

};

#endif //UNTITLED_OBJECT_H
