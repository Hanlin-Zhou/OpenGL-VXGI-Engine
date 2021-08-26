#include <ApplicationUI.h>


void ControlMenu(Application& application) {
	if (ImGui::BeginMenu("Control")) {
		ImGui::DragFloat("Walk Speed", &application.walkSensitivity, 0.005f, 0.01f, 0.4f);
		ImGui::DragFloat("Rotate Sensitivity", &application.rotateSensitivity, 0.005f, 0.1f, 2.0f);
		ImGui::EndMenu();
	}
}