project('radeonrays', 'cpp')
rr_SDK_home = get_option('rr_SDK_home')

src = ['main.cpp', 'shader_manager.cpp']
dir = [rr_SDK_home + '/RadeonRays/include',
       rr_SDK_home + '/CLW',
       rr_SDK_home + '/Calc/inc']

incdir = include_directories(dir)

CL = dependency('OpenCL')
GL = dependency('GL')
glew = dependency('glew')
glfw = dependency('glfw3')

comp = meson.get_compiler('cpp')

calc = comp.find_library('Calc', dirs : rr_SDK_home + '/build/Calc')
clw = comp.find_library('CLW', dirs : rr_SDK_home + '/build/CLW')
rrays = comp.find_library('RadeonRays', dirs : rr_SDK_home + '/build/bin')

executable('radeonrays', sources : src,
			 include_directories : incdir,
			 dependencies : [CL, GL, glew, glfw, calc, clw, rrays])