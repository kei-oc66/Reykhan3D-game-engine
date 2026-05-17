#pragma once

#include <vulkan/vulkan.h>

extern VkCommandPool commandPool;
extern VkCommandBuffer commandBuffer;

int createCommandPool(void);
int createCommandBuffer(void);
int recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

void destroyCommandPool(void);
