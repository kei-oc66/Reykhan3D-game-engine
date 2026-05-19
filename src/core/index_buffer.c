#include "core/index_buffer.h"
#include "core/device.h"
#include "renderer/vertex.h"

// std
#include <stdint.h>
#include <stdio.h>
#include <string.h>

VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;

// Local version of memory finder to guarantee context access
static uint32_t findLocalMemoryType(uint32_t typeFilter,
                                    VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }
  return -1;
}

int createIndexBuffer(void) {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indicesCount;

  // --- 1. Create Staging Buffer ---
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VkBufferCreateInfo bufferInfo = {0};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, NULL, &stagingBuffer) != VK_SUCCESS) {
    printf("ERROR: Failed to create index staging buffer!\n");
    return -1;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findLocalMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkAllocateMemory(device, &allocInfo, NULL, &stagingBufferMemory) !=
      VK_SUCCESS) {
    printf("ERROR: Failed to allocate index staging buffer memory!\n");
    return -1;
  }

  vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

  printf("--> About to map INDEX staging buffer!\n");
  if (stagingBufferMemory == VK_NULL_HANDLE) {
    printf("ERROR: Index staging memory is NULL!\n");
  }

  void *data;
  if (vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data) !=
      VK_SUCCESS) {
    printf("ERROR: vkMapMemory failed inside createIndexBuffer!\n");
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
  allocInfo.memoryTypeIndex = findLocalMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vkAllocateMemory(device, &allocInfo, NULL, &indexBufferMemory) !=
      VK_SUCCESS) {
    printf("failed to allocate local GPU index buffer memory!\n");
    return -1;
  }

  vkBindBufferMemory(device, indexBuffer, indexBufferMemory, 0);

  extern void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                         VkDeviceSize size);
  copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer, NULL);
  vkFreeMemory(device, stagingBufferMemory, NULL);

  return 0;
}

void destroyIndexBuffer(void) {
  vkDestroyBuffer(device, indexBuffer, NULL);
  vkFreeMemory(device, indexBufferMemory, NULL);
}
