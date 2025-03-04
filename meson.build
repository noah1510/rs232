project(
    'rs232',
    'cpp',
    'c',
    default_options : [
        'cpp_std=c++17',
        'c_std=c17',
        'warning_level=3',
    ],
    version : '2.0.0',
    meson_version : '>= 1.1'
)

# find the c and c++ compiler to check for working headers.
cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# define the correct export macro on windows
if target_machine.system() == 'windows'
    if get_option('buildtype') != 'static'
        add_project_arguments('-DRS232_EXPORT_MACRO=__declspec(dllexport)', language : 'cpp')
    else
        add_project_arguments('-DRS232_EXPORT_MACRO=__declspec(dllimport)', language : 'cpp')
    endif

    if target_machine.cpu_family() == 'x86_64'
        add_project_arguments('-D_AMD64_', language : 'cpp')
    elif target_machine.cpu_family() == 'x86'
        add_project_arguments('-D_X86_', language : 'cpp')
    elif target_machine.cpu_family() == 'ia64'
        add_project_arguments('-D_IA64_', language : 'cpp')
    elif target_machine.cpu_family() == 'arm'
        add_project_arguments('-D_ARM_', language : 'cpp')
    elif target_machine.cpu_family() == 'aarch64'
        add_project_arguments('-D_ARM64_', language : 'cpp')
    else
        if meson.is_cross_build()
            error('Unsupported cross compile cpu family:' + target_machine.cpu_family())
        endif
    endif
endif

# All the headers required to compile the lib on unix systems
unix_c_headers = [
    'termios.h',
    'sys/ioctl.h',
    'fcntl.h',
]

# All the headers required to compile the lib on windows
windows_c_headers = [
    'windows.h',
]

# All the required c++ headers, there are needed across all operating systems
cpp_headers = [
    'algorithm',
    'atomic',
    'chrono',
    'climits',
    'filesystem',
    'functional',
    'future',
    'iostream',
    'memory',
    'ostream',
    'regex',
    'shared_mutex',
    'sstream',
    'string',
    'string_view',
    'tuple',
    'vector'
]

# The array containing all the sources that will be compiled
# The os specific sources will be added later
sources = [
    'src/rs232.cpp',
    'src/rs232_native_common.cpp',
]

# check if the c++ headers exist and work
foreach header_name : cpp_headers
    cxx.check_header(header_name, required : true)
endforeach

#extra linker args
extra_linker_args = ''

# check if the code is compiled on windows or any other system
if host_machine.system() == 'windows'
    # add the windows source and check if the heads exist and work
    sources += 'src/rs232_native_win.cpp'

    foreach header_name : windows_c_headers
        cc.check_header(header_name, required : true)
    endforeach

else
    # add the unix source and check if the headers work
    sources += 'src/rs232_native_linux.cpp'

    foreach header_name : unix_c_headers
        cc.check_header(header_name, required : true)
    endforeach
endif

# add the include directory, so that the compiler can find the header file
incdir = include_directories('include')

# now that async is used in the library, 'threads' is a required dependency
deps = [dependency('threads')]

# this defines the library that should be compiled and all its dependencies
rs232 = library(
    'rs232',
    sources,
    version : meson.project_version(),
    soversion : '0',
    include_directories : incdir,
    link_args : extra_linker_args,
    dependencies : deps,
    install : true,
)

# this defines the compiled library as a dependency that can be used by other meson projects
rs232_dep = declare_dependency(
    include_directories : incdir,
    link_with : rs232,
    dependencies : deps,
    version : meson.project_version(),
)


build_sample = get_option('build_sample').enable_auto_if(not meson.is_subproject()).enabled()

if build_sample
    message('Building sample programs')

    sample_src = [
        'interfaceTest',
        'readInfinite',
    ]

    foreach sample_program : sample_src

        exe = executable(
            sample_program,
            'samples/' + sample_program + '.cpp',
            link_args : extra_linker_args,
            dependencies : rs232_dep
        )

    endforeach

endif

# this allows the library to be found by pkg config if you want to have a system installation
pkg = import('pkgconfig')
pkg.generate(rs232)

meson.override_dependency('rs232', rs232_dep)
