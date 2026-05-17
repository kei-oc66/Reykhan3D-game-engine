#include "core/test_app.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// std
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLAMP(val, min, max)                                                   \
  ((val) < (min) ? (min) : ((val) > (max) ? (max) : (max)))

GLFWwindow *window;

#define WIDTH 1000
#define HEIGHT 800

static char *NAME = "Reykhan";

VkInstance instance;
VkDevice device;
VkSurfaceKHR surface;
VkQueue presentQueue;
VkQueue graphicsQueue;
VkSwapchainKHR swapChain;
VkImage *swapChainImages;
uint32_t swapChainImagesCount;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkCommandPool commandPool;
VkCommandBuffer commandBuffer;
VkPipeline graphicsPipeline;
VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;
VkFence inFlightFence;

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
const char *deviceExtension[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const uint32_t validationLayerCount = 1;
const uint32_t deviceExtensionCount = 1;

#ifdef NDEBUG
const int enableValidationLayers = 0;
#else
const int enableValidationLayers = 1;
#endif

typedef struct {
  uint32_t value;
  int hasValue;
} OptionalUint32;

typedef struct {
  OptionalUint32 graphicsFamily;
  OptionalUint32 presentFamily;
} QueueFamilyIndices;

typedef struct {
  VkSurfaceCapabilitiesKHR capabilities;

  VkSurfaceFormatKHR *formats;
  uint32_t formatCount;

  VkPresentModeKHR *presentModes;
  uint32_t presentModeCount;
} SwapChainSupportDetails;

typedef struct {
  char *data;
  size_t size;
} FileData;

static void freeSwapChainSupportDetails(SwapChainSupportDetails *details);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

VkFramebuffer *swapChainFramebuffers;

static void initWindow(unsigned int width, unsigned int height, char *name) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow((int)width, (int)height, name, NULL, NULL);
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

  uint32_t totalCount = glfwExtensionCount + (enableValidationLayers ? 1 : 0);
  const char **extensions = malloc(sizeof(const char *) * totalCount);

  for (uint32_t i = 0; i < glfwExtensionCount; i++) {
    extensions[i] = glfwExtensions[i];
  }

  if (enableValidationLayers) {
    extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  }

  *count = totalCount;
  return extensions;
}

static int createInstance(void) {
  if (enableValidationLayers && !checkValidationLayerSupport()) {
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
  if (enableValidationLayers) {
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

int isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = findQueueFamilies(device);
  int extensionsSupported = checkDeviceExtensionSupport(device);

  SwapChainSupportDetails details = querySwapChainSupport(device);
  int swapChainAdequate =
      details.formatCount != 0 && details.presentModeCount != 0;
  freeSwapChainSupportDetails(&details);

  return indices.graphicsFamily.hasValue && extensionsSupported &&
         swapChainAdequate;
}

static int pickPhysicalDevice(VkPhysicalDevice *physicalDevice) {
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

static int createLogicalDevice(void) {
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

  if (enableValidationLayers) {
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

static void freeSwapChainSupportDetails(SwapChainSupportDetails *details) {
  free(details->formats);
  free(details->presentModes);
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details = {0};

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount,
                                       NULL);
  if (details.formatCount != 0) {
    details.formats = malloc(sizeof(VkSurfaceFormatKHR) * details.formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount,
                                         details.formats);
  }

  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &details.presentModeCount, NULL);
  if (details.presentModeCount != 0) {
    details.presentModes =
        malloc(sizeof(VkSurfacePresentModeKHR) * details.presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &details.presentModeCount, details.presentModes);
  }

  return details;
}

static VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats,
                        uint32_t formatCount) {
  for (uint32_t i = 0; i < formatCount; i++) {
    if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormats->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormats[i];
    }
  }

  return availableFormats[0];
}

static VkPresentModeKHR
chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes,
                      uint32_t presentModeCount) {
  for (uint32_t i = 0; i < presentModeCount; i++) {
    if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
      return availablePresentModes[i];
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D
chooseSwapExtent(const VkSurfaceCapabilitiesKHR *capabilities) {
  if (capabilities->currentExtent.width != UINT32_MAX)
    return capabilities->currentExtent;
  else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {(uint32_t)(width), (uint32_t)(height)};

    actualExtent.width =
        CLAMP(actualExtent.width, capabilities->minImageExtent.width,
              capabilities->maxImageExtent.width);
    actualExtent.height =
        CLAMP(actualExtent.height, capabilities->minImageExtent.height,
              capabilities->maxImageExtent.height);

    return actualExtent;
  }
}

static int createSwapChain(void) {
  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
      swapChainSupport.formats, swapChainSupport.formatCount);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(
      swapChainSupport.presentModes, swapChainSupport.presentModeCount);
  VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
  createInfo.queueFamilyIndexCount = 0;
  createInfo.pQueueFamilyIndices = NULL;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) !=
      VK_SUCCESS) {
    printf("failed to create swap chain!");
    return -1;
  }

  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
  swapChainImages = malloc(sizeof(VkImage) * imageCount);
  swapChainImagesCount = imageCount;
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;

  freeSwapChainSupportDetails(&swapChainSupport);

  return 0;
}

