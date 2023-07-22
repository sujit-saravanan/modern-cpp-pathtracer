#include "shape.h"
#include <glm/gtx/norm.hpp>
#include "raytracer_random.h"

static constexpr float miss_value = std::numeric_limits<float>::max();
static constexpr float epsilon = std::numeric_limits<float>::epsilon();

Triangle::Triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) : m_p1(p1), m_p2(p2), m_p3(p3) {
#ifndef SOA
        glm::vec3 edge1 = m_p2 - m_p1;
        glm::vec3 edge2 = m_p3 - m_p1;
        m_normal = glm::normalize(glm::cross(edge1, edge2));
#endif
}
float Triangle::intersect_impl(const Ray &ray) const noexcept {
        glm::vec3 edge1 = m_p2 - m_p1;
        glm::vec3 edge2 = m_p3 - m_p1;
        glm::vec3 h = glm::cross(ray.direction, edge2);
        float a = glm::dot(edge1, h);
        
        if (a > -epsilon && a < epsilon)
                return miss_value; // Ray is parallel to the triangle.
        
        float f = 1.0f / a;
        glm::vec3 s = ray.origin - m_p1;
        float u = f * glm::dot(s, h);
        if (u < epsilon || u > 1.0f)
                return miss_value; // Intersection is outside the triangle.
        
        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(ray.direction, q);
        if (v < epsilon || u + v > 1.0f)
                return miss_value; // Intersection is outside the triangle.
        
        float t = f * glm::dot(edge2, q);
        if (t > epsilon)
                return t; // Intersection exists and is within the triangle.
        
        return miss_value; // Intersection is behind the ray origin.
}
glm::vec3 Triangle::normal_impl(const Ray &ray, float distance) const noexcept {
#ifndef SOA
        return glm::dot(ray.direction, m_normal) > epsilon ? -m_normal : m_normal;
#else
        return {};
#endif
}
glm::vec3 Triangle::position_impl() const noexcept {
        return (m_p1 + m_p2 + m_p3) / 3.0f;
}
glm::vec3 Triangle::random_point_impl(uint32_t seed, glm::vec3) const noexcept {
        float rand1 = random_pcg(seed, 0, 1);
        float rand2 = random_pcg(seed, 0, 1);
        
        if (rand1 + rand2 > 1.0f) {
                rand1 = 1.0f - rand1;
                rand2 = 1.0f - rand2;
        }
        
        float rand3 = 1.0f - rand1 - rand2;
        
        return rand1 * m_p1 + rand2 * m_p2 + rand3 * m_p3;
}
#ifdef SOA
glm::vec3 Triangle::calculate_normal() const noexcept {
        glm::vec3 edge1 = m_p2 - m_p1;
        glm::vec3 edge2 = m_p3 - m_p1;
        return glm::normalize(glm::cross(edge1, edge2));
}
#endif


Circle::Circle(glm::vec3 center, float radius) : m_center(center), m_radius(radius) {

}
float Circle::intersect_impl(const Ray &ray) const noexcept {
        glm::vec3 oc = ray.origin - m_center;
        auto a = glm::length2(ray.direction);
        auto half_b = dot(oc, ray.direction);
        auto c = glm::length2(oc) - m_radius * m_radius;
        auto discriminant = half_b * half_b - a * c;
        
        if (discriminant < 0)
                return miss_value;
        return (-half_b - sqrtf(discriminant)) / a;
}
glm::vec3 Circle::normal_impl(const Ray &ray, float distance) const noexcept {
        glm::vec3 hit_point = ray.at(distance);
        glm::vec3 normal_at_hit = (hit_point - m_center) / m_radius;
        
        return glm::dot(ray.direction, normal_at_hit) < epsilon ? normal_at_hit : -normal_at_hit;
}
glm::vec3 Circle::position_impl() const noexcept {
        return m_center;
}
glm::vec3 Circle::random_point_impl(uint32_t seed, glm::vec3 direction_to_world_point) const noexcept {
        return m_center + m_radius * random_vector_in_cone(seed, direction_to_world_point, 0.5f);
}


Plane::Plane(glm::vec3 normal, float distance) : m_normal(normal), m_distance(distance) {

}
float Plane::intersect_impl(const Ray &ray) const noexcept {
        const float denom = glm::dot(ray.direction, m_normal);
        if (std::abs(denom) < epsilon)
                return miss_value;  // Ray is parallel to the plane
        
        const float t = (glm::dot(m_normal, ray.origin) + m_distance) / -denom;
        if (t < epsilon)
                return miss_value;  // Intersection point is behind the ray
        
        return t;
}
glm::vec3 Plane::normal_impl(const Ray &ray, float distance) const noexcept {
        return glm::dot(ray.direction, m_normal) > epsilon ? -m_normal : m_normal;
}
glm::vec3 Plane::position_impl() const noexcept {
        return glm::vec3(0.0f);
}
glm::vec3 Plane::random_point_impl(uint32_t seed, glm::vec3) const noexcept {
        return glm::vec3();
}