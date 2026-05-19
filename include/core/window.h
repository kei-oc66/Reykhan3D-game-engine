#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define WIDTH 800
#define HEIGHT 600

#define NAME "Reykhan"

extern GLFWwindow *window;

int isFrameBufferResized(void);
void resetFramebufferResized(void);

void initWindow(void);
void destroyWindow(void);
