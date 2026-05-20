#include "renderer/vertex.h"

Vertex vertices[] = {{{-0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
                     {{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
                     {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                     {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
uint32_t vertexCount = 4;

uint16_t indices[] = {0, 1, 2, 2, 3, 0};
uint32_t indicesCount = INDICES_COUNT;

VkVertexInputBindingDescription vertexGetBindingDescription(void) {
  VkVertexInputBindingDescription bindingDescription = {0};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Vertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescription;
}

void vertexGetAttributeDescriptions(
    VkVertexInputAttributeDescription attributeDescriptions[2]) {
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, pos);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);
}
