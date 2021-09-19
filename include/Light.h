#pragma once
#include <glm.hpp>

class Light {
public:
	glm::vec3 position;
	glm::vec3 normal;
	float strength;

	Light(glm::vec3 pos, glm::vec3 norm, float str);
};
