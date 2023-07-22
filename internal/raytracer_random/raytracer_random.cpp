#include "raytracer_random.h"

uint32_t pcg_hash(uint32_t input) {
        uint32_t state = input * 747796405u + 2891336453u;
        uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
}

float random_pcg(uint32_t &seed) {
        seed = pcg_hash(seed);
        return (float) seed / (float) std::numeric_limits<uint32_t>::max();
}

float random_pcg(uint32_t &seed, float min, float max) {
        return min + (max - min) * random_pcg(seed);
}

glm::vec3 random_vec3_pcg(uint32_t &seed, float min, float max) {
        return {random_pcg(seed, min, max), random_pcg(seed, min, max), random_pcg(seed, min, max)};
}

glm::vec3 random_in_unit_sphere_pcg(uint32_t &seed) {
        while (true) {
                auto p = random_vec3_pcg(seed, -1, 1);
                if (glm::length2(p) >= 1) continue;
                return p;
        }
}

glm::vec3 random_unit_vector_pcg(uint32_t &seed) {
        return glm::normalize(random_in_unit_sphere_pcg(seed));
}


glm::vec3 random_vector_in_cone(uint32_t &seed, glm::vec3 N, float angle) {
        glm::vec2 xi{random_pcg(seed), random_pcg(seed)};
        float phi = 2.0f * std::numbers::pi_v<float> * xi.x;
        
        float theta = sqrtf(xi.y) * angle;
        float cosTheta = cosf(theta);
        float sinTheta = sinf(theta);
        
        glm::vec3 H;
        H.x = cosf(phi) * sinTheta;
        H.y = sinf(phi) * sinTheta;
        H.z = cosTheta;
        
        glm::vec3 up = abs(N.z) < 0.999 ? glm::vec3(0.0, 0.0, 1.0) : glm::vec3(1.0, 0.0, 0.0);
        glm::vec3 tangent = normalize(cross(up, N));
        glm::vec3 bitangent = cross(N, tangent);
        glm::mat3 tbn = glm::mat3(tangent, bitangent, N);
        
        glm::vec3 sampleVec = tbn * H;
        return normalize(sampleVec);
}