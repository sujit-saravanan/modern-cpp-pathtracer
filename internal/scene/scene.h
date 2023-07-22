#pragma once
#include "image.h"
#include "shape.h"
#include "ray.h"
#include "camera.h"
#include "shape_soa.h"
#include "raytracer_random.h"

static constexpr int sample_count = 20000;
static constexpr int recurse_depth = 2000;

template<uint32_t WIDTH, uint32_t HEIGHT>
class Scene {
public:  // Public Constructors/Destructors/Overloads
        Scene() = default;
        explicit Scene(glm::vec3 camera_origin, glm::vec3 camera_look_direction, glm::vec3 camera_up_direction, float camera_fov)
                : m_camera(camera_origin, camera_look_direction, camera_up_direction, camera_fov, float(WIDTH) / float(HEIGHT)) {};
public:  // Public Member Functions
#ifndef SOA
        void addShape(const Shape &shape, glm::vec3 color, float intensity) noexcept;
        void addShape(Shape &&shape, glm::vec3 color, float intensity) noexcept;
#endif

        void render();
        void traceScanline(uint32_t v, int recursion_depth);
        
        glm::vec3 sample(uint32_t &seed, Ray &&ray, int recursion_depth, uint32_t &samples_obtained);
#ifndef SOA
        HitBuffer intersectWorld(const Ray &ray);
#endif
#ifdef SOA
        HitBuffer intersectSoA(const Ray &ray);
#endif
public:  // Public Member Variables
private: // Private Member Functions
public: // Private Member Variables
#ifndef SOA
        std::vector<Shape> m_shapes{};
        std::vector<size_t> m_light_indices{};
        
        // Material info
        std::vector<glm::vec3> m_colors{};
        std::vector<float> m_intensities{};
#endif
        Image<WIDTH, HEIGHT> m_image{};
        Image<WIDTH, HEIGHT> m_bloom_image{};
        Camera m_camera;
#ifdef SOA
        ShapeSoA m_shape_soa;
#endif
};



#ifndef SOA
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
        return HitBuffer{.index = closest_shape_index, .distance = closest_intersection_distance};
}
#endif
#ifdef SOA
template<uint32_t WIDTH, uint32_t HEIGHT>
HitBuffer Scene<WIDTH, HEIGHT>::intersectSoA(const Ray &ray) {
        return m_shape_soa.intersect_all(ray);
}
#endif

template<uint32_t WIDTH, uint32_t HEIGHT>
glm::vec3 Scene<WIDTH, HEIGHT>::sample(uint32_t &seed, Ray &&ray, int recursion_depth, uint32_t &samples_obtained) {
        if (recursion_depth <= 0) { // Return miss color when recursion depth is exceeded
                samples_obtained++;
                return glm::vec3{0};
        }

#ifdef SOA
        HitBuffer hit = intersectSoA(ray);
#else
        HitBuffer hit = intersectWorld(ray);
#endif
        if (!hit.is_hit()) { // Return miss color on miss
                samples_obtained++;
                return glm::vec3{0.0, 0.0, 0.0};
        }

#ifdef SOA
        auto color = m_shape_soa.color(hit.shape_type, hit.index);
        auto intensity =  m_shape_soa.intensity(hit.shape_type, hit.index);
#else
        auto color = m_colors[hit.index];
        auto intensity =  m_intensities[hit.index];
#endif
        if (intensity > 0) {
                samples_obtained++;
                if (recursion_depth == recurse_depth)
                        return (color * intensity) / 255.0f; // Return light color on direct light intersection
                return {0, 0, 0}; // Return miss color on indirect light intersection
        }
        
        auto hit_location = ray.at(hit.distance);
#ifdef SOA
        auto normal = m_shape_soa.normal(hit.shape_type, hit.index, ray, hit.distance);
#else
        auto normal = m_shapes[hit.index].normal(ray, hit.distance);
#endif
        
        // Next event estimation
        glm::vec3 next_event_color{0};
#ifdef SOA
        for (uint32_t light_index: m_shape_soa.m_circle_light_indices) {
                Shape light_source = m_shape_soa.circles[light_index];
#else
        for (uint32_t light_index: m_light_indices) {
                Shape light_source = m_shapes[light_index];
#endif
                // Sample a point on the light source
                glm::vec3 light_point = light_source.random_point(seed, light_source.position() - hit_location);
                
                // Calculate the direction from the hit point to the light source
                float light_distance = glm::length2(light_point - hit_location);
                glm::vec3 light_direction = glm::normalize(light_point - hit_location);
                
                // Create a shadow ray to check if the hit point is occluded by other objects
                Ray shadow_ray(hit_location + normal * 0.001f, light_direction);
#ifdef SOA
                HitBuffer shadow_hit = intersectSoA(shadow_ray);
#else
                HitBuffer shadow_hit = intersectWorld(shadow_ray);
#endif
                
                // If the shadow ray is not occluded, calculate the light's contribution
                if (shadow_hit.index == light_index) {
                        // Calculate the light intensity and BRDF
#ifdef SOA
                        float light_intensity = m_shape_soa.intensity(ShapeType::Circle, light_index);
                        glm::vec3 light_color =  m_shape_soa.color(ShapeType::Circle, light_index);
#else
                        float light_intensity = m_intensities[light_index];
                        glm::vec3 light_color = m_colors[light_index];
#endif
                        next_event_color += light_intensity * light_color * glm::max(glm::dot(light_direction, normal), 0.0f) / light_distance;
                }
                samples_obtained++;
        }
        
        // Indirect Lighting
        glm::vec3 target = hit_location + normal + random_unit_vector_pcg(seed);
        auto new_ray = Ray(hit_location + normal * 0.001f, target - hit_location);
        
        float cos_theta = glm::max(glm::dot(new_ray.direction, normal), 0.0f);
        glm::vec3 brdf = color * cos_theta;
        
        glm::vec3 reflected = sample(seed, Ray(hit_location, target - hit_location), recursion_depth - 1, samples_obtained);
        samples_obtained++;
        
        return next_event_color / 255.0f + (brdf * reflected) / 255.0f;
}

template<uint32_t WIDTH, uint32_t HEIGHT>
void Scene<WIDTH, HEIGHT>::traceScanline(uint32_t v, int recursion_depth) {
        for (uint32_t x = 0; x < WIDTH; x++) {
                glm::vec3 pixel_color{};
                uint32_t samples_obtained = 0;
                uint32_t seed = x + v * WIDTH;
                
                for (int s = 0; s < sample_count; ++s) {
                        auto light = sample(seed, m_camera.get_ray(glm::vec2{x + random_pcg(seed), v + random_pcg(seed)} / glm::vec2{WIDTH, HEIGHT}), recursion_depth, samples_obtained);
                        pixel_color += light;
                }
                
                pixel_color /= float(samples_obtained);
                if (glm::length(pixel_color) > 5.0f) // Write bright pixels to the bloom buffer
                        m_bloom_image.set(x, v, pixel_color);
                else
                        m_bloom_image.set(x, v, {0, 0, 0});
                m_image.set(x, v, pixel_color);
        }
}

template<uint32_t WIDTH, uint32_t HEIGHT>
void Scene<WIDTH, HEIGHT>::render() {
        BS::thread_pool pool(12);
        
        for (uint32_t y = 0; y < HEIGHT; y++)
                pool.push_task(&Scene<WIDTH, HEIGHT>::traceScanline, this, y, recurse_depth);
        pool.wait_for_tasks();
}

#ifndef SOA
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
#endif