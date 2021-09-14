#include <Light.h>


Light::Light() {
	position = glm::vec3(1, 10.3, 0.5);
	normal = glm::vec3(0, -1, 0);
	strength = 1000;
}


void Light::setPos(glm::vec3 pos) {
	position = pos;
}


glm::vec3 Light::getPos() {
	return position;
}

