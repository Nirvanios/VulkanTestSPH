//
// Created by Igor Frank on 07.10.19.
//


#include <Object.h>
#include <string>
#include <glm/gtx/component_wise.hpp>


const std::string &Object::getName() const {
    return name;
}

void Object::setName(const std::string &name) {
    Object::name = name;
}

const std::vector<Objects::VertexData> &Object::getVertices() const {
    return vertices;
}

void Object::appendVertex(const Objects::VertexData vertex) {
    this->vertices.emplace_back();
    vertices.back() = vertex;
}

const std::vector<uint32_t> &Object::getIndices() const {
    return indices;
}

void Object::appendIndex(const uint32_t &index) {
    indices.push_back(index);
}

void Object::addNormalAtIndex(const uint32_t &index, const glm::vec3 &normal) {
    vertices[index].normal += normal;
}

void Object::computeBoundigSphere() {
    glm::vec3 min, max, center;
    glm::ivec3 minIndex, maxIndex;
    float radius;

    min = max = vertices[0].pos;
    minIndex = maxIndex = glm::ivec3(0);

    auto currentIndex = 0;
    for (auto vertex : vertices) {
        if (vertex.pos.x > max.x) {
            maxIndex.x = currentIndex;
            max.x = vertex.pos.x;
        } else if (vertex.pos.x < min.x) {
            minIndex.x = currentIndex;
            min.x = vertex.pos.x;
        }
        if (vertex.pos.y > max.y) {
            maxIndex.y = currentIndex;
            max.y = vertex.pos.y;
        } else if (vertex.pos.y < min.y) {
            minIndex.y = currentIndex;
            min.y = vertex.pos.y;
        }
        if (vertex.pos.z > max.z) {
            maxIndex.z = currentIndex;
            max.z = vertex.pos.z;
        } else if (vertex.pos.z < min.z) {
            minIndex.z = currentIndex;
            min.z = vertex.pos.z;
        }
        ++currentIndex;
    }
    auto diff = glm::abs(max - min);
    auto aprox = glm::compMax(diff);
    if (aprox == diff.x) { //TODO epsilon
        radius = glm::distance(vertices[maxIndex.x].pos, vertices[minIndex.x].pos) / 2;
        center = vertices[minIndex.x].pos + (glm::abs(vertices[maxIndex.x].pos - vertices[minIndex.x].pos) / 2.0f);
    } else if (aprox == diff.y) { //TODO epsilon
        radius = glm::distance(vertices[maxIndex.y].pos, vertices[minIndex.y].pos) / 2;
        center = vertices[minIndex.y].pos + (glm::abs(vertices[maxIndex.y].pos - vertices[minIndex.y].pos) / 2.0f);
    } else if (aprox == diff.z) { //TODO epsilon
        radius = glm::distance(vertices[maxIndex.z].pos, vertices[minIndex.z].pos) / 2;
        auto a = glm::abs(vertices[maxIndex.z].pos - vertices[minIndex.z].pos);
        center = vertices[minIndex.z].pos + (glm::abs(vertices[maxIndex.z].pos - vertices[minIndex.z].pos) / 2.0f);
    } else {
        return; //TODO
    }

    for (auto vertex : vertices) {
        auto distance = glm::distance(vertex.pos, center);
        if (distance > radius) {
            radius = (radius + distance) / 2.0f;
            center += ((distance - radius) / distance) * (vertex.pos - center);
        }
    }
    boundingSphere.setCenter(center);
    boundingSphere.setRadius(radius);
}


const BoundingSphere &Object::getBoundingSphere() const {
    return boundingSphere;
}

void Object::setBoundingSphere(const BoundingSphere &boundingSphere){
     this->boundingSphere = boundingSphere;
}

const glm::mat4 &Object::getModelMatrix() const {
    return modelMatrix;
}

void Object::setModelMatrix(const glm::mat4 &modelMatrix) {
    Object::modelMatrix = modelMatrix;
}

void Object::appendBS(){
    auto bsCenter = boundingSphere.getCenter();
    auto bsRadius = boundingSphere.getRadius();

    Objects::VertexData vertex;

    auto index = vertices.size();

    vertex.pos = bsCenter;
    vertex.color = glm::vec3(1.0f, 0.0f, 0.0f);

    vertex.pos.x += bsRadius;
    vertices.push_back(vertex);
    vertex.pos = bsCenter;
    vertex.pos.x -= bsRadius;
    vertices.push_back(vertex);
    vertex.pos = bsCenter;
    vertex.pos.y += bsRadius;
    vertices.push_back(vertex);
    vertex.pos = bsCenter;
    vertex.pos.y -= bsRadius;
    vertices.push_back(vertex);
    vertex.pos = bsCenter;
    vertex.pos.z += bsRadius;
    vertices.push_back(vertex);
    vertex.pos = bsCenter;
    vertex.pos.z -= bsRadius;
    vertices.push_back(vertex);

    indices.push_back(index);
    indices.push_back(index+2);
    indices.push_back(index+3);

    indices.push_back(index+1);
    indices.push_back(index+2);
    indices.push_back(index+3);

    indices.push_back(index+2);
    indices.push_back(index+3);
    indices.push_back(index+4);

    indices.push_back(index+2);
    indices.push_back(index+3);
    indices.push_back(index+5);


}
