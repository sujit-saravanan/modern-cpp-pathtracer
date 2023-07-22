#pragma once
#include "shape.h"


#ifdef SOA

enum class ShapeType {
        Circle, Triangle, Plane
};
#endif

struct HitBuffer {
        size_t index;
        float distance;
#ifdef SOA
        ShapeType shape_type;
#endif
        [[nodiscard]] bool is_hit() const { return distance > 0.0001f && distance < std::numeric_limits<float>::max(); };
};

#ifdef SOA


class ShapeSoA {
public:
        std::vector<Circle> circles;
        std::vector<Triangle> triangles;
        std::vector<Plane> planes;
        
        std::vector<glm::vec3> m_circle_colors;
        std::vector<glm::vec3> m_triangle_colors;
        std::vector<glm::vec3> m_plane_colors;
        std::vector<glm::vec3> m_triangle_normals;
        
        std::vector<float> m_circle_intensities;
        std::vector<float> m_triangle_intensities;
        std::vector<float> m_plane_intensities;
        
        std::vector<uint32_t> m_circle_light_indices;
        std::vector<uint32_t> m_triangle_light_indices;
        std::vector<uint32_t> m_plane_light_indices;
        
        void insert(const Circle &circle, glm::vec3 color, float intensity) {
                circles.push_back(circle);
                m_circle_colors.push_back(color);
                m_circle_intensities.push_back(intensity);
                if (intensity > 0)
                        m_circle_light_indices.push_back(circles.size() - 1);
        }
        void insert(Circle &&circle, glm::vec3 color, float intensity) {
                circles.push_back(circle);
                m_circle_colors.push_back(color);
                m_circle_intensities.push_back(intensity);
                if (intensity > 0)
                        m_circle_light_indices.push_back(circles.size() - 1);
        }
        
        void insert(const Triangle &triangle, glm::vec3 color, float intensity) {
                triangles.push_back(triangle);
                m_triangle_colors.push_back(color);
                m_triangle_intensities.push_back(intensity);
                m_triangle_normals.push_back(triangle.calculate_normal());
                if (intensity > 0)
                        m_triangle_light_indices.push_back(circles.size() - 1);
        }
        void insert(Triangle &&triangle, glm::vec3 color, float intensity) {
                triangles.push_back(triangle);
                m_triangle_colors.push_back(color);
                m_triangle_intensities.push_back(intensity);
                m_triangle_normals.push_back(triangle.calculate_normal());
                if (intensity > 0)
                        m_triangle_light_indices.push_back(circles.size() - 1);
        }
        
        void insert(const Plane &plane, glm::vec3 color, float intensity) {
                planes.push_back(plane);
                m_plane_colors.push_back(color);
                m_plane_intensities.push_back(intensity);
                if (intensity > 0)
                        m_plane_light_indices.push_back(circles.size() - 1);
        }
        void insert(Plane &&plane, glm::vec3 color, float intensity) {
                planes.push_back(plane);
                m_plane_colors.push_back(color);
                m_plane_intensities.push_back(intensity);
                if (intensity > 0)
                        m_plane_light_indices.push_back(circles.size() - 1);
        }
        
        glm::vec3 color(ShapeType shape_type, uint32_t index) {
                switch (shape_type) {
                        case ShapeType::Circle:
                                return m_circle_colors[index];
                        case ShapeType::Triangle:
                                return m_triangle_colors[index];
                        case ShapeType::Plane:
                                return m_plane_colors[index];
                }
        }
        
        float intensity(ShapeType shape_type, uint32_t index) {
                switch (shape_type) {
                        case ShapeType::Circle:
                                return m_circle_intensities[index];
                        case ShapeType::Triangle:
                                return m_triangle_intensities[index];
                        case ShapeType::Plane:
                                return m_plane_intensities[index];
                }
        }
        
        glm::vec3 normal(ShapeType shape_type, uint32_t index, const Ray& ray, float distance) {
                switch (shape_type) {
                        case ShapeType::Circle:
                                return circles[index].normal(ray, distance);
                        case ShapeType::Triangle:
                                return glm::dot(ray.direction, m_triangle_normals[index]) > std::numeric_limits<float>::epsilon() ? -m_triangle_normals[index] : m_triangle_normals[index];
//                                return m_triangle_normals[index];
                        case ShapeType::Plane:
                                return planes[index].normal(ray, distance);
                }
        }
        
        
        HitBuffer intersect_all(const Ray &ray) {
                float closest_intersection_distance = std::numeric_limits<float>::max();
                size_t closest_shape_index = 0;
                ShapeType closest_shape_type;
                for (int i = 0; auto &shape: circles) {
                        float intersection_dist = shape.intersect(ray);
                        if (intersection_dist > 0.001 && intersection_dist < closest_intersection_distance) {
                                closest_intersection_distance = intersection_dist;
                                closest_shape_index = i;
                                closest_shape_type = ShapeType::Circle;
                        }
                        i++;
                }
                for (int i = 0; auto &shape: triangles) {
                        float intersection_dist = shape.intersect(ray);
                        if (intersection_dist > 0.001 && intersection_dist < closest_intersection_distance) {
                                closest_intersection_distance = intersection_dist;
                                closest_shape_index = i;
                                closest_shape_type = ShapeType::Triangle;
                        }
                        i++;
                }
                for (int i = 0; auto &shape: planes) {
                        float intersection_dist = shape.intersect(ray);
                        if (intersection_dist > 0.001 && intersection_dist < closest_intersection_distance) {
                                closest_intersection_distance = intersection_dist;
                                closest_shape_index = i;
                                closest_shape_type = ShapeType::Plane;
                        }
                        i++;
                }
                
                return {.index = closest_shape_index, .distance = closest_intersection_distance, .shape_type = closest_shape_type};
        }
};
#endif