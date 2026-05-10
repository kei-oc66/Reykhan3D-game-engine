project('vulkan-app', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c11'],
  default_options : ['buildtype=debug'])

vulkan_dep = dependency('vulkan')

glfw_dep = dependency('glfw3')

executable('vulkan_demo',
           'main.c',
           'src/core/test_app.c',
           include_directories : include_directories('include'),
           dependencies : [vulkan_dep, glfw_dep])
