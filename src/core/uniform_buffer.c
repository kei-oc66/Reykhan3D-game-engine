#include "core/uniform_buffer.h"
#include "core/device.h"
#include "core/swapchain.h"
#include "core/vertex_buffer.h"
#include <cglm/affine-pre.h>
#include <cglm/types.h>

#define CGLM_FORCE_RADIANS
#include <cglm/cglm.h>

#include <GLFW/glfw3.h>

//  std
#include <string.h>

VkDescriptorSetLayout descriptorSetLayout;
VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];

VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

int createDescriptorSetLayout(void) {
  VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = NULL;

  VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;

  if (vkCreateDescriptorSetLayout(device, &layoutInfo, NULL,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    printf("failed to create descriptor set layout!\n");
    return -1;
  }
  return 0;
}

int createUniformBuffers(void) {
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &uniformBuffers[i], &uniformBuffersMemory[i]);

    vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0,
                &uniformBuffersMapped[i]);
  }
  return 0;
}

void updateUniformBuffer(uint32_t currentImage) {
  static double startTime = -1.0;
  if (startTime < 0) {
    startTime = glfwGetTime();
  }

  double currentTime = glfwGetTime();
  float time = (float)(currentTime - startTime);

  UniformBufferObject ubo = {0};
  glm_mat4_identity(ubo.model);
  glm_rotate(ubo.model, time * glm_rad(90.0f), (vec3){0.0f, 0.0f, 1.0f});

  vec3 eye = {2.0f, 2.0f, 2.0f};
  vec3 center = {0.0f, 0.0f, 0.0f};
  vec3 up = {0.0f, 0.0f, 1.0f};
  glm_lookat(eye, center, up, ubo.view);

  float aspect = (float)swapChainExtent.width / (float)swapChainExtent.height;
  glm_perspective(glm_rad(45.0f), aspect, 0.1f, 10.0f, ubo.proj);

  ubo.proj[1][1] *= -1.0f;

  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

int createDescriptorPool(void) {
  VkDescriptorPoolSize poolSize = {0};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;

  VkDescriptorPoolCreateInfo poolInfo = {0};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = (uint32_t)MAX_FRAMES_IN_FLIGHT;

  if (vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptorPool) !=
      VK_SUCCESS) {
    printf("failed to create descriptor pool!\n");
    return -1;
  }
  return 0;
}

int createDescriptorSets(void) {
  VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    layouts[i] = descriptorSetLayout;
  }

  VkDescriptorSetAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
  allocInfo.pSetLayouts = layouts;
  if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets) !=
      VK_SUCCESS) {
    printf("failed to allocate descriptor sets!\n");
    return -1;
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo bufferInfo = {0};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite = {0};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSets[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, NULL);
  }

  return 0;
}

void destroyUniformBuffer(void) {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(device, uniformBuffers[i], NULL);
    vkFreeMemory(device, uniformBuffersMemory[i], NULL);
  }
  vkDestroyDescriptorPool(device, descriptorPool, NULL);

  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
}
