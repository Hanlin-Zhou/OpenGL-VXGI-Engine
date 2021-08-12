#include <Application.h>


bool Application::centerRotateMode = false;
unsigned int Application::windowWidth = 1024;
unsigned int Application::windowHeight = 1024;
int Application::leftMouseDown = 0;
int Application::rightMouseDown = 0;
float Application::rotateSensitivity = 0.3;
float Application::walkSensitivity = 0.05;
Camera Application::currCam = Camera(glm::vec3(0.0f, 0.0f, 10.0f), 
	glm::vec3(0.0f, 0.0f, 0.0f), 
	(float)Application::windowWidth / (float)Application::windowHeight);


Application::Application() {
	currRenderer = Renderer(windowWidth, windowHeight);
	create_window();
}


Application::~Application() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(glfw_window);
	glfwTerminate();
}

int Application::create_window() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfw_window = glfwCreateWindow(windowWidth, windowHeight, "Hello Atelier", NULL, NULL);
	if (glfw_window == NULL) {
		std::cout << "Creating window Failed\n" << std::endl;
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent(glfw_window);
	glfwSetKeyCallback(glfw_window, glfw_key_callback);
	glfwSetFramebufferSizeCallback(glfw_window, glfw_resize_callback);
	glfwSetMouseButtonCallback(glfw_window, glfw_MouseButtonCallback);
	glfwSetCursorPosCallback(glfw_window, glfw_CursorPosCallback);

	

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 0;
	}

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
	ImGui::StyleColorsDark();

	return 1;
}


void Application::Run() {
	while (!glfwWindowShouldClose(glfw_window)) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		processCamWalkInput();
		currRenderer.update(currCam.getViewMat(), currCam.getProjMat(), currCam.getPosition(), windowWidth, windowHeight);
		currRenderer.run();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(glfw_window);
		glfwPollEvents();
	}
}


void Application::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		centerRotateMode = !centerRotateMode;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		std::cout << glm::to_string(currCam.getPosition()) << std::endl;
	}
}


void Application::glfw_resize_callback(GLFWwindow* window, int new_width, int new_height) {
	glViewport(0, 0, new_width, new_height);
	windowWidth = new_width;
	windowHeight = new_height;
	currCam.aspect = (float)new_width / (float)new_height;
}


void Application::glfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
			leftMouseDown = 1;
		}
		if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) {
			rightMouseDown = 1;
		}
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
			leftMouseDown = 0;
		}
		if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE) {
			rightMouseDown = 0;
		}
	}
}


void Application::glfw_CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	static double CursorLastX = xpos;
	static double CursorLastY = ypos;
	ImGuiIO& io = ImGui::GetIO();
	if (leftMouseDown && !io.WantCaptureMouse) {
		if (!centerRotateMode) {
			glm::mat4 rot_mat = glm::mat4(1.0f);
			float hor_r = -glm::radians(rotateSensitivity * (xpos - CursorLastX) / windowWidth) * 500.0;
			float ver_r = glm::radians(rotateSensitivity * (ypos - CursorLastY) / windowHeight) * 400.0;
			currCam.rotate(hor_r, ver_r);
		}
		else {
			// TODO
		}
	}
	CursorLastX = xpos;
	CursorLastY = ypos;
}


void Application::processCamWalkInput() {
	if (currRenderer.getState() == 2) {
		glm::vec3 front = currCam.getLookingDirection();
		glm::vec3 right = currCam.getRightHandDirection();
		glm::vec3 up = currCam.getUpDirection();
		if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(glfw_window, true);
		}
		if (glfwGetKey(glfw_window, GLFW_KEY_W) == GLFW_PRESS) {
			currCam.translate(walkSensitivity * front);
		}
		else if (glfwGetKey(glfw_window, GLFW_KEY_S) == GLFW_PRESS) {
			currCam.translate(walkSensitivity * -front);
		}
		if (glfwGetKey(glfw_window, GLFW_KEY_A) == GLFW_PRESS) {
			currCam.translate(walkSensitivity * right);
		}
		else if (glfwGetKey(glfw_window, GLFW_KEY_D) == GLFW_PRESS) {
			currCam.translate(walkSensitivity * -right);
		}
		if (glfwGetKey(glfw_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			currCam.translate(walkSensitivity * up);
		}
		else if (glfwGetKey(glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			currCam.translate(walkSensitivity * -up);
		}
	}
}