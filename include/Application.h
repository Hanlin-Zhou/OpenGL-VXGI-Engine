#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Renderer.h>
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>
#include <Camera.h>


class Application
{
private:
	GLFWwindow* glfw_window = nullptr;

	static unsigned int windowWidth;
	static unsigned int windowHeight;


	static Camera currCam;

	Renderer currRenderer;

	static int leftMouseDown;
	static int rightMouseDown;
	static float rotateSensitivity;
	static float walkSensitivity;
	static bool centerRotateMode;

	static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void glfw_resize_callback(GLFWwindow* window, int width, int height);
	static void glfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void glfw_CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

	int create_window();
	void processCamWalkInput();
	void create_renderer();

public:
	Application();
	~Application();
	void Run();
};