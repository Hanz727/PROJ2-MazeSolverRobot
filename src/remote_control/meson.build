project('remote_control', ['c','cpp'],
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

ncurses_dep = []

# Check the platform and define the sources and executable name
if host_machine.system() == 'windows'
    exe_name = 'remote_control.exe'
    srcs = files('windows.cpp')
elif host_machine.system() == 'linux'
    exe_name = 'remote_control'
    srcs = files('linux.c')
    srcs = files('linux.cpp')
    ncurses_dep = dependency('ncurses', required: true)
else
    error('Unsupported platform')
endif

# Build the executable with platform-specific settings
exe = executable(exe_name,
                 srcs,
                 install : true,
                 dependencies: host_machine.system() == 'linux' ? ncurses_dep : [],
                 link_args: host_machine.system() == 'linux' ? ['-lncurses'] : [])

