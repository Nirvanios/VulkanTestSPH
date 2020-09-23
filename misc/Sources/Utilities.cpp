//
// Created by Igor Frank on 07.10.19.
//

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>


#include <iostream>
#include <tiny_obj_loader.h>
#include "../Headres/Utilities.h"
#include <ctime>
#include <random>

uint32_t Utilities::getVertexBufferSize(const std::vector<Object> &objects) {
    uint32_t count = 0;
    for(const auto& object : objects){
        count += object.getVertices().size();
    }
    return count * sizeof(objects[0].getVertices()[0]);
}

uint32_t Utilities::getIndexBufferSize(const std::vector<Object> &objects) {
    uint32_t count = 0;
    for(const auto& object : objects){
        count += object.getIndices().size();
    }
    return count * sizeof(objects[0].getIndices()[0]);
}

std::vector<Object> Utilities::loadModel(const std::string &modelPath) {
    std::cout << "Loading model...";
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }
    std::vector<Object> loadedObjects;

    for (const auto &shape : shapes) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution distribution(0.0, 1.0);
        auto color = glm::vec3{0.5f};
        std::cout << "\t" << shape.name << std::endl;
        std::unordered_map<Objects::VertexData, uint32_t> uniqueVertices{};
        //auto indicesOffset = indices.size();
        loadedObjects.emplace_back();
        auto &currentObject = loadedObjects.back();
        currentObject.setName(shape.name);
        for (const auto &index : shape.mesh.indices) {
            //auto color = glm::vec3(distribution(gen), distribution(gen), distribution(gen));
            Objects::VertexData vertex{};

            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

/*            vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
*/
            vertex.color = color;

            if (uniqueVertices.count(vertex) == 0) { //TODO FIX
                vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                };
                const auto& currentObjectVertices = currentObject.getVertices();
                uniqueVertices[vertex] = static_cast<uint32_t>(currentObjectVertices.size());
                currentObject.appendVertex(vertex);

            } else {
                vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                };
                currentObject.addNormalAtIndex(uniqueVertices[vertex], vertex.normal);
            }

            currentObject.appendIndex(uniqueVertices[vertex]);
        }
        currentObject.computeBoundigSphere();
    }
    std::cout << "DONE" << std::endl;
    return loadedObjects;
}

std::tuple<int, float> Utilities::pickObject(const std::vector<Object> &objects, const glm::mat4 &projectionMatrix,
                          const glm::mat4 &viewMatrix,
                          const glm::vec2 &normalizedViewportCoordinates) {

    auto tmpProj = projectionMatrix;
    tmpProj[1][1] *= -1;

    auto rayClip = glm::vec4(normalizedViewportCoordinates, -1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(tmpProj) * rayClip;
    rayEye = glm::vec4(rayEye.xy(), -1.0f, 0.0f);
    glm::vec3 rayWorld = (glm::inverse(viewMatrix) * rayEye).xyz();
    rayWorld = glm::normalize(rayWorld);

    auto currIndex = 0;
    auto selectedIndex = -1;
    float distance = std::numeric_limits<float>::max();
    auto selectedDistance = distance;
    for(auto object : objects){
        auto boundingSphere = object.getBoundingSphere();
        if(glm::intersectRaySphere(glm::vec3(5.0f, 5.0f, 5.0f), rayWorld, boundingSphere.getCenter(), static_cast<float>(boundingSphere.getRadiusSquare()), distance)){
            if(distance < selectedDistance) {
                selectedIndex = currIndex;
                selectedDistance = distance;
            }
        }
        ++currIndex;
    }
    return {selectedIndex, selectedDistance};
}

std::string Utilities::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::string buffer(fileSize, ' ');

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
