project(
    'helium',
    'cpp',
    version: '0.1.0',
    default_options: ['cpp_std=c++20']
)

gtk = dependency('gtk4')
layer_shell = dependency('gtk4-layer-shell-0')

executable(
    'helium',
    'src/main.cpp',
    dependencies: [
        gtk,
        layer_shell
    ]
)