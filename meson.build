project(
    'rs232',
    'cpp',
    default_options : [
        'cpp_std=c++17',
    ],
    version : '1.0.0'
)

sources = [
    'rs232.cc'
]

if host_machine.system() == 'windows'
    sources += 'rs232_win.cc'
else
    sources += 'rs232_linux.cc'
endif

incdir = include_directories('.')

rs232 = library(
    'rs232', 
    sources, 
    version : meson.project_version(), 
    soversion : '0',
    include_directories : incdir,
    install : true,
)

rs232_dep = declare_dependency(
    include_directories : incdir,
    link_with : rs232,
    version: meson.project_version(),
)

pkg = import('pkgconfig')
pkg.generate(rs232)
