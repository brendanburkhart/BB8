project(
    'bb8_simulation',
    'cpp',
    version : '0.0.1',
    default_options: [
        'cpp_std=c++17',
        'werror=true',
        'warning_level=3',
    ]
)

cpp = meson.get_compiler('cpp')
add_project_arguments(cpp.get_supported_arguments(['-Wno-cast-function-type']), language: 'cpp')

# Meson will find correct threading library (i.e. pthreads or W32)
thread_dep = dependency('threads')

cpp_compiler = meson.get_compiler('cpp')
dl_dep = cpp_compiler.find_library('dl', required: false)

vulkan_dep = dependency('vulkan')
eigen_dep = dependency('eigen3')
glfw_dep = dependency('glfw3')

subdir('src')
