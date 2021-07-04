#include <ApplicationUtility.h>

//GLFWwindow* init(const int width, const int height) {
//	/// to be moved in to another file for refactoring
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	//	//init error check
//	GLFWwindow* window = glfwCreateWindow(width, height, "Hello Atelier", NULL, NULL);
//	if (window == NULL) {
//		std::cout << "Creating window Failed\n" << std::endl;
//		glfwTerminate();
//		return window;
//	}
//
//	glfwMakeContextCurrent(window);
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//	glfwSetMouseButtonCallback(window, processMouseInput);
//	glfwSetCursorPosCallback(window, processCursusPos);
//	// get function pointers
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//	{
//		std::cout << "Failed to initialize GLAD" << std::endl;
//		return window;
//	}
//	return window;
//}




void processMouseInput(GLFWwindow* window, int button, int action, int mods) {
	/*if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
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
	}*/
}

void processCursusPos(GLFWwindow* window, double xpos, double ypos) {
	/*if (Left_Mouse_down) {
		std::cout << "X:" << xpos << "\nY: " << ypos << "\nLMB: " << Left_Mouse_down << std::endl;
		static double CursorLastX = xpos;
		static double CursorLastY = ypos;

	}*/
}