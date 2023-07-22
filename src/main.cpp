#include "scene.h"
#include "shape.h"

struct testt {
        std::vector<int> a;
        std::vector<double> b;
};

int main(int argc, char *argv[]) {
        std::cout << "Start\n";
        Scene<1200, 1200> scene({-2, 2, 1}, glm::vec3{0, 0, -1}, {0, 1, 0}, 90);

#ifdef SOA
        scene.m_shape_soa.insert(Circle(glm::vec3{0.0, 1.5, -1.0}, 1), {200, 100, 100}, 10);
        scene.m_shape_soa.insert(Plane(glm::vec3{0.0, 1.0, 0.0}, 0), {200, 200, 200}, 0);
        scene.m_shape_soa.insert(Triangle(glm::vec3{5.0, 0.0, 0.0}, glm::vec3{6.0, 1.0, 0.0}, glm::vec3{4.0, 0.0, 1.0}), {200, 100, 100}, 0);
        scene.m_shape_soa.insert(Triangle(glm::vec3{2.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0, 0.0, 1.0}), {100, 200, 100}, 0);
        scene.m_shape_soa.insert(Triangle(glm::vec3{-2.0, 0.0, 0.0}, glm::vec3{-1.0, 1.0, 0.0}, glm::vec3{-1.0, 0.0, 1.0}), {100, 100, 200}, 0);
        scene.m_shape_soa.insert(Triangle(glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0, 0.0, 1.0}), {100, 100, 200}, 0);
        scene.m_shape_soa.insert(Circle(glm::vec3{0.0, 0.0, -1.0}, 0.5), {255, 255, 255}, 10);
        scene.m_shape_soa.insert(Circle(glm::vec3{-1.0, 0.0, -1.0}, 0.5), {100, 200, 100}, 0);
        scene.m_shape_soa.insert(Circle(glm::vec3{1.0, 0.0, -1.0}, 0.5), {100, 100, 200}, 0);
#else
        scene.addShape(Circle(glm::vec3{0.0, 1.5, -1.0}, 1), {200, 100, 100}, 10);
        scene.addShape(Plane(glm::vec3{0.0, 1.0, 0.0}, 0), {200, 200, 200}, 0);
        scene.addShape(Triangle(glm::vec3{5.0, 0.0, 0.0}, glm::vec3{6.0, 1.0, 0.0}, glm::vec3{4.0, 0.0, 1.0}), {200, 100, 100}, 0);
        scene.addShape(Triangle(glm::vec3{2.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0, 0.0, 1.0}), {100, 200, 100}, 0);
        scene.addShape(Triangle(glm::vec3{-2.0, 0.0, 0.0}, glm::vec3{-1.0, 1.0, 0.0}, glm::vec3{-1.0, 0.0, 1.0}), {100, 100, 200}, 0);
        scene.addShape(Triangle(glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0, 0.0, 1.0}), {100, 100, 200}, 0);
        scene.addShape(Circle(glm::vec3{0.0, 0.0, -1.0}, 0.5), {255, 255, 255}, 10);
        scene.addShape(Circle(glm::vec3{-1.0, 0.0, -1.0}, 0.5), {100, 200, 100}, 0);
        scene.addShape(Circle(glm::vec3{1.0, 0.0, -1.0}, 0.5), {100, 100, 200}, 0);
#endif

        scene.render();
        scene.m_image.writeToFile("assets/test.png");
        std::cout << "Done\n";
        std::cout << "Applying Bloom\n";
        scene.m_bloom_image.writeToFile("assets/mask.png");
        scene.m_bloom_image.box_blur(0);
        scene.m_bloom_image.writeToFile("assets/bloom.png");

        scene.m_image.additive_blend(scene.m_bloom_image);
        scene.m_image.writeToFile("assets/post.png");
        std::cout << "Done\n";
        return 0;
}
