#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <variant>

#include "ray.h"

template<class... Ts>
struct overloaded : Ts ... {
        using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template<typename Impl>
struct ShapeStruct {
        [[nodiscard]] float intersect(const Ray &ray) const noexcept {
                return static_cast<const Impl &>(*this).intersect_impl(ray);
        }
        [[nodiscard]] glm::vec3 normal(const Ray &ray, float distance) const noexcept {
                return static_cast<const Impl &>(*this).normal_impl(ray, distance);
        }
        [[nodiscard]] glm::vec3 position() const noexcept {
                return static_cast<const Impl &>(*this).position_impl();
        }
        [[nodiscard]] glm::vec3 random_point(uint32_t seed, glm::vec3 world_point) const noexcept {
                return static_cast<const Impl &>(*this).random_point_impl(seed, world_point);
        }
};


class Triangle : public ShapeStruct<Triangle> {
public:  // Public Member Functions
        Triangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
public:  // Public Constructors/Destructors/Overloads
        [[nodiscard]] float intersect_impl(const Ray &ray) const noexcept;
        [[nodiscard]] glm::vec3 normal_impl(const Ray &ray, float distance) const noexcept;
        [[nodiscard]] glm::vec3 position_impl() const noexcept;
        [[nodiscard]] glm::vec3 random_point_impl(uint32_t seed, glm::vec3 world_point) const noexcept;
public:  // Public Member Variables
private: // Private Member Functions
private: // Private Member Variablesg
        glm::vec3 m_p1{};
        glm::vec3 m_p2{};
        glm::vec3 m_p3{};
        glm::vec3 m_normal{}; // precomputed as triangle normals are expensive to calculate
};

class Circle : public ShapeStruct<Circle> {
public:  // Public Constructors/Destructors/Overloads
        Circle(glm::vec3 center, float radius);
public:  // Public Member Functions
        [[nodiscard]] float intersect_impl(const Ray &ray) const noexcept;
        [[nodiscard]] glm::vec3 normal_impl(const Ray &ray, float distance) const noexcept;
        [[nodiscard]] glm::vec3 position_impl() const noexcept;
        [[nodiscard]] glm::vec3 random_point_impl(uint32_t seed, glm::vec3 world_point) const noexcept;
public:  // Public Member Variables
private: // Private Member Functions
        glm::vec3 m_center{};
        float m_radius{};
public: // Private Member Variables
};

class Plane : public ShapeStruct<Plane> {
public:  // Public Constructors/Destructors/Overloads
        Plane(glm::vec3 normal, float distance);
public:  // Public Member Functions
        [[nodiscard]] float intersect_impl(const Ray &ray) const noexcept;
        [[nodiscard]] glm::vec3 normal_impl(const Ray &ray, float distance) const noexcept;
        [[nodiscard]] glm::vec3 position_impl() const noexcept;
        [[nodiscard]] glm::vec3 random_point_impl(uint32_t seed, glm::vec3 world_point) const noexcept;
public:  // Public Member Variables
private: // Private Member Functions
        glm::vec3 m_normal{};
        float m_distance{};
private: // Private Member Variables
};

class Shape : public std::variant<Triangle, Circle, Plane> {
public:  // Public Constructors/Destructors/Overloads
        using variant<Triangle, Circle, Plane>::variant;
public:  // Public Member Functions
        [[nodiscard]] float intersect(const Ray &ray) const noexcept {
                return std::visit([ray](auto &&shape) { return shape.intersect(ray); }, *this);
        }

        [[nodiscard]] glm::vec3 normal(const Ray &ray, float distance) const noexcept {
                return std::visit([ray, distance](auto &&shape) { return shape.normal(ray, distance); }, *this);
        }

        [[nodiscard]] glm::vec3 position() const noexcept {
                return std::visit([](auto &&shape) { return shape.position(); }, *this);
        }
        
        [[nodiscard]] glm::vec3 random_point(uint32_t seed, glm::vec3 world_point) const noexcept {
                return std::visit([seed, world_point](auto &&shape) { return shape.random_point(seed, world_point); }, *this);
        }
};