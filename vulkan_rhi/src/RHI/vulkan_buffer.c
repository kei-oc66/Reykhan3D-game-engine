#include "RHI/vulkan_buffer.h"
#include "RHI/vulkan_device.h"
#include "RHI/vulkan_swapchain.h"
#include "renderer/vertex.h"
#define CGLM_FORCE_RADIANS
#include <GLFW/glfw3.h>
#include <cglm/affine-pre.h>
#include <cglm/cglm.h>
#include <cglm/types.h>
//  std
#include <stdio.h>
#include <string.h>

VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;
VkDescriptorSetLayout descriptorSetLayout;
VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

extern VkCommandPool commandPool;

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  printf("failed to find suitable memory type!\n");
  return UINT32_MAX;
}

int createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkBuffer *buffer,
                 VkDeviceMemory *bufferMemory) {
  VkBufferCreateInfo bufferInfo = {0};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
    printf("failed to create buffer!\n");
    return -1;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (allocInfo.memoryTypeIndex == UINT32_MAX) {
    printf("failed to find memory type for buffer!\n");
    return -1;
  }

  if (vkAllocateMemory(device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
    printf("failed to allocate buffer memory!\n");
    return -1;
  }

  vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
  return 0;
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer temporaryCommandBuffer;
  if (vkAllocateCommandBuffers(device, &allocInfo, &temporaryCommandBuffer) !=
      VK_SUCCESS) {
    printf("failed to allocate temporary transfer command buffer!\n");
    return;
  }

  VkCommandBufferBeginInfo beginInfo = {0};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(temporaryCommandBuffer, &beginInfo);

  VkBufferCopy copyRegion = {0};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size = size;
  vkCmdCopyBuffer(temporaryCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(temporaryCommandBuffer);

  VkSubmitInfo submitInfo = {0};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &temporaryCommandBuffer;

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);

  vkFreeCommandBuffers(device, commandPool, 1, &temporaryCommandBuffer);
}

int createVertexBuffer(void) {
  VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VkBufferCreateInfo bufferInfo = {0};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, NULL, &stagingBuffer) != VK_SUCCESS) {
    printf("failed to create vertex staging buffer!\n");
    return -1;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (allocInfo.memoryTypeIndex == UINT32_MAX) {
    printf("failed to find memory type for vertex staging buffer!\n");
    return -1;
  }

  if (vkAllocateMemory(device, &allocInfo, NULL, &stagingBufferMemory) !=
      VK_SUCCESS) {
    printf("failed to allocate vertex staging buffer memory!\n");
    return -1;
  }

  vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

  void *data;
  if (vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data) !=
      VK_SUCCESS) {
    printf("failed to map vertex staging buffer memory!\n");
    return -1;
  }
  memcpy(data, vertices, (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  bufferInfo.usage =
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  if (vkCreateBuffer(device, &bufferInfo, NULL, &vertexBuffer) != VK_SUCCESS) {
    printf("failed to create local GPU vertex buffer!\n");
    return -1;
  }

  vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (allocInfo.memoryTypeIndex == UINT32_MAX) {
    printf("failed to find memory type for vertex buffer!\n");
    return -1;
  }

  if (vkAllocateMemory(device, &allocInfo, NULL, &vertexBufferMemory) !=
      VK_SUCCESS) {
    printf("failed to allocate local GPU vertex buffer memory!\n");
    return -1;
  }

  vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
  copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, NULL);
  vkFreeMemory(device, stagingBufferMemory, NULL);

  return 0;
}

int createIndexBuffer(void) {
  VkDeviceSize bufferSize = sizeof(uint16_t) * indicesCount;

  VkBuffer stagingBuffer = VK_NULL_HANDLE;
  VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

  VkBufferCreateInfo bufferInfo = {0};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, NULL, &stagingBuffer) != VK_SUCCESS) {
    printf("failed to create index staging buffer!\n");
    return -1;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (allocInfo.memoryTypeIndex == UINT32_MAX) {
    printf("failed to find memory type for index staging buffer!\n");
    return -1;
  }

  if (vkAllocateMemory(device, &allocInfo, NULL, &stagingBufferMemory) !=
      VK_SUCCESS) {
    printf("failed to allocate index staging buffer memory!\n");
    return -1;
  }

  vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

  void *data;
  if (vkMapMemory(device, stagingBufferMemory, 0, memRequirements.size, 0,
                  &data) != VK_SUCCESS) {
    printf("failed inside createIndexBuffer!\n");
    return -1;
  }
  memcpy(data, indices, (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  bufferInfo.usage =
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  if (vkCreateBuffer(device, &bufferInfo, NULL, &indexBuffer) != VK_SUCCESS) {
    printf("failed to create local GPU index buffer!\n");
    return -1;
  }

  vkGetBufferMemoryRequirements(device, indexBuffer, &memRequirements);
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (allocInfo.memoryTypeIndex == UINT32_MAX) {
    printf("failed to find memory type for index buffer!\n");
    return -1;
  }

  if (vkAllocateMemory(device, &allocInfo, NULL, &indexBufferMemory) !=
      VK_SUCCESS) {
    printf("failed to allocate local GPU index buffer memory!\n");
    return -1;
  }

  vkBindBufferMemory(device, indexBuffer, indexBufferMemory, 0);

  copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, NULL);
  vkFreeMemory(device, stagingBufferMemory, NULL);

  return 0;
}

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

void destroyVulkanBuffer(void) {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(device, uniformBuffers[i], NULL);
    vkFreeMemory(device, uniformBuffersMemory[i], NULL);
  }
  vkDestroyDescriptorPool(device, descriptorPool, NULL);
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
  vkDestroyBuffer(device, indexBuffer, NULL);
  vkFreeMemory(device, indexBufferMemory, NULL);
  vkDestroyBuffer(device, vertexBuffer, NULL);
  vkFreeMemory(device, vertexBufferMemory, NULL);
}
