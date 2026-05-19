#pragma once

#include <vulkan/vulkan.h>

#include <cglm/cglm.h>
#include <cglm/types.h>

//  std
#include <stdint.h>

typedef struct {
  vec2 pos;
  vec3 color;
} Vertex;

extern Vertex vertices[];
extern uint32_t vertexCount;

extern uint16_t indices[];
extern uint32_t indicesCount;

VkVertexInputBindingDescription vertexGetBindingDescription(void);
void vertexGetAttributeDescriptions(
    VkVertexInputAttributeDescription attributeDescriptions[2]);
