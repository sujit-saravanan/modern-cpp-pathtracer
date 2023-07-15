#pragma once
#include "ray.h"

class Camera {
public:  // Public Constructors/Destructors/Overloads
        Camera(glm::vec3 origin, glm::vec3 look_direction, glm::vec3 up_direction, float vertical_fov, float aspect_ratio);
public:  // Public Member Functions
        void setOrigin(glm::vec3 new_position) noexcept;
        void translate(glm::vec3 offset) noexcept;
        
        Ray get_ray(glm::vec2 uv);
public:  // Public Member Variables
private: // Private Member Functions
private: // Private Member Variables
        glm::vec3 m_lower_left_corner{};
        glm::vec3 m_origin{};
        glm::vec3 m_horizontal{};
        glm::vec3 m_vertical{};
};