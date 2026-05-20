#include "core/commands.h"
#include "core/device.h"
#include "core/pipeline.h"
#include "core/renderpass.h"
#include "core/swapchain.h"
#include "core/vertex_buffer.h"
#include "renderer/vertex.h"

//  std
#include <stdio.h>
#include <vulkan/vulkan_core.h>

VkCommandPool commandPool;
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];

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
  allocInfo.commandBufferCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;

  if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers) !=
      VK_SUCCESS) {
    printf("failed to allocate command buffers!\n");
    return -1;
  }

  return 0;
}

int recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                        uint32_t currentFrame) {
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

  VkBuffer vertexBuffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  extern VkBuffer indexBuffer;
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

  extern VkPipelineLayout pipelineLayout;
  extern VkDescriptorSet descriptorSets[];

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipelineLayout, 0, 1, &descriptorSets[currentFrame],
                          0, NULL);

  extern uint32_t indicesCount;
  vkCmdDrawIndexed(commandBuffer, indicesCount, 1, 0, 0, 0);

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