static int createSurface(void) {
  if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) {
    printf("failed to create window surface!");
    return -1;
  }

  return 0;
}

static int createImageViews(void) {
  swapChainImageViews = malloc(sizeof(VkImageView) * swapChainImagesCount);
  if (!swapChainImageViews)
    return -1;
  for (uint32_t i = 0; i < swapChainImagesCount; i++) {
    VkImageViewCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapChainImageFormat;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) !=
        VK_SUCCESS) {
      printf("failed to create image views!\n");
      return -1;
    }
  }

  return 0;
}

static FileData readFile(const char *fileName) {
  FileData result = {0};

  FILE *file = fopen(fileName, "rb"); //    rb  =  read binary
  if (!file) {
    printf("failed to open file: %s\n", fileName);
    return result;
  }

  fseek(file, 0, SEEK_END);
  result.size = ftell(file);
  rewind(file);

  result.data = malloc(result.size);
  fread(result.data, 1, result.size, file);

  fclose(file);
  return result;
}

VkShaderModule createShaderModule(FileData *code) {
  VkShaderModuleCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code->size;
  createInfo.pCode = (const uint32_t *)code->data;

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) !=
      VK_SUCCESS) {
    printf("failed to create shader module!\n");
    return VK_NULL_HANDLE;
  }

  return shaderModule;
}

static int createGraphicsPipeline(void) {
  FileData vertShader = readFile("out/build/shaders/vertex/vert.spv");
  FileData fragShader = readFile("out/build/shaders/fragment/frag.spv");

  printf("vert shader size: %zu bytes\n", vertShader.size);
  printf("frag shader size: %zu bytes\n", fragShader.size);

  VkShaderModule vertShaderModule = createShaderModule(&vertShader);
  VkShaderModule fragShaderModule = createShaderModule(&fragShader);

  free(vertShader.data);
  free(fragShader.data);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState = {0};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = NULL;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = NULL;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState = {0};
  viewportState.pViewports = NULL;
  viewportState.pScissors = NULL;

  VkPipelineRasterizationStateCreateInfo rasterizer = {0};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisampling = {0};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = NULL;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending = {0};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = NULL;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = NULL;

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL,
                             &pipelineLayout) != VK_SUCCESS) {
    printf("failed to create pipeline layout!\n");
    return -1;
  }

  VkGraphicsPipelineCreateInfo pipelineInfo = {0};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = NULL;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL,
                                &graphicsPipeline) != VK_SUCCESS) {
    printf("failed to create graphics pipeline!\n");
    return -1;
  }

  vkDestroyShaderModule(device, fragShaderModule, NULL);
  vkDestroyShaderModule(device, vertShaderModule, NULL);

  return 0;
}

static int createRenderPass(void) {
  VkAttachmentDescription colorAttachment = {0};
  colorAttachment.format = swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {0};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo = {0};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  VkSubpassDependency dependency = {0};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) !=
      VK_SUCCESS) {
    printf("failed to create render pass!\n");
    return -1;
  }

  return 0;
}

static int createFrameBuffers(void) {
  swapChainFramebuffers = malloc(sizeof(VkFramebuffer) * swapChainImagesCount);

  for (uint32_t i = 0; i < swapChainImagesCount; i++) {
    VkImageView attachments[] = {swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo = {0};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, NULL,
                            &swapChainFramebuffers[i]) != VK_SUCCESS) {
      printf("failed to create framebuffer!\n");
      return -1;
    }
  }

  return 0;
}

static int createCommandPool(void) {
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

  VkCommandPoolCreateInfo poolInfo = {0};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value;

  if (vkCreateCommandPool(device, &poolInfo, NULL, &commandPool) !=
      VK_SUCCESS) {
    printf("failed to create command pool!\n");
    return -1;
  }

  return 0;
}

