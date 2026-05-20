#include "core/vertex_buffer.h"
#include "core/device.h"
#include "renderer/vertex.h"

// std
#include <stdio.h>
#include <string.h>

VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;

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
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i)) {
      return i;
    }
  }
  return 0;
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
  vkQueueWaitIdle(graphicsQueue); // Wait for the copy to finish immediately

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

void destroyVertexBuffer(void) {
  vkDestroyBuffer(device, vertexBuffer, NULL);
  vkFreeMemory(device, vertexBufferMemory, NULL);
}
