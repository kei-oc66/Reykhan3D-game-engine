#include <vulkan/vulkan_core.h>

#include "core/instance.h"
#include "core/surface.h"
#include "core/window.h"

//  std
#include <stdio.h>

VkSurfaceKHR surface;

int createSurface(void) {
  if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) {
    printf("failed to create window surface!");
    return -1;
  }

  return 0;
}

void destroySurface(void) { vkDestroySurfaceKHR(instance, surface, NULL); }
