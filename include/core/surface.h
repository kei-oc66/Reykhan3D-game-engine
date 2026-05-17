#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

extern VkSurfaceKHR surface;

int createSurface(void);
void destroySurface(void);
