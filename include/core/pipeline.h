#pragma once

#include <vulkan/vulkan.h>

//  std
#include <stdio.h>

extern VkPipeline graphicsPipeline;
extern VkPipelineLayout pipelineLayout;

typedef struct {
  char *data;
  size_t size;
} FileData;

int createGraphicsPipeline(void);

void destroyPipeLine(void);
