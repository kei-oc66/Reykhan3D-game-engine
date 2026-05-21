#pragma once

#include <vulkan/vulkan.h>

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS 0
#else
#define ENABLE_VALIDATION_LAYERS 1
#endif

extern VkInstance instance;

extern const char *validationLayers[];
extern const uint32_t validationLayerCount;

int createInstance(void);
void destroyInstance(void);
