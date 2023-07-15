#pragma once
#include <glm/glm.hpp>

class Ray {
public:  // Public Constructors/Destructors/Overloads
public:  // Public Member Functions
        [[nodiscard]] glm::vec3 at(float t) const noexcept { return origin + t * direction; }
//        [[nodiscard]] glm::vec3 at(float t) noexcept { return origin + t * direction; }
public:  // Public Member Variables
        glm::vec3 origin;
        glm::vec3 direction;
private: // Private Member Functions
private: // Private Member Variables
};