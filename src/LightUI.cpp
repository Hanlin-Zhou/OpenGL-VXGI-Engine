#include <LightUI.h>

static bool settingWindow = false;

void LightMenu(PointLight & myPointLight, DirectionalLight &myDirectionalLight) {
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
			ImGui::SetNextWindowSize({ 250.0f, 240.0f }, ImGuiCond_Appearing);
			ImGui::Begin("Light Setting", &settingWindow, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::Text("Point Light:");
			ImGui::DragFloat3("Position##PL", glm::value_ptr(myPointLight.position), 0.005f);
			ImGui::DragFloat("Strength##PL", &myPointLight.strength, 30.0f, 0.0f, 10000.0f);
			ImGui::Separator();
			ImGui::Text("Directional Light:");
			ImGui::DragFloat3("Direction##DL", glm::value_ptr(myDirectionalLight.normal), 0.005f);
			ImGui::DragFloat("Strength##DL", &myDirectionalLight.strength, 0.03f, 0.0f, 100.0f);
			ImGui::End();
		}
	}
}