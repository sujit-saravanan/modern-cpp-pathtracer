#pragma once
#include "image.h"
#include "shape.h"
#include "ray.h"
#include "camera.h"
#include <random>
#include <glm/gtx/norm.hpp>


static constexpr int sample_count = 20000;
static constexpr int recurse_depth = 2000;

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

inline static glm::vec3 cosine_hemisphere(uint32_t &seed, glm::vec3 n) {
        // Create a vector perpendicular to the normal
        uint nx = uint(n.x == 0);
        glm::vec3 perp = cross(n, glm::vec3(nx, 0, 1-nx));
        
        // Rotate perp randomly to create a hollow disk
        float theta = random_pcg(seed) * std::numbers::pi_v<float>;
        glm::vec3 rotated = (perp * cosf(theta)) + (cross(n, perp) * sinf(theta));
        
        // Multiply it by a random radius to fill the disk
        float r = random_pcg(seed);
        glm::vec3 disk = rotated * (r / length(rotated));
        float up = sqrtf(1 - (r*r));
        return disk + (n * up);
}

inline static glm::vec3 random_unit_vector_pcg(uint32_t &seed) {
        return glm::normalize(random_in_unit_sphere_pcg(seed));
}


struct HitBuffer {
        size_t index;
        float distance;
        [[nodiscard]] bool is_hit() const { return distance > 0.0001f && distance < std::numeric_limits<float>::max(); };
};
template<uint32_t WIDTH, uint32_t HEIGHT>
class Scene {
public:  // Public Constructors/Destructors/Overloads
        Scene() = default;
        explicit Scene(glm::vec3 camera_origin, glm::vec3 camera_look_direction, glm::vec3 camera_up_direction, float camera_fov)
                : m_camera(camera_origin, camera_look_direction, camera_up_direction, camera_fov, float(WIDTH) / float(HEIGHT)) {};
public:  // Public Member Functions
        void addShape(const Shape &shape, glm::vec3 color, float intensity) noexcept;
        void addShape(Shape &&shape, glm::vec3 color, float intensity) noexcept;
        
        void render();
        void trace(uint32_t u, uint32_t v, int recursion_depth);
        
        glm::vec3 sample(uint32_t &seed, Ray &&ray, int recursion_depth, uint32_t& samples_obtained);
        HitBuffer intersectWorld(const Ray &ray);
public:  // Public Member Variables
private: // Private Member Functions
public: // Private Member Variables
        std::vector<Shape> m_shapes{};
        std::vector<size_t> m_light_indices{};
        
        // Material info
        std::vector<glm::vec3> m_colors{};
        std::vector<float> m_intensities{};
        
        Image<WIDTH, HEIGHT> m_image{};
        Camera m_camera;
};


template<uint32_t WIDTH, uint32_t HEIGHT>
HitBuffer Scene<WIDTH, HEIGHT>::intersectWorld(const Ray &ray) {
        float closest_intersection_distance = std::numeric_limits<float>::max();
        size_t closest_shape_index = 0;
        for (int i = 0; auto &shape: m_shapes) {
                float intersection_dist = shape.intersect(ray);
                if (intersection_dist > 0.001 && intersection_dist < closest_intersection_distance) {
                        closest_intersection_distance = intersection_dist;
                        closest_shape_index = i;
                }
                i++;
        }
        
        return {.index = closest_shape_index, .distance = closest_intersection_distance};
}

