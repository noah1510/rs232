project(
    'rs232',
    'cpp',
    'c',
    default_options : [
        'cpp_std=c++17',
        'c_std=c17',
    ],
    version : '1.2.0',
    meson_version : '>= 0.55'

)

# find the c and c++ compiler to check for working headers.
cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# All the headers required to compile the lib on unix systems
unix_c_headers = [
    'termios.h',
    'sys/ioctl.h',
    'unistd.h',
    'fcntl.h',
    'limits.h',
]

# All the headers required to compile the lib on windows
windows_c_headers = [
    'windows.h',
]

# All the required c++ headers, there are needed across all operating systems
cpp_headers = [
    'string',
    'string_view',
    'tuple',
    'vector',
    'sstream',
    'algorithm',
    'iostream',
]

# The array containing all the sources that will be compiled
# The os specific sources will be added later
sources = [
    'src/rs232.cc'
]

# check if the c++ headers exist and work
foreach header_name : cpp_headers
    cxx.check_header(header_name, required: true)
endforeach

#extra linker args
extra_linker_args = ''

# check if the code is compiled on windows or any other system
if host_machine.system() == 'windows'
    # add the windows source and check if the heads exist and work
    sources += 'src/rs232_win.cc'

    foreach header_name : windows_c_headers
        cc.check_header(header_name, required: true)
    endforeach

else
    # add the unix source and check if the headers work
    sources += 'src/rs232_linux.cc'

    foreach header_name : unix_c_headers
        cc.check_header(header_name, required: true)
    endforeach
endif

# add the include directory, so that the compiler can find the header file
incdir = include_directories('include')

# this defines the library that should be compiled and all its dependencies
rs232 = library(
    'rs232', 
    sources, 
    version : meson.project_version(), 
    soversion : '0',
    include_directories : incdir,
    link_args: extra_linker_args,
    install : true,
)

# this defines the compiled library as a dependency that can be used by other meson projects
rs232_dep = declare_dependency(
    include_directories : incdir,
    link_with : rs232,
    version: meson.project_version(),
)

if get_option('build_sample')
    sample_src = [
        'interfaceTest'
    ]
    
    if host_machine.system() == 'windows'
        message('No sample specific for windows exists yet! Feel free to open a merge request to add one.')
    else
        message('No sample specific for unix exists yet! Feel free to open a merge request to add one.')
    endif

    foreach sample_program : sample_src

        exe = executable(
            sample_program, 
            'samples/' + sample_program + '.cpp',
            link_args: extra_linker_args,
            dependencies: rs232_dep
        )
        
    endforeach

endif

# this allows the library to be found by pkg config if you want to have a system installation
pkg = import('pkgconfig')
pkg.generate(rs232)

meson.override_dependency('rs232', rs232_dep)
