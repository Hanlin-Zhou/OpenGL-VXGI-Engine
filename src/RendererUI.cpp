#include <RendererUI.h>

void InitRendererMenu(Renderer* renderer) {
	ImGui::Begin("Create Renderer");
	ImGui::Checkbox("Debug Window", &renderer->Debug);
	ImGui::Checkbox("4X MSAA", &renderer->MSAA);
	if (renderer->MSAA) {
		renderer->MSAASample = 4;
	}
	else {
		renderer->MSAASample = 1;
	}
	ImGui::Checkbox("PCSS", &renderer->PCSS);
	ImGui::Checkbox("SSAO", &renderer->SSAO);
	ImGui::Checkbox("SkyBox", &renderer->SkyBox);
	if (ImGui::Button("Create")) {
		renderer->state = 1;
	}
	ImGui::End();
}