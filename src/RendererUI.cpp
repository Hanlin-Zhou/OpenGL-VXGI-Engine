#include <RendererUI.h>
#include <stdio.h>


void RendererInitSetting(Renderer &renderer) {
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, { 0.5f, 0.5f });
	ImGui::SetNextWindowSize({ 500.0f, 500.0f });
	ImGui::Begin("Renderer Setting", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::Checkbox("4X MSAA", &renderer.MSAA);
	if (renderer.MSAA) {
		renderer.MSAASample = 4;
	}
	else {
		renderer.MSAASample = 1;
	}
	ImGui::Checkbox("SVOGI", &renderer.SVOGI);
	renderer.PCSS = !renderer.SVOGI;
	ImGui::SameLine(150);
	ImGui::Checkbox("PCSS Direct Lighting", &renderer.PCSS);
	renderer.SVOGI = !renderer.PCSS;
	ImGui::Checkbox("SSAO", &renderer.SSAO);
	ImGui::Checkbox("SkyBox", &renderer.SkyBox);
	ImGui::Checkbox("OpenGl Debug Output", &renderer.GLDebugOutput);
	// Model
	if (ImGui::Button("Load OBJ Model")) {
		tinyfd_assumeGraphicDisplay = 1;
		char const* lFilterPatterns[1] = { "*.obj" };
		renderer.modelPath = tinyfd_openFileDialog("Open Your OBJ Model", "./model/", 1, lFilterPatterns, "OBJ File", 0);
	}
	ImGui::SameLine(150);
	ImGui::Text("Model selected : %s", renderer.modelPath.c_str());
	// HDRI
	if (ImGui::Button("Load HDRI")) {
		if (renderer.SkyBox) {
			tinyfd_assumeGraphicDisplay = 1;
			char const* lFilterPatterns[1] = { "*.hdr" };
			renderer.HDRIPath = tinyfd_openFileDialog("Open Your HDRI", "./hdri/", 1, lFilterPatterns, "HDR File", 0);
		}
		else {
			
		}
		
	}
	ImGui::SameLine(150);
	if (renderer.SkyBox) {
		ImGui::Text("HDRI selected : %s", renderer.HDRIPath.c_str());
	}
	else {
		ImGui::Text("HDRI SkyBox not enabled");
	}
	

	if (ImGui::Button("Create")) {
		renderer.state = 1;
	}
	if (ImGui::Button("Save Setting")) {
		ImGui::SameLine(150);
		ImGui::Text("Saved!");
		renderer.SaveInit();
	}
	ImGui::End();

}


void LoadMenu(Renderer &renderer) {
	// ""Load" Menu
	if (ImGui::BeginMenu("Load")) {

		if (ImGui::MenuItem("New Model", NULL)) {
			tinyfd_assumeGraphicDisplay = 1;
			char const* lFilterPatterns[1] = { "*.obj" };
			renderer.modelPath = tinyfd_openFileDialog("Open Your OBJ Model", "./model/", 1, lFilterPatterns, "OBJ File", 0);
			renderer.loadModel();
			if (renderer.SVOGI) {
				renderer.voxelizeStatic();
			}
		}

		if (ImGui::MenuItem("New HDRI", NULL)) {
			tinyfd_assumeGraphicDisplay = 1;
			char const* lFilterPatterns[1] = { "*.hdr" };
			renderer.HDRIPath = tinyfd_openFileDialog("Open Your HDRI", "./hdri/", 1, lFilterPatterns, "HDR File", 0);
			renderer.loadHDRI(renderer.SkyBox);
		}
		ImGui::EndMenu();
	}
}


void RendererMenu(Renderer &renderer) {
	if (ImGui::BeginMenu("Renderer")) {
		ImGui::Checkbox("Show Texture", &renderer.ShowTexture);
		ImGui::Checkbox("Show Normal", &renderer.ShowNormal);
		ImGui::Separator();
		ImGui::Checkbox("Peter Pan", &renderer.PeterPan);
		ImGui::Checkbox("HDR", &renderer.HDR);
		ImGui::Checkbox("Blur Shadow", &renderer.ShadowBluring);
		ImGui::EndMenu();
	}
}


