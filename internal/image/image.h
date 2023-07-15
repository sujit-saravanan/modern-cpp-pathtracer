#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

template<uint32_t X, uint32_t Y>
class Image {
        typedef glm::u8vec3 rgb;
public:  // Public Constructors/Destructors/Overloads
public:  // Public Member Functions
        [[nodiscard]] inline consteval size_t x() noexcept { return X; }
        [[nodiscard]] inline consteval size_t y() noexcept { return Y; }
        [[nodiscard]] inline consteval glm::uvec2 resolution() noexcept { return m_image_resolution; }
        
        [[nodiscard]] constexpr std::vector<rgb> &data() noexcept { return m_data; }
        [[nodiscard]] constexpr const std::vector<rgb> &data() const noexcept { return m_data; }
        
        template<uint32_t INDEX_X, uint32_t INDEX_Y>
        requires (INDEX_X < X and INDEX_Y < Y)
        constexpr void set(rgb color) noexcept {
                m_data[INDEX_X * X + INDEX_Y] = color;
        }
        constexpr void set(uint32_t INDEX_X, uint32_t INDEX_Y, rgb color) noexcept {
                assert(INDEX_X < X and INDEX_Y < Y);
                m_data[INDEX_Y * X + INDEX_X] = color;
        }
        
        void writeToFile(const char *filepath) noexcept {
                stbi_flip_vertically_on_write(true);
                stbi_write_png(filepath, X, Y, 3, m_data.data(), 3 * m_image_resolution.x);
        }
public:  // Public Member Variables
private: // Private Member Functions
private: // Private Member Variables
        const glm::uvec2 m_image_resolution = {X, Y};
        std::vector<rgb> m_data = std::vector<rgb>(X * Y);
};