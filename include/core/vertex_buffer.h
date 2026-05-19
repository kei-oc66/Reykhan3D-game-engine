#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

extern VkBuffer vertexBuffer;
extern VkDeviceMemory vertexBufferMemory;

int createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkBuffer *buffer,
                 VkDeviceMemory *bufferMemory);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
int createVertexBuffer(void);
void destroyVertexBuffer(void);
