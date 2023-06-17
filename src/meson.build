subdir('visualization')

entrypoint = files([ 'main.cpp' ])

executable(
    'bb8_simulation',
    entrypoint + visualization_src,
    include_directories: include_directories('.'),
    dependencies: [ thread_dep, eigen_dep, vulkan_dep, glfw_dep, visualization_deps ],
)
