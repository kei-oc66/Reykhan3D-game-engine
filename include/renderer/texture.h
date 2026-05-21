#pragma once

#include <vulkan/vulkan.h>

extern VkBuffer stagingBuffer;
extern VkDeviceMemory stagingBufferMemory;

extern VkImage textureImage;
extern VkDeviceMemory textureImageMemory;

int createTextureImage(void);
