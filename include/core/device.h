#pragma once

#include <vulkan/vulkan.h>

extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
extern VkQueue graphicsQueue;
extern VkQueue presentQueue;

typedef struct {
  uint32_t value;
  int hasValue;
} OptionalUint32;

typedef struct {
  OptionalUint32 graphicsFamily;
  OptionalUint32 presentFamily;
} QueueFamilyIndices;

int pickPhysicalDevice(VkPhysicalDevice *physicalDevice);
int createLogicalDevice(void);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

void destroyDevice(void);
