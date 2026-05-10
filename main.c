#include <stdio.h>

#include "core/test_app.h"

int main(void) {
  printf("Hello triangle!\n");

  HelloTriangleApp app;

  run(&app);

  return 0;
}
