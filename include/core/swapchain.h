#pragma once

#include <vulkan/vulkan.h>

extern VkSwapchainKHR swapChain;
extern VkImage *swapChainImages;
extern uint32_t swapChainImagesCount;
extern VkFormat swapChainImageFormat;
extern VkExtent2D swapChainExtent;
extern VkImageView *swapChainImageViews;
extern VkFramebuffer *swapChainFramebuffers;

typedef struct {
  VkSurfaceCapabilitiesKHR capabilities;

  VkSurfaceFormatKHR *formats;
  uint32_t formatCount;

  VkPresentModeKHR *presentModes;
  uint32_t presentModeCount;
} SwapChainSupportDetails;

int createSwapChain(void);

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

void freeSwapChainSupportDetails(SwapChainSupportDetails *details);

int createImageViews(void);

int createFrameBuffers(void);

void destroySwapChain(void);
void destroySwapChainFrameBuffers(void);
