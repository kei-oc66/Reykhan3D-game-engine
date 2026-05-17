#include "core/commands.h"
#include "core/device.h"
#include "core/pipeline.h"
#include "core/renderpass.h"
#include "core/swapchain.h"

//  std
#include <stdio.h>
#include <stdlib.h>

VkCommandPool commandPool;
VkCommandBuffer commandBuffer;

int createCommandPool(void) {
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

int createCommandBuffer(void) {
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

int recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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

void destroyCommandPool(void) {

  vkDestroyCommandPool(device, commandPool, NULL);
}
