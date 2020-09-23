//
// Created by Igor Frank on 08.10.19.
//

#ifndef VULKANTEST_BOUNDINGSPHERE_H
#define VULKANTEST_BOUNDINGSPHERE_H


#include <glm/glm.hpp>

class BoundingSphere {
private:
    double radius;
    glm::vec3 center;

public:
    BoundingSphere(double center, double radius) : center(center), radius(radius) {};
    BoundingSphere() = default;;

    [[nodiscard]] glm::vec3 getCenter() const;

    void setCenter(glm::vec3 center);

    [[nodiscard]] double getRadius() const;

    [[nodiscard]] double getRadiusSquare() const;

    void setRadius(double radius);
};


#endif //VULKANTEST_BOUNDINGSPHERE_H
