#include "core/swapchain.h"
#include "core/device.h"
#include "core/renderpass.h"
#include "core/surface.h"
#include "core/window.h"

//  std
#include <stdio.h>
#include <stdlib.h>

#define CLAMP(val, min, max)                                                   \
  ((val) < (min) ? (min) : ((val) > (max) ? (max) : (max)))

VkSwapchainKHR swapChain;
VkImage *swapChainImages;
uint32_t swapChainImagesCount;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;
VkFramebuffer *swapChainFramebuffers;

static VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats,
                        uint32_t formatCount);
static VkPresentModeKHR
chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes,
                      uint32_t presentModeCount);
static VkExtent2D
chooseSwapExtent(const VkSurfaceCapabilitiesKHR *capabilities);

int createSwapChain(void) {
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

void freeSwapChainSupportDetails(SwapChainSupportDetails *details) {
  free(details->formats);
  free(details->presentModes);
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

int createImageViews(void) {
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

int createFrameBuffers(void) {
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

void destroySwapChain(void) {
  for (uint32_t i = 0; i < swapChainImagesCount; i++) {
    vkDestroyImageView(device, swapChainImageViews[i], NULL);
  }
  free(swapChainImageViews);

  vkDestroySwapchainKHR(device, swapChain, NULL);
  free(swapChainImages);
}

void destroySwapChainFrameBuffers(void) {
  for (uint32_t i = 0; i < swapChainImagesCount; i++) {
    vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
  }
  free(swapChainFramebuffers);
}
