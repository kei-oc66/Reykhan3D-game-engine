#pragma once

#include <vulkan/vulkan.h>

extern VkSemaphore imageAvailableSemaphore;
extern VkSemaphore renderFinishedSemaphore;
extern VkFence inFlightFence;

int createSyncObjects(void);

void destroySyncObjects(void);
