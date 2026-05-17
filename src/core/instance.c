#include "core/instance.h"

#include <GLFW/glfw3.h>

//  std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VkInstance instance;

const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
const uint32_t validationLayerCount = 1;

static int checkValidationLayerSupport(void);
static const char **getRequiredExtensions(uint32_t *count);

int createInstance(void) {
  if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
    printf("validation layers requested but not available!\n");
    return -1;
  }

  VkApplicationInfo appInfo = {0};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  uint32_t extensionCount = 0;
  const char **extensions = getRequiredExtensions(&extensionCount);
  createInfo.enabledExtensionCount = extensionCount;
  createInfo.ppEnabledExtensionNames = extensions;
  if (ENABLE_VALIDATION_LAYERS) {
    createInfo.enabledLayerCount = validationLayerCount;
    createInfo.ppEnabledLayerNames = validationLayers;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;
  }

  VkResult result = vkCreateInstance(&createInfo, NULL, &instance);

  free(extensions);

  if (result != VK_SUCCESS) {
    printf("failed to create instance!\n");
    return -1;
  }
  return 0;
}

static int checkValidationLayerSupport(void) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, NULL);

  VkLayerProperties *availableLayers =
      malloc(sizeof(VkLayerProperties) * layerCount);

  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

  for (uint32_t i = 0; i < validationLayerCount; i++) {
    int layerFound = 0;

    for (uint32_t j = 0; j < layerCount; j++) {
      if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
        layerFound = 1;
        break;
      }
    }

    if (!layerFound) {
      free(availableLayers);
      return 0;
    }
  }

  free(availableLayers);
  return 1;
}

static const char **getRequiredExtensions(uint32_t *count) {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  uint32_t totalCount = glfwExtensionCount + (ENABLE_VALIDATION_LAYERS ? 1 : 0);
  const char **extensions = malloc(sizeof(const char *) * totalCount);

  for (uint32_t i = 0; i < glfwExtensionCount; i++) {
    extensions[i] = glfwExtensions[i];
  }

  if (ENABLE_VALIDATION_LAYERS) {
    extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  }

  *count = totalCount;
  return extensions;
}

void destroyInstance(void) { vkDestroyInstance(instance, NULL); }
