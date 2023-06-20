subdir('resources')
subdir('vulkan')

visualization_src = files([
    'visualization.cpp',
])

visualization_src += resources_src
visualization_src += vulkan_src

visualization_deps = [ vulkan_deps ]
