#include "core/window.h"
#include <GLFW/glfw3.h>

GLFWwindow *window;

static int framebufferResized = 0;

static void framebufferResizeCallback(GLFWwindow *window, int width,
                                      int height) {
  (void)window;
  (void)width;
  (void)height;
  framebufferResized = 1;
}

int isFrameBufferResized(void) { return framebufferResized; }

void resetFramebufferResized(void) { framebufferResized = 0; }

void initWindow(void) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(WIDTH, HEIGHT, NAME, NULL, NULL);

  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void destroyWindow(void) {
  glfwDestroyWindow(window);
  glfwTerminate();
}
