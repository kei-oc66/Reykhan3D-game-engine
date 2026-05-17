#include "core/device.h"
#include "core/instance.h"
#include "core/swapchain.h"

//  std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VkPhysicalDevice physicalDevice;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

const char *deviceExtension[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const uint32_t deviceExtensionCount = 1;

static int checkDeviceExtensionSupport(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
static int isDeviceSuitable(VkPhysicalDevice device);

int pickPhysicalDevice(VkPhysicalDevice *physicalDevice) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

  if (deviceCount == 0) {
    printf("failed to find GPUs with Vulkan support!\n");
    return -1;
  }

  VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  for (uint32_t i = 0; i < deviceCount; i++) {
    if (isDeviceSuitable(devices[i])) {
      *physicalDevice = devices[i];
      free(devices);
      return 0;
    }
  }

  free(devices);
  printf("failed to find a suitable GPU!\n");
  return -1;
}

static int isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = findQueueFamilies(device);
  int extensionsSupported = checkDeviceExtensionSupport(device);

  SwapChainSupportDetails details = querySwapChainSupport(device);
  int swapChainAdequate =
      details.formatCount != 0 && details.presentModeCount != 0;
  freeSwapChainSupportDetails(&details);

  return indices.graphicsFamily.hasValue && extensionsSupported &&
         swapChainAdequate;
}

int createLogicalDevice(void) {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  uint32_t uniqueFamilies[2];
  uint32_t uniqueCount = 0;

  uniqueFamilies[uniqueCount++] = indices.graphicsFamily.value;

  float queuePriority = 1.0f;

  VkDeviceQueueCreateInfo queueCreateInfos[2] = {0};

  for (uint32_t i = 0; i < uniqueCount; i++) {
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].queueFamilyIndex = uniqueFamilies[i];
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].pQueuePriorities = &queuePriority;
  }

  VkPhysicalDeviceFeatures deviceFeatures = {0};

  VkDeviceCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.queueCreateInfoCount = 1;
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = deviceExtensionCount;
  createInfo.ppEnabledExtensionNames = deviceExtension;

  if (ENABLE_VALIDATION_LAYERS) {
    createInfo.enabledLayerCount = validationLayerCount;
    createInfo.ppEnabledLayerNames = validationLayers;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) !=
      VK_SUCCESS) {
    printf("failed to create logical device!\n");
    return -1;
  }

  vkGetDeviceQueue(device, indices.graphicsFamily.value, 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.graphicsFamily.value, 0, &presentQueue);

  return 0;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;
  indices.graphicsFamily.hasValue = 0;
  indices.graphicsFamily.value = 0;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

  VkQueueFamilyProperties *queueFamilies =
      malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies);

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily.value = i;
      indices.graphicsFamily.hasValue = 1;
      break;
    }
  }

  free(queueFamilies);
  return indices;
}

static int checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;

  vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

  VkExtensionProperties *availableExtensions =
      malloc(sizeof(VkExtensionProperties) * extensionCount);

  vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
                                       availableExtensions);

  for (uint32_t i = 0; i < deviceExtensionCount; i++) {
    int found = 0;
    for (uint32_t j = 0; j < extensionCount; j++) {
      if (strcmp(deviceExtension[i], availableExtensions[j].extensionName) ==
          0) {
        found = 1;
        break;
      }
    }
    if (!found) {
      free(availableExtensions);
      return 0;
    }
  }

  free(availableExtensions);
  return 1;
}

void destroyDevice(void) { vkDestroyDevice(device, NULL); }
