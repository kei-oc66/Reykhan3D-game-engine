#pragma once

//  vulkan
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

//  glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct {
  VkInstance instance;
} HelloTriangleApp;

void run(HelloTriangleApp *app);
