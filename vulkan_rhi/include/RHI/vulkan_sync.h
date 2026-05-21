#pragma once

#include "RHI/vulkan_buffer.h"

#include <vulkan/vulkan.h>

extern VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore *renderFinishedSemaphores;
extern VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];

int createSyncObjects(void);
void destroySyncObjects(void);
