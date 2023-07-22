#pragma once
#include <random>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

[[nodiscard]] uint32_t pcg_hash(uint32_t input);

[[nodiscard]] float random_pcg(uint32_t &seed);

[[nodiscard]] float random_pcg(uint32_t &seed, float min, float max);

[[nodiscard]] glm::vec3 random_vec3_pcg(uint32_t &seed, float min, float max);

[[nodiscard]] glm::vec3 random_in_unit_sphere_pcg(uint32_t &seed);

[[nodiscard]] glm::vec3 random_unit_vector_pcg(uint32_t &seed);

[[nodiscard]] glm::vec3 random_vector_in_cone(uint32_t &seed, glm::vec3 N, float angle);