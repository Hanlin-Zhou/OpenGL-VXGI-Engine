#include <LightUI.h>

static bool settingWindow = false;

void LightMenu(Light &mylight) {
	if (true) {
		if (ImGui::BeginMenu("Light")) {
			if (ImGui::MenuItem("Setting", NULL)) {
				settingWindow = true;
			}

			ImGui::EndMenu();
		}

		// -------------------------------------------------------------------------------------------------------------- //
		if (settingWindow) {
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowSize()[0], 22.0), ImGuiCond_Appearing, { 1.0f, 0.0f });
			ImGui::SetNextWindowSize({ 250.0f, 80.0f }, ImGuiCond_Appearing);
			ImGui::Begin("Light Setting", &settingWindow, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::DragFloat3("Position", glm::value_ptr(mylight.position), 0.005f);
			ImGui::End();
		}
	}
}