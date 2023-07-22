#include "shape.h"
#include <glm/gtx/norm.hpp>

static constexpr float miss_value = std::numeric_limits<float>::max();
static constexpr float epsilon = std::numeric_limits<float>::epsilon();

inline uint32_t pcg_hash(uint32_t input) {
        uint32_t state = input * 747796405u + 2891336453u;
        uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
}

inline float random_pcg(uint32_t &seed) {
        seed = pcg_hash(seed);
        return (float) seed / (float) std::numeric_limits<uint32_t>::max();
}

inline float random_pcg(uint32_t &seed, float min, float max) {
        return min + (max - min) * random_pcg(seed);
}

inline static glm::vec3 random_vec3_pcg(uint32_t &seed, float min, float max) {
        return {random_pcg(seed, min, max), random_pcg(seed, min, max), random_pcg(seed, min, max)};
}

inline static glm::vec3 random_in_unit_sphere_pcg(uint32_t &seed) {
        while (true) {
                auto p = random_vec3_pcg(seed, -1, 1);
                if (glm::length2(p) >= 1) continue;
                return p;
        }
}

inline static glm::vec3 random_unit_vector_pcg(uint32_t &seed) {
        return glm::normalize(random_in_unit_sphere_pcg(seed));
}

Triangle::Triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) : m_p1(p1), m_p2(p2), m_p3(p3) {
        glm::vec3 edge1 = m_p2 - m_p1;
        glm::vec3 edge2 = m_p3 - m_p1;
        m_normal = glm::normalize(glm::cross(edge1, edge2));
        
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
        return glm::dot(ray.direction, m_normal) > epsilon ? -m_normal : m_normal;
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
glm::vec3 Circle::random_point_impl(uint32_t seed, glm::vec3) const noexcept {
        glm::vec3 rand = random_unit_vector_pcg(seed);
        
        return m_center + m_radius * rand;
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