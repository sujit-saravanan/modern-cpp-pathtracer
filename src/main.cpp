#include "scene.h"
#include "shape.h"

int main(int argc, char *argv[]) {
        std::cout << "Start\n";
        Scene<800, 450> scene({-2, 2, 1}, glm::vec3{0, 0, -1}, {0, 1, 0}, 90);
        scene.addShape(Circle(glm::vec3{0.0, 1.5, -1.0}, 1), {200, 100, 100}, 0);
        scene.addShape(Plane(glm::vec3{0.0, 1.0, 0.0}, 0), {200, 200, 200}, 0);
        scene.addShape(Triangle(glm::vec3{5.0, 0.0, 0.0}, glm::vec3{6.0, 1.0, 0.0}, glm::vec3{4.0, 0.0, 1.0}), {200, 100, 100}, 0);
        scene.addShape(Triangle(glm::vec3{2.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0, 0.0, 1.0}), {100, 200, 100}, 0);
        scene.addShape(Triangle(glm::vec3{-2.0, 0.0, 0.0}, glm::vec3{-1.0, 1.0, 0.0}, glm::vec3{-1.0, 0.0, 1.0}), {100, 100, 200}, 0);
        scene.addShape(Triangle(glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0, 0.0, 1.0}), {100, 100, 200}, 0);
        scene.addShape(Circle(glm::vec3{0.0, 0.0, -1.0}, 0.5), {255, 255, 255}, 10000);
        scene.addShape(Circle(glm::vec3{-1.0, 0.0, -1.0}, 0.5), {100, 200, 100}, 0);
        scene.addShape(Circle(glm::vec3{1.0, 0.0, -1.0}, 0.5), {100, 100, 200}, 0);

        scene.render();
        scene.m_image.writeToFile("assets/test.png");
        std::cout << "Done\n";
        
        return 0;
}
