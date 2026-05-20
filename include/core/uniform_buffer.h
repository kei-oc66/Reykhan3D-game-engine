#pragma once

#include <vulkan/vulkan.h>

#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <cglm/types.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
  mat4 model;
  mat4 view;
  mat4 proj;
} UniformBufferObject;

extern VkDescriptorSetLayout descriptorSetLayout;

extern VkBuffer indexBuffer;
extern VkDeviceMemory indexBufferMemory;
extern VkDescriptorPool descriptorPool;
extern VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];

extern VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
extern VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
extern void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

int createDescriptorSetLayout(void);
int createDescriptorPool(void);
int createDescriptorSets(void);
int createUniformBuffers(void);
void updateUniformBuffer(uint32_t currentImage);
void destroyUniformBuffer(void);
