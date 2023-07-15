#include <numbers>
#include <iostream>
#include "camera.h"

inline float degrees_to_radians(float degrees) {
        return degrees * std::numbers::pi_v<float> / 180.0f;
}

Camera::Camera(glm::vec3 origin, glm::vec3 look_direction, glm::vec3 up_direction, float vertical_fov, float aspect_ratio) : m_origin(origin) {
        
        float theta = degrees_to_radians(vertical_fov);
        float h = tanf(theta / 2.0f);
        float viewport_height = 2.0f * h;
        float viewport_width = aspect_ratio * viewport_height;
        
        auto camera_z = glm::normalize(origin - look_direction);
        auto camera_x = glm::normalize(cross(up_direction, camera_z));
        auto camera_y = cross(camera_z, camera_x);
        
        m_horizontal = viewport_width * camera_x;
        m_vertical = viewport_height * camera_y;
        m_lower_left_corner = origin - m_horizontal / 2.0f - m_vertical / 2.0f - camera_z;
}

void Camera::setOrigin(glm::vec3 new_position) noexcept {
        m_origin = new_position;
}
void Camera::translate(glm::vec3 offset) noexcept {
        m_origin += offset;
}

Ray Camera::get_ray(glm::vec2 uv) {
        return Ray(m_origin, {glm::normalize(m_lower_left_corner + uv.x * m_horizontal + uv.y * m_vertical - m_origin)});
}