void ModelMenu(Renderer& renderer) {
	if (ImGui::BeginMenu("Model")) {
		int remove = -1;
		for (int i = 0; i < renderer.modelList.size(); i++)
		{
			ImGui::Text(renderer.modelList[i].name.c_str());
			ImGui::PushID(renderer.modelList[i].name.c_str());
			if (renderer.modelList[i].isStatic) {
				ImGui::Text("This Object is Static");
			}
			else {
				ImGui::DragFloat("Scale", &renderer.modelList[i].scale, 0.01f, 0.1f, 20.0f);
				ImGui::DragFloat3("Position", glm::value_ptr(renderer.modelList[i].offset), 0.005f);
			}
			if (ImGui::Button("Remove")) {
				remove = i;
			}
			ImGui::PopID();
			ImGui::Separator();
		}
		if (ImGui::Button("Add")) {
			tinyfd_assumeGraphicDisplay = 1;
			char const* lFilterPatterns[1] = { "*.obj" };
			renderer.modelPath = tinyfd_openFileDialog("Open Your OBJ Model", "./model/", 1, lFilterPatterns, "OBJ File", 0);
			renderer.addModel();
		}
		if (remove != -1) {
			renderer.modelList.erase(renderer.modelList.begin() + remove);
			remove = -1;
		}
		ImGui::EndMenu();
	}
}


void DebugMenu(Renderer &renderer) {
	if (ImGui::BeginMenu("Debug")) {
		ImGui::Text("Debug Window Size");
		ImGui::DragFloat("##Debug Window Size", &renderer.DebugWindowSize, 0.02f, -1.0f, 0.9f);
		ImGui::Checkbox("Show Debug Overlay", &renderer.ShowDebug);
		
		static std::string current_item = "Select Debug View";
		static std::string voxel_current_item = "Select Voxel";
		if (ImGui::BeginCombo("##combo", current_item.c_str())) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < renderer.DebugViewsName.size(); n++)
			{
				if (renderer.DebugViewsID[n] != 0) {
					bool is_selected = (current_item.compare(renderer.DebugViewsName[n]) == 0);
					if (ImGui::Selectable(renderer.DebugViewsName[n].c_str(), is_selected)) {
						current_item = renderer.DebugViewsName[n];
						renderer.CurrentDebugView = renderer.DebugViewsID[n];
					}
					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			}
			ImGui::EndCombo();
		}
		if (renderer.CurrentDebugView == renderer.VoxelVisOut) {
			if (ImGui::BeginCombo("##combo3D", voxel_current_item.c_str()))
			{
				for (int n = 0; n < renderer.VoxelDebugViewsName.size(); n++)
				{
					if (renderer.VoxelDebugViewsID[n] != 0) {
						bool is_selected = (voxel_current_item.compare(renderer.VoxelDebugViewsName[n]) == 0);
						if (ImGui::Selectable(renderer.VoxelDebugViewsName[n].c_str(), is_selected)) {
							voxel_current_item = renderer.VoxelDebugViewsName[n];
							renderer.VoxelCurrentDebugView = renderer.VoxelDebugViewsID[n];
						}
						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
				}
				ImGui::EndCombo();
			}
		}
		ImGui::EndMenu();
	}
}

void SVOGIDebugMenu(Renderer& renderer) {
	if (ImGui::BeginMenu("GI Debug")) {
		
		ImGui::DragFloat("Specular Aperture", &renderer.GI_SpecularAperture, 0.02f, 0.02f, 1.5f);
		ImGui::DragFloat("Specular Offset", &renderer.GI_SpecularOffsetFactor, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("Specular Max T", &renderer.GI_SpecularMaxT, 0.01f, 0.01f, 2.0f);
		ImGui::Separator();
		ImGui::DragFloat("Diffuse Aperture", &renderer.GI_DiffuseAperture, 0.02f, 0.02f, 1.5f);
		ImGui::DragFloat("Diffuse Cone Angle", &renderer.GI_DiffuseConeAngleMix, 0.01f, 0.01f, 1.0f);
		ImGui::DragFloat("Diffuse Offset", &renderer.GI_DiffuseOffsetFactor, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("Diffuse Max T", &renderer.GI_DiffuseMaxT, 0.01f, 0.01f, 2.0f);
		ImGui::Separator();
		ImGui::DragFloat("Occulsion Aperture", &renderer.GI_OcculsionAperture, 0.01f, 0.01f, 1.5f);
		ImGui::DragFloat("Occulsion Offset", &renderer.GI_OcclusionOffsetFactor, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("DirectionalMaxT", &renderer.GI_DirectionalMaxT, 0.1f, 0.1f, 2.0f);
		ImGui::Separator();
		ImGui::DragFloat("Step Size", &renderer.GI_stepSize, 0.1f, 0.02f, 10.0f);
		
		ImGui::EndMenu();
	}
}