static int createCommandBuffer(void) {
  VkCommandBufferAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) !=
      VK_SUCCESS) {
    printf("failed to allocate command buffers!\n");
    return -1;
  }

  return 0;
}

static int recordCommandBuffer(VkCommandBuffer commandBuffer,
                               uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo = {0};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = NULL;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    printf("failed to begin recording command buffer!\n");
    return -1;
  }

  VkRenderPassBeginInfo renderPassInfo = {0};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
  renderPassInfo.renderArea.extent = swapChainExtent;
  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphicsPipeline);

  VkViewport viewport = {0};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)(swapChainExtent.width);
  viewport.height = (float)(swapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor = {0};
  scissor.offset = (VkOffset2D){0, 0};
  scissor.extent = swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdDraw(commandBuffer, 3, 1, 0, 0);
  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    printf("failed to record command buffer!\n");
    return -1;
  }

  return 0;
}

static int createSyncObjects(void) {
  VkSemaphoreCreateInfo semaphoreInfo = {0};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {0};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateSemaphore(device, &semaphoreInfo, NULL,
                        &imageAvailableSemaphore) != VK_SUCCESS ||
      vkCreateSemaphore(device, &semaphoreInfo, NULL,
                        &renderFinishedSemaphore) != VK_SUCCESS ||
      vkCreateFence(device, &fenceInfo, NULL, &inFlightFence) != VK_SUCCESS) {
    printf("failed to create semaphores!\n");
    return -1;
  }

  return 0;
}

static int initVulkan(void) {
  if (createInstance() != 0)
    return -1;
  if (createSurface() != 0)
    return -1;
  if (pickPhysicalDevice(&physicalDevice) != 0)
    return -1;
  if (createLogicalDevice() != 0)
    return -1;
  if (createSwapChain() != 0)
    return -1;
  if (createImageViews() != 0)
    return -1;
  if (createRenderPass() != 0)
    return -1;
  if (createGraphicsPipeline() != 0)
    return -1;
  if (createFrameBuffers() != 0)
    return -1;
  if (createCommandPool() != 0)
    return -1;
  if (createCommandBuffer() != 0)
    return -1;
  if (createSyncObjects() != 0)
    return -1;
  return 0;
}

static int drawFrame(void) {
  vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
  vkResetFences(device, 1, &inFlightFence);

  uint32_t imageIndex;
  vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore,
                        VK_NULL_HANDLE, &imageIndex);

  vkResetCommandBuffer(commandBuffer, 0);
  recordCommandBuffer(commandBuffer, imageIndex);

  VkSubmitInfo submitInfo = {0};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) !=
      VK_SUCCESS) {
    printf("failed to submit draw command buffer!\n");
    return -1;
  }

  VkPresentInfoKHR presentInfo = {0};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = NULL;

  vkQueuePresentKHR(presentQueue, &presentInfo);

  return 0;
}

static void mainLoop(void) {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    drawFrame();
  }
}

static void cleanUp(void) {
  vkDestroySemaphore(device, imageAvailableSemaphore, NULL);
  vkDestroySemaphore(device, renderFinishedSemaphore, NULL);
  vkDestroyFence(device, inFlightFence, NULL);

  vkDestroyCommandPool(device, commandPool, NULL);

  for (uint32_t i = 0; i < swapChainImagesCount; i++) {
    vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
  }
  free(swapChainFramebuffers);

  vkDestroyPipelineLayout(device, pipelineLayout, NULL);
  vkDestroyRenderPass(device, renderPass, NULL);

  for (uint32_t i = 0; i < swapChainImagesCount; i++) {
    vkDestroyImageView(device, swapChainImageViews[i], NULL);
  }
  free(swapChainImageViews);

  vkDestroySwapchainKHR(device, swapChain, NULL);
  free(swapChainImages);

  vkDestroyDevice(device, NULL);
  vkDestroySurfaceKHR(instance, surface, NULL);
  vkDestroyInstance(instance, NULL);
  glfwDestroyWindow(window);
  glfwTerminate();
}

void run(HelloTriangleApp *app) {
  initWindow(WIDTH, HEIGHT, NAME);
  if (initVulkan() != 0) {
    printf("Vulkan initialization failed.\n");
    return;
  }
  mainLoop();
  cleanUp();
}
