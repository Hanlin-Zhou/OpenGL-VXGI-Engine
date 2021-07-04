#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <eigen/Core>
#include <Eigen/Geometry>
#include <shader.h>
#include <setup.h>
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
#define SD_WIDTH 1000
#define SD_HEIGHT 1000
int Left_Mouse_down = 0;
int Right_Mouse_down = 0;
bool center_rotate = false;
float rotate_sensi = 0.3;
float walk_sensi = 0.05;
bool poly_mode = false;

Eigen::Affine3f view_mat;
Eigen::Vector3f cam_pos;
Eigen::Vector3f cam_right;
Eigen::Vector3f cam_up;
Eigen::Vector3f axis;
Eigen::Matrix4f proj_mat;
void renderQuad();
void processCamWalkInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		view_mat.pretranslate(Eigen::Vector3f(0, 0, walk_sensi));
		cam_pos(2) += walk_sensi;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		view_mat.pretranslate(Eigen::Vector3f(0, 0, -walk_sensi));
		cam_pos(2) -= walk_sensi;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		view_mat.pretranslate(Eigen::Vector3f(walk_sensi, 0, 0));
		cam_pos(0) += walk_sensi;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		view_mat.pretranslate(Eigen::Vector3f(-walk_sensi, 0, 0));
		cam_pos(0) -= walk_sensi;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		view_mat.pretranslate(Eigen::Vector3f(0, -walk_sensi, 0));
		cam_pos(1) -= walk_sensi;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		view_mat.pretranslate(Eigen::Vector3f(0, walk_sensi, 0));
		cam_pos(1) += walk_sensi;
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
				//std::cout << 1 << std::endl;
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
			//std::cout << "X:" << xpos << "\nY: " << ypos << "\nLMB: " << Left_Mouse_down << std::endl;
			
			if (!center_rotate) {
				view_mat.rotate(Eigen::AngleAxisf(-rotate_sensi * M_PI * (xpos - CursorLastX) / INIT_WIDTH, -Eigen::Vector3f::UnitY()));
			}
			else {
				view_mat.prerotate(Eigen::AngleAxisf(rotate_sensi * M_PI * (xpos - CursorLastX) / INIT_WIDTH, -Eigen::Vector3f::UnitY()));
				view_mat.prerotate(Eigen::AngleAxisf(rotate_sensi * M_PI * (ypos - CursorLastY) / INIT_HEIGHT, -Eigen::Vector3f::UnitX()));
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
	
	init_view_proj(view_mat, proj_mat, cam_pos, 35.0, INIT_WIDTH, INIT_HEIGHT);
	

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







	///depth
	Light mylight= Light();
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

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "asdhasdasdasdasda" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//test
	unsigned int lightcamfbo;
	glGenFramebuffers(1, &lightcamfbo);
	glBindFramebuffer(GL_FRAMEBUFFER, lightcamfbo);
	unsigned int lightcam;
	glGenTextures(1, &lightcam);
	glBindTexture(GL_TEXTURE_2D, lightcam);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SD_WIDTH, SD_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightcam, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "asdhasdasdasdasda" << std::endl;
	}
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SD_WIDTH, SD_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	Shader shadowDepth("./shader/ShadowDepth.vert", "./shader/empty.frag");
	Shader ourShader("./shader/ShadowRender.vert", "./shader/ShadowRender.frag");
	Shader testShader("./shader/ShadowRender.vert", "./shader/ShadowRender.frag");
	Shader DebugShader("./shader/debug.vert", "./shader/debug.frag");
	Model MyModel("./model/test.obj");




	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
	ImGui::StyleColorsDark();

	while (!glfwWindowShouldClose(window)) {
		processCamWalkInput(window);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// param
		float near_plane = 0.1f, far_plane = 100.0f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(mylight.position,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		
		//setting
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glViewport(0, 0, SD_WIDTH, SD_HEIGHT);

		//depthMap
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		shadowDepth.use();
		shadowDepth.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix), false);
		MyModel.Draw(shadowDepth);
		//3d model
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ourShader.use();
		ourShader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix), false);
		ourShader.setVec3("lightPos", glm::value_ptr(mylight.position));
		ourShader.setMat4("proj", proj_mat.data(), false);
		ourShader.setMat4("view", view_mat.matrix().data(), false);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		// ourShader.setInt("shadowMap", 0);
		if (poly_mode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		MyModel.Draw(ourShader);
		//test
		
		/*glBindFramebuffer(GL_FRAMEBUFFER, lightcamfbo);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glClearColor(0.3f, 0.4f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		testShader.use();
		testShader.setVec3("lightPos", glm::value_ptr(mylight.position));
		testShader.setMat4("proj", glm::value_ptr(lightProjection), false);
		testShader.setMat4("view", glm::value_ptr(lightView), false);
		MyModel.Draw(testShader);*/
		//top right quad
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		DebugShader.use();
		DebugShader.setInt("depthMap", 0);
		DebugShader.setFloat("near_plane", near_plane);
		DebugShader.setFloat("far_plane", far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		renderQuad();

		//imgui
		{
			ImGui::Begin("Setting");
			ImGui::SliderFloat3("lightPos", glm::value_ptr(mylight.position), 0.0f, 8.0f);
			ImGui::Text("This is some useful text.");
			ImGui::Checkbox("Rotate Mode", &center_rotate);
			ImGui::Checkbox("Poly Mode", &poly_mode);
			ImGui::SliderFloat("rotate speed", &rotate_sensi, 0.0f, 1.0f);
			ImGui::SliderFloat("walk speed", &walk_sensi, 0.0f, 0.1f);

			if (ImGui::Button("Reset Cam")) {
				view_mat = Eigen::Affine3f::Identity() * Eigen::Translation3f(Eigen::Vector3f(0, 0, -10));
				cam_pos = Eigen::Vector3f(0, 0, -10);
			}
			if (ImGui::Button("Print Debug")) {
				std::cout << "cam_pos: \n" << cam_pos << "\nview_mat: \n" << view_mat.matrix() << "\trans_cam: \n" << axis <<"\n ========================"<< std::endl;
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
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			0.3f,  1.0f, 0.0f, 0.0f, 1.0f,
			0.3f, 0.3f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, 0.3f, 0.0f, 1.0f, 0.0f,
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
