project('vulkan-app', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c11'],
  default_options : ['buildtype=debug'])

vulkan_dep = dependency('vulkan')

glfw_dep = dependency('glfw3')

glslc = find_program('glslc')

vert_shader = custom_target('vert_shader',
    input: 'shaders/vertex/shader.vert',
    output: 'vert.spv',
    command: [glslc, '@INPUT@', '-o', meson.current_build_dir() / 'shaders/vertex/vert.spv'],
    build_by_default: true
)

frag_shader = custom_target('frag_shader',
    input: 'shaders/fragment/shader.frag',
    output: 'frag.spv',
    command: [glslc, '@INPUT@', '-o', meson.current_build_dir() / 'shaders/fragment/frag.spv'],
    build_by_default: true
)

executable('vulkan_demo',
           'main.c',
           'src/core/test_app.c',
           include_directories : include_directories('include'),
           dependencies : [vulkan_dep, glfw_dep])
