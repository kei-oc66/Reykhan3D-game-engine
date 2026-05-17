#include "core/sync.h"
#include "core/device.h"

//  std
#include <stdio.h>

VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;
VkFence inFlightFence;

int createSyncObjects(void) {
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

void destroySyncObjects(void) {
  vkDestroySemaphore(device, imageAvailableSemaphore, NULL);
  vkDestroySemaphore(device, renderFinishedSemaphore, NULL);
  vkDestroyFence(device, inFlightFence, NULL);
}
