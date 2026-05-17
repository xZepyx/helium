project(
    'helium',
    'cpp',
    version: '0.1.0',
    default_options: ['cpp_std=c++20']
)

pybind11 = dependency('pybind11')

gtk = dependency('gtk4')

layer_shell = dependency('gtk4-layer-shell-0')

nlohmann_json = dependency('nlohmann_json', required: false)

executable(
    'helium_test',

    'src/main.cpp',

    dependencies: [
        gtk,
        layer_shell
    ]
)

python = import('python').find_installation()

python.extension_module(
    'helium',

    'src/python/module.cpp',

    dependencies: [
        pybind11,
        gtk,
        layer_shell,
        nlohmann_json
    ],

    install: true
)