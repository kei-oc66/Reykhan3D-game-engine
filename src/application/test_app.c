#include "application/test_app.h"

#include <vulkan/vulkan_core.h>

#include "core/commands.h"
#include "core/device.h"
#include "core/index_buffer.h"
#include "core/instance.h"
#include "core/pipeline.h"
#include "core/renderpass.h"
#include "core/surface.h"
#include "core/swapchain.h"
#include "core/sync.h"
#include "core/vertex_buffer.h"
#include "core/window.h"

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
  if (createVertexBuffer() != 0)
    return -1;
  if (createIndexBuffer() != 0)
    return -1;
  if (createCommandBuffer() != 0)
    return -1;
  if (createSyncObjects() != 0)
    return -1;
  return 0;
}

static int drawFrame(void) {
  vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                                          imageAvailableSemaphore,
                                          VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return 0;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    printf("failed to acquire swap chain image!\n");
    return -1;
  }

  vkResetFences(device, 1, &inFlightFence);

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

  result = vkQueuePresentKHR(presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      isFrameBufferResized()) {
    resetFramebufferResized();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    printf("failed to present swap chain image!\n");
    return -1;
  }

  return 0;
}

static void mainLoop(void) {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    drawFrame();
  }

  vkDeviceWaitIdle(device);
}

static void cleanUp(void) {
  destroySyncObjects();
  destroyCommandPool();
  destroySwapChainFrameBuffers();
  destroyPipeLine();
  destroyRenderPass();
  destroySwapChain();
  destroyIndexBuffer();
  destroyVertexBuffer();
  destroyDevice();
  destroySurface();
  destroyInstance();
  destroyWindow();
}

void run(void) {
  initWindow();
  if (initVulkan() != 0) {
    printf("Vulkan initialization failed.\n");
    return;
  }
  mainLoop();
  cleanUp();
}
