#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
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

int Left_Mouse_down = 0;
int Right_Mouse_down = 0;
float rotate_sensi = 0.3;
float walk_sensi = 0.05;

bool center_rotate = false;
bool poly_mode = false;
bool cube_shadow_enabled = false;
bool debug_window = false;
bool show_light = true;
bool soft_shadow = false;

glm::vec3 cameraPos;
glm::vec3 cameraTarget;
glm::vec3 cameraUp;
glm::vec3 cameraRight;

float near_plane = 0.1f;
float far_plane = 50.0f;
float fov = 90.0;
float scr_aspect = (float)INIT_WIDTH / (float)INIT_HEIGHT;
float sd_aspect = (float)SD_WIDTH / (float)SD_HEIGHT;

float light_offset = 0.1;

void renderQuad(float size);


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
	glfwWindowHint(GLFW_SAMPLES, 4);

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
	
	// camera
	cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	// test teture
	unsigned int texture1 = loadTexture("./data/uv.jpg");

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
	Shader planeShadowSoftShader("./shader/SoftShadowRender.vert", "./shader/SoftShadowRender.frag");
	Shader cubeShadowShader("./shader/CubeShadowRender.vert", "./shader/CubeShadowRender.frag");
	Shader DebugShader("./shader/debug.vert", "./shader/debug.frag");
	Shader showLightShader("./shader/showLight.vert", "./shader/showLight.frag");

	Model MyModel("./model/opengl_render_testing.obj");
	Model LightModel("./model/light.obj");




	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
	ImGui::StyleColorsDark();

	double lastTime = glfwGetTime();
	int nbFrames = 0;
	float frameCostMS = 0.0;

	while (!glfwWindowShouldClose(window)) {
		// fps
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) {
			frameCostMS = 1000.0 / double(nbFrames);
			nbFrames = 0;
			lastTime = currentTime;
		}
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
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		if (poly_mode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		
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
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			cubeShadowDepth.use();
			cubeShadowDepth.setVec3("lightPos", glm::value_ptr(mylight.position));
			for (unsigned int i = 0; i < 6; ++i)
				cubeShadowDepth.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
			cubeShadowDepth.setFloat("far_plane", far_plane);
			MyModel.Draw(cubeShadowDepth);

			//3d model
			glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cubeShadowShader.use();
			cubeShadowShader.setVec3("lightPos", glm::value_ptr(mylight.position));
			cubeShadowShader.setFloat("far_plane", far_plane);
			cubeShadowShader.setMat4("proj", glm::value_ptr(proj_mat), false);
			cubeShadowShader.setMat4("view", glm::value_ptr(view_mat), false);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
			MyModel.Draw(cubeShadowShader);

		}
		else {
			// setup
			glm::mat4 lightView = glm::lookAt(mylight.position,
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;

			//soft Shadow
			glm::mat4 lightSpaceMatrixArray[6];
			if (soft_shadow) {
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

			if (soft_shadow) {
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
			glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			planeShadowShader.use();
			planeShadowShader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix), false);
			planeShadowShader.setVec3("lightPos", glm::value_ptr(mylight.position));
			planeShadowShader.setMat4("proj", glm::value_ptr(proj_mat), false);
			planeShadowShader.setMat4("view", glm::value_ptr(view_mat), false);
			planeShadowShader.setBool("soft_shadow", soft_shadow);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			if (soft_shadow) {
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
			glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			DebugShader.use();
			DebugShader.setInt("depthMap", 0);
			DebugShader.setFloat("near_plane", near_plane);
			DebugShader.setFloat("far_plane", far_plane);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMaps[2]);
			renderQuad(0.3);
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
			ImGui::SliderFloat3("lightPos", glm::value_ptr(mylight.position), 0.0f, 15.0f);
			ImGui::Checkbox("Rotate Mode", &center_rotate);
			ImGui::Checkbox("Poly Mode", &poly_mode);
			ImGui::Checkbox("Debug Window", &debug_window);
			ImGui::Checkbox("Cube Shadow", &cube_shadow_enabled);
			ImGui::Checkbox("Soft Shadow", &soft_shadow);
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
			ImGui::Text("%f ms/frame", frameCostMS);
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

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(float size)
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			size,  1.0f, 0.0f, 0.0f, 1.0f,
			size, size, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, size, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
