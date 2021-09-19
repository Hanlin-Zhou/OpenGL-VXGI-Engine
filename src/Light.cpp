#include <Light.h>


Light::Light(glm::vec3 pos = glm::vec3(0.0, 5.0, 0.0), glm::vec3 norm = glm::vec3(0, -1, 0), float str = 1000) {
	position = pos;
	normal = norm;
	strength = str;
}

