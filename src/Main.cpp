#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <random>
#include <shader.h>
#define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
#include <ApplicationUtility.h>
#include "Model.h"
#include <Light.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>

#define INIT_WIDTH 1000
#define INIT_HEIGHT 1000
#define SD_WIDTH 2000
#define SD_HEIGHT 2000
#define MSAA_SAMPLES 4

int Left_Mouse_down = 0;
int Right_Mouse_down = 0;
float rotate_sensi = 0.3;
float walk_sensi = 0.05;

bool center_rotate = false;
bool poly_mode = false;
bool cube_shadow_enabled = true;
bool debug_window = false;
bool show_light = true;
bool multiCam = false;
bool peter_pan = true;
bool gamma_correction = false;
bool HDR = false;
bool deferred_shading = false;

glm::vec3 cameraPos;
glm::vec3 cameraTarget;
glm::vec3 cameraUp;
glm::vec3 cameraRight;

float near_plane = 0.1f;
float far_plane = 100.0f;
float fov = 90.0;
float scr_aspect = (float)INIT_WIDTH / (float)INIT_HEIGHT;
float sd_aspect = (float)SD_WIDTH / (float)SD_HEIGHT;

float light_offset = 0.1;


void processCamWalkInput(GLFWwindow* window) {
	glm::vec3 front_direction = glm::normalize(cameraTarget - cameraPos);
	// front_direction[1] = 0.0;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += walk_sensi * front_direction;
		cameraTarget += walk_sensi * front_direction;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos -= walk_sensi * front_direction;
		cameraTarget -= walk_sensi * front_direction;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPos += walk_sensi * cameraRight;
		cameraTarget += walk_sensi * cameraRight;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos -= walk_sensi * cameraRight;
		cameraTarget -= walk_sensi * cameraRight;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		cameraPos += walk_sensi * cameraUp;
		cameraTarget += walk_sensi * cameraUp;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		cameraPos -= walk_sensi * cameraUp;
		cameraTarget -= walk_sensi * cameraUp;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		center_rotate = !center_rotate;
		std::cout << center_rotate << std::endl;
	}
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_SAMPLES, 4);
	

	//	//init error check
	GLFWwindow* window = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT, "Hello Atelier", NULL, NULL);
	if (window == NULL) {
		std::cout << "Creating window Failed\n" << std::endl;
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse) {
			if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
				Left_Mouse_down = 1;
			}
			if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) {
				Right_Mouse_down = 1;
			}
			if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
				Left_Mouse_down = 0;
			}
			if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE) {
				Right_Mouse_down = 0;
			}
		}
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow * window, double xpos, double ypos) {
		static double CursorLastX = xpos;
		static double CursorLastY = ypos;
		ImGuiIO& io = ImGui::GetIO();
		if (Left_Mouse_down && !io.WantCaptureMouse) {
			if (!center_rotate) {
				glm::mat4 rot_mat = glm::mat4(1.0f);
				float hor_r = -glm::radians(rotate_sensi * (xpos - CursorLastX) / INIT_WIDTH) * 500.0;
				float ver_r = glm::radians(rotate_sensi * (ypos - CursorLastY) / INIT_HEIGHT) * 400.0;
				rot_mat = glm::rotate(rot_mat, hor_r, cameraUp);
				rot_mat = glm::rotate(rot_mat, ver_r, cameraRight);
				glm::vec4 temp_cameraPos = rot_mat * glm::vec4(cameraPos.x - cameraTarget.x, cameraPos.y - cameraTarget.y, cameraPos.z - cameraTarget.z, 1.0);
				cameraPos = glm::vec3(cameraTarget.x + temp_cameraPos.x / temp_cameraPos[3], cameraTarget.y + temp_cameraPos.y / temp_cameraPos[3], cameraTarget.z + temp_cameraPos.z / temp_cameraPos[3]);
			}
			else {
			}
		}
		CursorLastX = xpos;
		CursorLastY = ypos;
	});

	                   
	// get function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 0;
	}
	

	// ----------------------------------------------------------------------------------------------------- //
	// ----------------------------------------------------------------------------------------------------- //
	// ----------------------------------------------------------------------------------------------------- //
	glEnable(GL_MULTISAMPLE);
	// camera
	cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	// test texture
	unsigned int texture1 = loadTexturePath("./data/uv.jpg");	

	// G Buffer
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	unsigned int gPosition = bindColorBuffer(gBuffer, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT0, GL_RGBA32F, MSAA_SAMPLES, GL_LINEAR);
	unsigned int gNormal = bindColorBuffer(gBuffer, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT1, GL_RGBA32F, MSAA_SAMPLES, GL_LINEAR);
	unsigned int gAlbedoSpec = bindColorBuffer(gBuffer, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT2, GL_RGBA32F, MSAA_SAMPLES, GL_LINEAR);
	attachRBOToBuffer(gBuffer, INIT_WIDTH, INIT_HEIGHT, GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT, MSAA_SAMPLES);

	// Down sampled G Buffer
	unsigned int ds_gBuffer;
	glGenFramebuffers(1, &ds_gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ds_gBuffer);
	unsigned int ds_attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	glDrawBuffers(4, ds_attachments);

	unsigned int ds_gPosition = bindColorBuffer(ds_gBuffer, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT0, GL_RGBA32F, 1, GL_LINEAR);
	unsigned int ds_gNormal = bindColorBuffer(ds_gBuffer, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT1, GL_RGBA32F, 1, GL_LINEAR);
	unsigned int ds_gAlbedoSpec = bindColorBuffer(ds_gBuffer, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT2, GL_RGBA32F, 1, GL_LINEAR);
	unsigned int ds_gViewPos = bindColorBuffer(ds_gBuffer, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT3, GL_RGBA32F, 1, GL_LINEAR);

	// Post Processing 
	// SSAO
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); 
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / 64.0;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}

	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	unsigned int ssaoFBO;
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	unsigned int ssao = bindColorBuffer(ssaoFBO, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT0, GL_RED, 1, GL_LINEAR);

	// Post Processing Buffer
	unsigned int PostProcessingFBO;
	glGenFramebuffers(1, &PostProcessingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, PostProcessingFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	unsigned int blurOut = bindColorBuffer(PostProcessingFBO, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_ATTACHMENT0, GL_RGBA32F, 1, GL_LINEAR);


	// light
	Light mylight= Light();

	// ---depth---
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	// default depth map
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	unsigned int depthMap = bindDepthMap(depthMapFBO, SD_WIDTH, SD_HEIGHT);

	// soft shadow depth maps
	unsigned int depthMapFBOs[6];
	glGenFramebuffers(6, depthMapFBOs);
	unsigned int depthMaps[6];
	for (unsigned int i = 0; i < 6; i++)
	{
		depthMaps[i] = bindDepthMap(depthMapFBOs[i], SD_WIDTH, SD_HEIGHT);
	}

	// ---cube depth---
	glm::mat4 perspectiveShadowProj = glm::perspective(glm::radians(fov), sd_aspect, near_plane, far_plane);

	// default cube depth map
	unsigned int depthCubeFBO;
	glGenFramebuffers(1, &depthCubeFBO);
	unsigned int depthCubemap = bindCubeDepthMap(depthCubeFBO, SD_WIDTH, SD_HEIGHT);


	// light offsets for soft shadow
	glm::vec3 offsets[6];
	offsets[0] = glm::vec3(light_offset, 0.0, 0.0);
	offsets[1] = glm::vec3(-light_offset, 0.0, 0.0);
	offsets[2] = glm::vec3(0.0, light_offset, 0.0);
	offsets[3] = glm::vec3(0.0, -light_offset, 0.0);
	offsets[4] = glm::vec3(0.0, 0.0, light_offset);
	offsets[5] = glm::vec3(0.0, 0.0, -light_offset);


	Shader shadowDepth("./shader/ShadowDepth.vert", "./shader/empty.frag");
	Shader cubeShadowDepth("./shader/cubeDepth.vert", "./shader/cubeDepth.frag", "./shader/cubeDepth.geom");
	Shader planeShadowShader("./shader/ShadowRender.vert", "./shader/ShadowRender.frag");
	// Shader planeShadowSoftShader("./shader/SoftShadowRender.vert", "./shader/SoftShadowRender.frag");
	Shader cubeShadowShader("./shader/CubeShadowRender.vert", "./shader/CubePCSS.frag");
	Shader DebugShader("./shader/quad.vert", "./shader/debug.frag");
	Shader showLightShader("./shader/showLight.vert", "./shader/showLight.frag");
	Shader gBufferGeoPass("./shader/CubeShadowRender.vert", "./shader/gBuffer.frag");
	Shader gBufferLightPass("./shader/deferred_shading.vert", "./shader/MSgCubePCSS.frag");
	Shader SSAO_Shader("./shader/deferred_shading.vert", "./shader/SSAO.frag");
	Shader DownSampleShader("./shader/deferred_shading.vert", "./shader/downSample.frag");
	Shader BlurShader("./shader/deferred_shading.vert", "./shader/Blur.frag");

	Model MyModel("./model/sponza/sponza.obj"); 
	// Model MyModel("./model/opengl_render_testing.obj");
	// Model MyModel("./model/softshadowtest.obj");
	Model LightModel("./model/light.obj");



	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
	ImGui::StyleColorsDark();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	

	while (!glfwWindowShouldClose(window)) {
		// start
		processCamWalkInput(window);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		cameraRight = glm::normalize(glm::cross(glm::normalize(cameraPos - cameraTarget), cameraUp));
		glm::mat4 view_mat = glm::lookAt(cameraPos, cameraTarget, cameraUp);
		glm::mat4 proj_mat = glm::perspective(glm::radians(fov), scr_aspect, near_plane, far_plane);
		//setting
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (poly_mode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (cube_shadow_enabled) {
			// setup
			std::vector<glm::mat4> shadowTransforms;
			shadowTransforms.push_back(perspectiveShadowProj *
				glm::lookAt(mylight.position, mylight.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
			shadowTransforms.push_back(perspectiveShadowProj *
				glm::lookAt(mylight.position, mylight.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
			shadowTransforms.push_back(perspectiveShadowProj *
				glm::lookAt(mylight.position, mylight.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
			shadowTransforms.push_back(perspectiveShadowProj *
				glm::lookAt(mylight.position, mylight.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
			shadowTransforms.push_back(perspectiveShadowProj *
				glm::lookAt(mylight.position, mylight.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
			shadowTransforms.push_back(perspectiveShadowProj *
				glm::lookAt(mylight.position, mylight.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

			//depthMap
			glViewport(0, 0, SD_WIDTH, SD_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthCubeFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			cubeShadowDepth.use();
			cubeShadowDepth.setVec3("lightPos", glm::value_ptr(mylight.position));
			for (unsigned int i = 0; i < 6; ++i)
				cubeShadowDepth.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
			cubeShadowDepth.setFloat("far_plane", far_plane);
			if (peter_pan) {
				glCullFace(GL_FRONT);
				MyModel.Draw(cubeShadowDepth);
				glCullFace(GL_BACK);
			}
			else {
				MyModel.Draw(cubeShadowDepth);
			}

			//3d model

			// gBuffer Geometry Pass
			glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			gBufferGeoPass.use();
			gBufferGeoPass.setMat4("proj", glm::value_ptr(proj_mat), false);
			gBufferGeoPass.setMat4("view", glm::value_ptr(view_mat), false);
			MyModel.Draw(gBufferGeoPass);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// Down sample
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ds_gBuffer);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glBlitFramebuffer(0, 0, INIT_WIDTH, INIT_HEIGHT, 0, 0, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, INIT_WIDTH, INIT_HEIGHT, 0, 0, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			glDrawBuffer(GL_COLOR_ATTACHMENT2);
			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glBlitFramebuffer(0, 0, INIT_WIDTH, INIT_HEIGHT, 0, 0, INIT_WIDTH, INIT_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			DownSampleShader.use();
			glDrawBuffer(GL_COLOR_ATTACHMENT3);
			DownSampleShader.setMat4("view", glm::value_ptr(view_mat), false);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ds_gPosition);
			SSAO_Shader.setInt("ds_gPosition", 0);
			unsigned int DS_quad = initQuad(-1.0, 0.7);
			renderQuad(DS_quad);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// SSAO
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			SSAO_Shader.use();
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (unsigned int i = 0; i < 64; i++){
				SSAO_Shader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
			}
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ds_gPosition);
			SSAO_Shader.setInt("gPosition", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, ds_gNormal);
			SSAO_Shader.setInt("gNormal", 1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);
			SSAO_Shader.setInt("texNoise", 2);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ds_gViewPos);
			SSAO_Shader.setInt("gViewPos", 3);
			SSAO_Shader.setMat4("proj", glm::value_ptr(proj_mat), false);
			SSAO_Shader.setMat4("view", glm::value_ptr(view_mat), false);
			unsigned int SSAO_quad = initQuad(-1.0, 0.7);
			renderQuad(SSAO_quad);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// Post Processing Blur
			glBindFramebuffer(GL_FRAMEBUFFER, PostProcessingFBO);
			BlurShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ssao);
			BlurShader.setInt("ssaoInput", 0);
			BlurShader.setInt("sampleSize", 2);
			unsigned int blur_quad = initQuad(-1.0, 0.7);
			renderQuad(blur_quad);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// Lighting Pass
			glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			gBufferLightPass.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gPosition);
			gBufferLightPass.setInt("gPosition", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gNormal);
			gBufferLightPass.setInt("gNormal", 1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gAlbedoSpec);
			gBufferLightPass.setInt("gAlbedoSpec", 2);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
			gBufferLightPass.setInt("depthCubemap", 3);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, blurOut);
			gBufferLightPass.setInt("SSAO", 4);
			gBufferLightPass.setVec3("lightPos", glm::value_ptr(mylight.position));
			gBufferLightPass.setVec3("viewPos", glm::value_ptr(cameraPos));
			gBufferLightPass.setFloat("far_plane", far_plane);
			gBufferLightPass.setBool("HDR", HDR);
			gBufferLightPass.setInt("MSAA_Sample", MSAA_SAMPLES);
			unsigned int deferred_render_window = initQuad(-1.0, 0.7);
			glEnable(GL_FRAMEBUFFER_SRGB);
			renderQuad(deferred_render_window);
			glDisable(GL_FRAMEBUFFER_SRGB);
		}
		else {
			// setup
			glm::mat4 lightView = glm::lookAt(mylight.position,
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;

			//soft Shadow
			glm::mat4 lightSpaceMatrixArray[6];
			if (multiCam) {
				for (unsigned int i = 0; i < 6; i++)
				{
					glm::mat4 lightView = glm::lookAt(mylight.position + offsets[i],
						glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(0.0f, 1.0f, 0.0f));
					lightSpaceMatrixArray[i] = lightProjection * lightView;
				}
			}
			

			//depthMap
			glViewport(0, 0, SD_WIDTH, SD_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			shadowDepth.use();
			shadowDepth.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix), false);
			MyModel.Draw(shadowDepth);

			if (multiCam) {
				for (unsigned int i = 0; i < 6; i++)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[i]);
					glEnable(GL_DEPTH_TEST);
					glClear(GL_DEPTH_BUFFER_BIT);
					shadowDepth.use();
					shadowDepth.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrixArray[i]), false);
					MyModel.Draw(shadowDepth);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}
			}

			//3d model
			planeShadowShader.use();
			planeShadowShader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix), false);
			planeShadowShader.setVec3("lightPos", glm::value_ptr(mylight.position));
			planeShadowShader.setVec3("viewPos", glm::value_ptr(cameraPos));
			planeShadowShader.setMat4("proj", glm::value_ptr(proj_mat), false);
			planeShadowShader.setMat4("view", glm::value_ptr(view_mat), false);
			planeShadowShader.setBool("multiCam", multiCam);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			// planeShadowShader.setInt("depthMap", 0);
			if (multiCam) {
				for (unsigned int i = 0; i < 6; i++) {
					planeShadowShader.setMat4("LSM[" + std::to_string(i) + "]", lightSpaceMatrixArray[i]);
					glActiveTexture(GL_TEXTURE0 + i + 1);
					glBindTexture(GL_TEXTURE_2D, depthMaps[i]);
					planeShadowShader.setInt("depthMaps[" + std::to_string(i) + "]", 1 + i);
				}
			}
			
			MyModel.Draw(planeShadowShader);
		}
		
		// top right debug 
		if (debug_window) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			DebugShader.use();
			DebugShader.setInt("depthMap", 0);
			DebugShader.setFloat("near_plane", near_plane);
			DebugShader.setFloat("far_plane", far_plane);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, blurOut);
			DebugShader.setInt("depthMap", 0);
			unsigned int debugWindow = initQuad(-1.0, 0.5);
			renderQuad(debugWindow);
		}
		if (show_light) {
			glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, mylight.position);
			showLightShader.use();
			showLightShader.setMat4("model", glm::value_ptr(model), false);
			showLightShader.setMat4("view", glm::value_ptr(view_mat), false);
			showLightShader.setMat4("proj", glm::value_ptr(proj_mat), false);
			LightModel.Draw(showLightShader);
		}
		//imgui
		{
			ImGui::Begin("Setting");
			ImGui::SliderFloat3("lightPos", glm::value_ptr(mylight.position), -30.0f, 30.0f);
			ImGui::Checkbox("Gamma Correction", &gamma_correction);
			ImGui::Checkbox("Rotate Mode", &center_rotate);
			ImGui::Checkbox("Poly Mode", &poly_mode);
			ImGui::Checkbox("Debug Window", &debug_window);
			ImGui::Checkbox("Cube Shadow", &cube_shadow_enabled);
			ImGui::Checkbox("Soft Shadow", &multiCam);
			ImGui::Checkbox("Peter Pan", &peter_pan);
			ImGui::Checkbox("HDR", &HDR);
			ImGui::SliderFloat("rotate speed", &rotate_sensi, 0.0f, 1.0f);
			ImGui::SliderFloat("walk speed", &walk_sensi, 0.0f, 0.1f);

			if (ImGui::Button("Reset Cam")) {
				cameraPos = glm::vec3(0.0f, 0.0f, -20.0f);
				cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
				cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
				cameraRight = glm::normalize(glm::cross(cameraUp, glm::normalize(cameraPos - cameraTarget)));
			}
			if (ImGui::Button("Reset Target")) {
				cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
			}
			if (ImGui::Button("Print Debug")) {
				std::cout << glm::to_string(view_mat) << std::endl;
				std::cout << glm::to_string(cameraPos) << std::endl;
				// std::cout << "cam_pos: \n" << cam_pos << "\nview_mat: \n" << view_mat.matrix() << "\trans_cam: \n" << axis <<"\n ========================"<< std::endl;
			}
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			GLint nTotalMemoryInKB = 0;
			glGetIntegerv(0x9048,
				&nTotalMemoryInKB);

			GLint nCurAvailMemoryInKB = 0;
			glGetIntegerv(0x9049,
				&nCurAvailMemoryInKB);
			ImGui::Text("GPU Usage %.1f / %.1f", (float)nCurAvailMemoryInKB / 1024.0, (float)nTotalMemoryInKB / 1024.0);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


