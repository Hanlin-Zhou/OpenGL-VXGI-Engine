#pragma once
#include <glm.hpp>

class Light {
public:
	glm::vec3 position;
	glm::vec3 normal;

	Light();
	void setPos(glm::vec3 pos);
	glm::vec3 getPos();
	void setState(bool onoff);
	void setIntensity(float i);
};
