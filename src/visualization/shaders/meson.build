glslc = find_program('glslc')

shaders = files([
    'shader.vert',
    'shader.frag',
])

shaders_src = files([
    'uniform_buffer_object.cpp',
    'vertex.cpp',
])

# Compile GLSL shaders to SPIR-V binaries
vert_shader = custom_target(
    'vert_shader',
    command : [glslc, '@INPUT@', '-o', '@OUTPUT@'],
    input : shaders[0],
    output : '@PLAINNAME@.spv',
)

frag_shader = custom_target(
    'frag_shader',
    command : [glslc, '@INPUT@', '-o', '@OUTPUT@'],
    input : shaders[1],
    output : '@PLAINNAME@.spv',
)

# Embed the SPIR-V binaries into 'embdedded_shaders.hpp'
python = find_program('python3')
embed_program = files(['embed.py'])[0]

embedded_shaders = custom_target(
    'embdedded_shaders',
    command: [python, embed_program, '@OUTPUT@', 'vert_shader', '@INPUT0@', 'frag_shader', '@INPUT1@'],
    input: [vert_shader[0], frag_shader[0]],
    output: 'shaders.hpp',
)

shaders_dep = declare_dependency(
    sources : [embedded_shaders[0]],
)
