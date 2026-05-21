#pragma once

#include "RHI/vulkan_buffer.h"
#include <vulkan/vulkan.h>

extern VkCommandPool commandPool;
extern VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];

int createCommandPool(void);
int createCommandBuffer(void);
int recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                        uint32_t currentFrame);
void destroyCommandPool(void);
