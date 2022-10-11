#include <DirectionalLight.h>

DirectionalLight::DirectionalLight() : Light(glm::vec3(0, 30, 0), glm::vec3(0, -1, 0.3), 6.0) {}