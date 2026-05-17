#pragma once

#include <vulkan/vulkan.h>

extern VkRenderPass renderPass;

int createRenderPass(void);

void destroyRenderPass(void);
