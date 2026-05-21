#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <cglm/types.h>

#define MAX_FRAMES_IN_FLIGHT 2

extern VkBuffer vertexBuffer;
extern VkDeviceMemory vertexBufferMemory;

extern VkDescriptorSetLayout descriptorSetLayout;

extern VkBuffer indexBuffer;
extern VkDeviceMemory indexBufferMemory;
extern VkDescriptorPool descriptorPool;
extern VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];

extern VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
extern VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
extern void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

typedef struct {
  mat4 model;
  mat4 view;
  mat4 proj;
} UniformBufferObject;

int createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkBuffer *buffer,
                 VkDeviceMemory *bufferMemory);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
int createVertexBuffer(void);
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
int createIndexBuffer(void);
int createDescriptorSetLayout(void);
int createDescriptorPool(void);
int createDescriptorSets(void);
int createUniformBuffers(void);
void updateUniformBuffer(uint32_t currentImage);
void destroyVulkanBuffer(void);
