#include <CameraUI.h>

void CameraMenu(Camera& cam) {
	if (ImGui::BeginMenu("Camera")) {
		ImGui::Text(("Position: " + glm::to_string(cam.camPosition)).c_str());
		ImGui::DragFloat("FOV", &cam.fov, 0.03f, 60.0f, 120.0f);
		ImGui::DragFloat("Near Plane", &cam.near_plane, 0.03f, 0.01f, 10.0f);
		ImGui::DragFloat("Far Plane", &cam.far_plane, 0.03f, 50.0f, 150.0f);
		ImGui::Separator();
		int remove = -1;
		for (int i = 0; i < cam.IDs.size(); i++) {
			ImGui::PushID(i);
			ImGui::Text("Preset %d", i + 1);
			ImGui::SameLine(100);
			if (ImGui::Button("Load")) {
				cam.loadPreset(cam.IDs[i]);
			}
			ImGui::SameLine(200);
			if (ImGui::Button("Save")) {
				cam.savePreset(cam.IDs[i], false);
			}
			ImGui::SameLine(300);
			if (ImGui::Button("Delete")) {
				cam.deletePreset(cam.IDs[i]);
				remove = i;
			}
			ImGui::PopID();
			ImGui::Separator();
		}
		if (remove != -1) {
			cam.IDs.erase(cam.IDs.begin() + remove);
			remove = -1;
		}
		if (ImGui::Button("Save New Camera")) {
			if (cam.IDs.empty()) {
				cam.savePreset(0, true);
			}
			else {
				cam.savePreset(cam.IDs[cam.IDs.size() - 1] + 1, true);
			}
			
		}
		ImGui::EndMenu();
	}
}