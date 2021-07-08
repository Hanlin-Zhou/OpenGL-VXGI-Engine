#include <glm.hpp>

class Light {
public:
	glm::vec3 position;
	glm::vec3 normal;

	Light() {
		position = glm::vec3(1, 5.3, 0.5);
		normal = glm::vec3(0, -1, 0);
	}

};
