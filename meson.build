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
    input: 'assets/shaders/vertex/shader.vert',
    output: 'vert.spv',
    command: [glslc, '@INPUT@', '-o', meson.current_build_dir() / 'shaders/vertex/vert.spv'],
    build_by_default: true
)

frag_shader = custom_target('frag_shader',
    input: 'assets/shaders/fragment/shader.frag',
    output: 'frag.spv',
    command: [glslc, '@INPUT@', '-o', meson.current_build_dir() / 'shaders/fragment/frag.spv'],
    build_by_default: true
)

rhi_sources = []
rhi_includes = []

engine_deps = [glfw_dep, cglm_dep, m_dep]
chosen_backend = 'vulkan'

if chosen_backend == 'vulkan'
  add_project_arguments('-DENGINE_RENDERER_VULKAN', language : 'c')
  subdir('vulkan_rhi')
elif chosen_backend == 'opengl'
  add_project_arguments('-DENGINE_RENDERER_OPENGL', language : 'c')
  subdir('rhi_opengl')
endif

engine_sources = files(
    'main.c',
    'src/application/test_app.c',
    'src/core/window.c',
    'src/renderer/vertex.c',
    'src/renderer/texture.c',
    'externals/stb/stb_image.c',
  )

executable('vulkan_demo',
            engine_sources,
            rhi_sources,
            include_directories : [include_directories('include', 'externals', 'assets'),
            rhi_includes],
            dependencies : engine_deps
           )