template<uint32_t WIDTH, uint32_t HEIGHT>
glm::vec3 Scene<WIDTH, HEIGHT>::sample(uint32_t &seed, Ray &&ray, int recursion_depth, uint32_t& samples_obtained) {
        if (recursion_depth <= 0){ // Return miss color when recursion depth is exceeded
                samples_obtained++;
                return glm::vec3{0};
        }
        
        HitBuffer hit = intersectWorld(ray);
        if (!hit.is_hit()){ // Return miss color on miss
                samples_obtained++;
                return glm::vec3{0.0, 0.0, 0.0};
        }
        
        auto color = m_colors[hit.index];
        auto intensity = m_intensities[hit.index];
        if (intensity > 0){
                samples_obtained++;
                if (recursion_depth == recurse_depth)
                        return (color * intensity) / 255.0f; // Return light color
                return {0, 0, 0}; // Return miss color on light intersect (For debugging)
        }
        
        auto hit_location = ray.at(hit.distance);
        auto normal = m_shapes[hit.index].normal(ray, hit.distance);
        
        // Next event estimation
        glm::vec3 next_event_color{0};
        for (uint32_t light_index : m_light_indices) {
                Shape light_source = m_shapes[light_index];
                // Sample a point on the light source
                glm::vec3 light_point = light_source.random_point(seed, hit_location);

                // Calculate the direction from the hit point to the light source
                float light_distance = glm::length2(light_point - hit_location);
                glm::vec3 light_direction = glm::normalize(light_point - hit_location);

                // Create a shadow ray to check if the hit point is occluded by other objects
                Ray shadow_ray(hit_location + normal * 0.001f, light_direction);
                HitBuffer shadow_hit = intersectWorld(shadow_ray);

                // If the shadow ray is not occluded, calculate the light's contribution
                if (shadow_hit.index == light_index) {
                        // Calculate the light intensity and BRDF
                        float light_intensity = m_intensities[light_index];
                        glm::vec3 light_color = m_colors[light_index];
                        next_event_color += light_intensity * light_color * glm::max(glm::dot(light_direction, normal), 0.0f) / light_distance;
                }
                samples_obtained++;
        }
        
        // Indirect Lighting
        glm::vec3 target = hit_location + normal + random_unit_vector_pcg(seed);
        auto new_ray = Ray(hit_location + normal * 0.001f, target - hit_location);
        
        float cos_theta = glm::max(glm::dot(new_ray.direction, normal), 0.0f);
        glm::vec3 brdf = color * cos_theta;
        
        glm::vec3 reflected = sample(seed,Ray(hit_location, target - hit_location), recursion_depth - 1, samples_obtained);
        samples_obtained++;

        return   next_event_color / 255.0f + (brdf * reflected) / 255.0f;
}

template<uint32_t WIDTH, uint32_t HEIGHT>
void Scene<WIDTH, HEIGHT>::trace(uint32_t u, uint32_t v, int recursion_depth) {
        glm::vec3 pixel_color{};
        uint32_t samples_obtained = 0;
        uint32_t seed = u + v * WIDTH;
        for (int s = 0; s < sample_count; ++s) {
                auto light = sample(seed, m_camera.get_ray(glm::vec2{u + random_pcg(seed), v + random_pcg(seed)} / glm::vec2{WIDTH, HEIGHT}), recursion_depth, samples_obtained);
                pixel_color += light;
        }
        
        pixel_color /= float(samples_obtained);
        pixel_color /= (1.0f + pixel_color);
        pixel_color *= 255.0f;
        m_image.set(u, v, pixel_color);
}

template<uint32_t WIDTH, uint32_t HEIGHT>
void Scene<WIDTH, HEIGHT>::render() {
        BS::thread_pool pool(12);
        
        for (uint32_t y = 0; y < HEIGHT; y++)
                for (uint32_t x = 0; x < WIDTH; x++)
//                        trace(x, y, recurse_depth);
                        pool.push_task(&Scene<WIDTH, HEIGHT>::trace, this, x, y, recurse_depth);
        pool.wait_for_tasks();
}

template<uint32_t WIDTH, uint32_t HEIGHT>
void Scene<WIDTH, HEIGHT>::addShape(const Shape &shape, glm::vec3 color, float intensity) noexcept {
        m_shapes.emplace_back(shape);
        m_colors.emplace_back(color);
        m_intensities.emplace_back(intensity);
        if (intensity > std::numeric_limits<float>::epsilon())
                m_light_indices.push_back(m_shapes.size() - 1);
}
template<uint32_t WIDTH, uint32_t HEIGHT>
void Scene<WIDTH, HEIGHT>::addShape(Shape &&shape, glm::vec3 color, float intensity) noexcept {
        m_shapes.emplace_back(shape);
        m_colors.emplace_back(color);
        m_intensities.emplace_back(intensity);
        if (intensity > std::numeric_limits<float>::epsilon())
                m_light_indices.push_back(m_shapes.size() - 1);
}
