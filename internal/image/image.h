#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

template<uint32_t X, uint32_t Y>
class Image {
        typedef glm::vec3 rgb;
        typedef glm::u8vec3 rgb_u8;
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
                std::vector<rgb_u8> final_buffer(X * Y);
                for (int i = 0; i < m_data.size(); i++) {
                        glm::vec3 pixel_color = m_data[i] / (1.0f + m_data[i]);
//                        pixel_color /= (1.0f + pixel_color);
                        pixel_color *= 255.0f;
                        final_buffer[i] = pixel_color;
                }
                stbi_flip_vertically_on_write(true);
                stbi_write_png(filepath, X, Y, 3, final_buffer.data(), 3 * m_image_resolution.x);
        }
        
        rgb getPixelOrBlack(int x, int y) {
                if (x < 0 || x >= X || y < 0 || y >= Y)
                        return {0, 0, 0};
                return m_data[y * X + x];
        }
        
        rgb box_average(uint32_t offset_x, uint32_t offset_y, uint32_t blur_radius) {
                glm::vec3 total{0};
                for (uint32_t y = offset_y; y < offset_y+blur_radius; y++)
                        for (uint32_t x = offset_x; x < offset_x + blur_radius; x++)
                                total += getPixelOrBlack(x, y);
                return (total) / float(blur_radius * blur_radius);
        }
        
        void box_blur(uint32_t blur_radius) noexcept { // 3x3 box blur
                if (blur_radius == 0)
                        return;
                
                auto image = std::vector<rgb>(X * Y);
                for (size_t y = blur_radius; y < Y; y++)
                        for (size_t x = blur_radius; x < X; x++)
                                image[y * X + x] = box_average(x - blur_radius / 2, y - blur_radius / 2, blur_radius);
                m_data = image;
        }
        
        void additive_blend(const Image &image) noexcept {
                for (size_t y = 0; y < Y; y++)
                        for (size_t x = 0; x < X; x++)
                                if (image.data()[y * X + x] != glm::vec3{0, 0, 0})
                                        m_data[y * X + x] = m_data[y * X + x] + image.data()[y * X + x];
        }
public:  // Public Member Variables
private: // Private Member Functions
private: // Private Member Variables
        const glm::uvec2 m_image_resolution = {X, Y};
        std::vector<rgb> m_data = std::vector<rgb>(X * Y);
};