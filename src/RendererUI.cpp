#include <RendererUI.h>
#include <tinyfiledialogs.h>
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


void DebugMenu(Renderer &renderer) {
	if (ImGui::BeginMenu("Debug")) {
		ImGui::Checkbox("Show Debug Overlay", &renderer.ShowDebug);
		ImGui::EndMenu();
	}
}

void SVOGIDebugMenu(Renderer& renderer) {
	if (ImGui::BeginMenu("GI Debug")) {
		ImGui::DragFloat("Offset Factor", &renderer.GI_offsetFactor, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("Specular Aperture", &renderer.GI_SpecularAperture, 0.02f, 0.02f, 1.5f);
		ImGui::DragFloat("Diffuse Aperture", &renderer.GI_DiffuseAperture, 0.02f, 0.02f, 1.5f);
		ImGui::DragFloat("Occulsion Aperture", &renderer.GI_OcculsionAperture, 0.01f, 0.01f, 1.5f);
		ImGui::DragFloat("Step Size", &renderer.GI_stepSize, 0.1f, 0.02f, 10.0f);
		ImGui::EndMenu();
	}
}