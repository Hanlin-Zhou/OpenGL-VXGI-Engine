#include <Camera.h>


Camera::Camera(glm::vec3 position, glm::vec3 lookat, float in_aspect) {
	camPosition = position;
	camLookAt = lookat;
	camUp = glm::vec3(0.0f, 1.0f, 0.0f);
	camRight = glm::normalize(glm::cross(glm::normalize(camPosition - camLookAt), camUp));
	near_plane = 0.1;
	far_plane = 80.0;
	fov = 90.0;
	aspect = in_aspect;

	std::ifstream inputfile("./json/CameraSetting.json");
	inputfile >> setting;
	IDs = setting["IDs"].get<std::vector<int>>();
}


void Camera::rotate(float horizontalRad, float verticalRad) {
	glm::mat4 rot_mat = glm::mat4(1.0f);
	rot_mat = glm::rotate(rot_mat, horizontalRad, camUp);
	rot_mat = glm::rotate(rot_mat, verticalRad, camRight);
	glm::vec4 temp_cameraPos = rot_mat * glm::vec4(camPosition.x - camLookAt.x, camPosition.y - camLookAt.y, camPosition.z - camLookAt.z, 1.0);
	camPosition = glm::vec3(camLookAt.x + temp_cameraPos.x / temp_cameraPos[3], camLookAt.y + temp_cameraPos.y / temp_cameraPos[3], camLookAt.z + temp_cameraPos.z / temp_cameraPos[3]);
	camRight = glm::normalize(glm::cross(glm::normalize(camPosition - camLookAt), camUp));
}


void Camera::turn(float horizontalRad, float verticalRad) {
	glm::vec4 dir = glm::vec4(camLookAt.x - camPosition.x, camLookAt.y - camPosition.y, camLookAt.z - camPosition.z, 1.0);
	glm::mat4 rot_mat = glm::mat4(1.0f);
	rot_mat = glm::rotate(rot_mat, horizontalRad, camUp);
	rot_mat = glm::rotate(rot_mat, verticalRad, camRight);
	dir = rot_mat * dir;
	camLookAt = camPosition + glm::vec3(dir.x, dir.y, dir.z);
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


void Camera::savePreset(int index, bool isNew) {
	std::ofstream outputfile("./json/CameraSetting.json");
	std::string indexString = std::to_string(index);
	
	setting[indexString]["camPosition"]["0"] = camPosition[0];
	setting[indexString]["camPosition"]["1"] = camPosition[1];
	setting[indexString]["camPosition"]["2"] = camPosition[2];

	setting[indexString]["camUp"]["0"] = camUp[0];
	setting[indexString]["camUp"]["1"] = camUp[1];
	setting[indexString]["camUp"]["2"] = camUp[2];

	setting[indexString]["camLookAt"]["0"] = camLookAt[0];
	setting[indexString]["camLookAt"]["1"] = camLookAt[1];
	setting[indexString]["camLookAt"]["2"] = camLookAt[2];

	setting[indexString]["camRight"]["0"] = camRight[0];
	setting[indexString]["camRight"]["1"] = camRight[1];
	setting[indexString]["camRight"]["2"] = camRight[2];

	setting[indexString]["near_plane"] = near_plane;
	setting[indexString]["far_plane"] = far_plane;
	setting[indexString]["fov"] = fov;

	
	if (isNew) {
		IDs.push_back(index);
	}
	
	setting["IDs"] = IDs;

	outputfile << std::setw(4) << setting << std::endl;
}


void Camera::loadPreset(int index) {
	std::string indexString = std::to_string(index);
	camPosition[0] = setting[indexString]["camPosition"]["0"];
	camPosition[1] = setting[indexString]["camPosition"]["1"];
	camPosition[2] = setting[indexString]["camPosition"]["2"];

	camUp[0] = setting[indexString]["camUp"]["0"];
	camUp[1] = setting[indexString]["camUp"]["1"];
	camUp[2] = setting[indexString]["camUp"]["2"];

	camLookAt[0] = setting[indexString]["camLookAt"]["0"];
	camLookAt[1] = setting[indexString]["camLookAt"]["1"];
	camLookAt[2] = setting[indexString]["camLookAt"]["2"];

	camRight[0] = setting[indexString]["camRight"]["0"];
	camRight[1] = setting[indexString]["camRight"]["1"];
	camRight[2] = setting[indexString]["camRight"]["2"];

	near_plane = setting[indexString]["near_plane"];
	far_plane = setting[indexString]["far_plane"];
	fov = setting[indexString]["fov"];
}


void Camera::deletePreset(int index) {
	std::string indexString = std::to_string(index);
	setting.erase(indexString);
}