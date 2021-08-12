#include <Camera.h>

//glm::vec3 Camera::camPosition = glm::vec3(0.0f, 0.0f, 10.0f);
//glm::vec3 Camera::camUp = glm::vec3(0.0f, 1.0f, 0.0f);
//glm::vec3 Camera::camLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
//glm::vec3 Camera::camRight = glm::normalize(glm::cross(glm::normalize(Camera::camPosition - Camera::camLookAt), Camera::camUp));

Camera::Camera(glm::vec3 position, glm::vec3 lookat, float in_aspect) {
	camPosition = position;
	camLookAt = lookat;
	camUp = glm::vec3(0.0f, 1.0f, 0.0f);
	camRight = glm::normalize(glm::cross(glm::normalize(camPosition - camLookAt), camUp));
	near_plane = 0.1;
	far_plane = 80.0;
	fov = 90.0;
	aspect = in_aspect;
}


void Camera::rotate(float horizontalRad, float verticalRad) {
	glm::mat4 rot_mat = glm::mat4(1.0f);
	rot_mat = glm::rotate(rot_mat, horizontalRad, camUp);
	rot_mat = glm::rotate(rot_mat, verticalRad, camRight);
	glm::vec4 temp_cameraPos = rot_mat * glm::vec4(camPosition.x - camLookAt.x, camPosition.y - camLookAt.y, camPosition.z - camLookAt.z, 1.0);
	camPosition = glm::vec3(camLookAt.x + temp_cameraPos.x / temp_cameraPos[3], camLookAt.y + temp_cameraPos.y / temp_cameraPos[3], camLookAt.z + temp_cameraPos.z / temp_cameraPos[3]);
	camRight = glm::normalize(glm::cross(glm::normalize(camPosition - camLookAt), camUp));
}


void Camera::translate(glm::vec3 offset) {
	camPosition += offset;
	camLookAt += offset;
}


void Camera::reset() {
	camPosition = glm::vec3(0.0f, 0.0f, 10.0f);
	camLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
	camUp = glm::vec3(0.0f, 1.0f, 0.0f);
	camRight = glm::normalize(glm::cross(glm::normalize(camPosition - camLookAt), camUp));
}

glm::vec3 Camera::getLookingDirection() {
	return glm::normalize(camLookAt - camPosition);
}


glm::vec3 Camera::getRightHandDirection() {
	return camRight;
}


glm::vec3 Camera::getUpDirection() {
	return camUp;
}


glm::vec3 Camera::getPosition() {
	return camPosition;
}


glm::mat4 Camera::getViewMat() {
	return glm::lookAt(camPosition, camLookAt, camUp);
}


glm::mat4 Camera::getProjMat() {
	return glm::perspective(glm::radians(fov), aspect, near_plane, far_plane);
}