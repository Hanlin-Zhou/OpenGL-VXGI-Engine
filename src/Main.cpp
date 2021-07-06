#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <shader.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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
bool debug_window = true;

glm::vec3 cameraPos;
glm::vec3 cameraTarget;
glm::vec3 cameraUp;
glm::vec3 cameraRight;

float near_plane = 0.1f;
float far_plane = 100.0f;
float fov = 90.0;
float scr_aspect = (float)INIT_WIDTH / (float)INIT_HEIGHT;
float sd_aspect = (float)SD_WIDTH / (float)SD_HEIGHT;

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
	
	// camera
	cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


	// can be turn into function
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("./data/uv.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);





	Light mylight= Light();

	///depth
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SD_WIDTH, SD_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// cube depth
	glm::mat4 perspectiveShadowProj = glm::perspective(glm::radians(fov), sd_aspect, near_plane, far_plane);
	unsigned int depthCubeFBO;
	glGenFramebuffers(1, &depthCubeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthCubeFBO);
	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SD_WIDTH, SD_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Shader shadowDepth("./shader/ShadowDepth.vert", "./shader/empty.frag");
	Shader cubeShadowDepth("./shader/cubeDepth.vert", "./shader/cubeDepth.frag", "./shader/cubeDepth.geom");
	Shader planeShadowShader("./shader/ShadowRender.vert", "./shader/ShadowRender.frag");
	Shader cubeShadowShader("./shader/CubeShadowRender.vert", "./shader/CubeShadowRender.frag");
	Shader DebugShader("./shader/debug.vert", "./shader/debug.frag");
	Model MyModel("./model/opengl_render_testing.obj");




	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
	ImGui::StyleColorsDark();

	while (!glfwWindowShouldClose(window)) {
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

			//depthMap
			glViewport(0, 0, SD_WIDTH, SD_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			shadowDepth.use();
			shadowDepth.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix), false);
			MyModel.Draw(shadowDepth);

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
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
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
			glBindTexture(GL_TEXTURE_2D, depthMap);
			renderQuad(0.3);
		}

		//imgui
		{
			ImGui::Begin("Setting");
			ImGui::SliderFloat3("lightPos", glm::value_ptr(mylight.position), 0.0f, 15.0f);
			ImGui::Checkbox("Rotate Mode", &center_rotate);
			ImGui::Checkbox("Poly Mode", &poly_mode);
			ImGui::Checkbox("Debug Window", &debug_window);
			ImGui::Checkbox("Cube Shadow", &cube_shadow_enabled);
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
