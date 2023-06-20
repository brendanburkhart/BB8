subdir('shaders')

vulkan_src = files([
    'application.cpp',
    'buffer.cpp',
    'depth_buffer.cpp',
    'device.cpp',
    'frame_resources.cpp',
    'image.cpp',
    'memory.cpp',
    'swap_chain.cpp',
    'texture.cpp',
    'utilities.cpp',
    'window.cpp',
])

vulkan_src += shaders_src

vulkan_deps = [ shaders_dep ]
