#include "core/sync.h"
#include "core/device.h"
#include "core/swapchain.h"

//  std
#include <stdio.h>

VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
VkSemaphore *renderFinishedSemaphores = NULL;
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];

int createSyncObjects(void) {
  VkSemaphoreCreateInfo semaphoreInfo = {0};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {0};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device, &semaphoreInfo, NULL,
                          &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]) !=
            VK_SUCCESS) {
      printf("failed to create sync objects for frame %zu!\n", i);
      return -1;
    }
  }

  printf("swapChainImageCount = %u\n", swapChainImagesCount);

  renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * swapChainImagesCount);

  printf("renderFinishedSemaphores ptr = %p\n",
         (void *)renderFinishedSemaphores);

  if (!renderFinishedSemaphores) {
    printf("failed to allocate renderFinishedSemaphores!\n");
    return -1;
  }

  for (size_t i = 0; i < swapChainImagesCount; i++) {
    if (vkCreateSemaphore(device, &semaphoreInfo, NULL,
                          &renderFinishedSemaphores[i]) != VK_SUCCESS) {
      printf("failed to create render finished semaphore for image %zu!\n", i);
      return -1;
    }
  }

  return 0;
}

void destroySyncObjects(void) {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
    vkDestroyFence(device, inFlightFences[i], NULL);
  }

  for (size_t i = 0; i < swapChainImagesCount; i++) {
    vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
  }
  free(renderFinishedSemaphores);
  renderFinishedSemaphores = NULL;
}
