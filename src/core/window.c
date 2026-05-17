#include "core/window.h"

GLFWwindow *window;

void initWindow(void) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, NAME, NULL, NULL);
}

void destroyWindow(void) {
  glfwDestroyWindow(window);
  glfwTerminate();
}
