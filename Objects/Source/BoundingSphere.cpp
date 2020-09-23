//
// Created by Igor Frank on 08.10.19.
//

#include <BoundingSphere.h>

glm::vec3 BoundingSphere::getCenter() const {
    return center;
}

void BoundingSphere::setCenter(glm::vec3 center) {
    BoundingSphere::center = center;
}

double BoundingSphere::getRadius() const {
    return radius;
}

void BoundingSphere::setRadius(double radius) {
    BoundingSphere::radius = radius;
}

double BoundingSphere::getRadiusSquare() const {
    return radius * radius;
}
