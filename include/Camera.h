#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>

class Camera 
{
private:
	glm::vec3 camPosition;
	glm::vec3 camUp;
	glm::vec3 camLookAt;
	glm::vec3 camRight;
	float near_plane;
	float far_plane ;
	float fov;
	
public:
	float aspect;

	Camera(glm::vec3 position, glm::vec3 lookat, float in_aspect);
	void translate(glm::vec3 offset);
	void rotate(float horizontalRad, float verticalRad);
	void reset();
	glm::vec3 getLookingDirection();
	glm::vec3 getRightHandDirection();
	glm::vec3 getUpDirection();
	glm::vec3 getPosition();
	glm::mat4 getViewMat();
	glm::mat4 getProjMat();
};