#pragma once

#include <vulkan/vulkan.h>

extern VkBuffer indexBuffer;
extern VkDeviceMemory indexBufferMemory;

int createIndexBuffer(void);

void destroyIndexBuffer(void);
