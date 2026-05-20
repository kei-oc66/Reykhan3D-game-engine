project('vulkan-app', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c11'],
  default_options : ['buildtype=debug'])

vulkan_dep = dependency('vulkan')

glfw_dep = dependency('glfw3')

glslc = find_program('glslc')

cglm_dep = dependency('cglm')

m_dep = meson.get_compiler('c').find_library('m')

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
           'src/main.c',
           'src/core/window.c',
           'src/core/instance.c',
           'src/core/device.c',
           'src/core/surface.c',
           'src/core/swapchain.c',
           'src/core/pipeline.c',
           'src/core/renderpass.c',
           'src/core/commands.c',
           'src/core/sync.c',
           'src/core/vertex_buffer.c',
           'src/core/index_buffer.c',
           'src/core/uniform_buffer.c',
           'src/application/test_app.c',
           'src/renderer/vertex.c',
           include_directories : include_directories('include'),
           dependencies : [vulkan_dep, glfw_dep, cglm_dep, m_dep]
           )
