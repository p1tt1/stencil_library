project('02_auto_vectorization', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++17',
                     'optimization=3'])

# > optimize
add_global_arguments('-march=native', language : 'cpp')
add_global_arguments('-mtune=native', language : 'cpp')

# > strict aliasing
add_global_arguments('-fstrict-aliasing', language : 'cpp')

# > gcc auto veectorization
add_global_arguments('-ftree-vectorize', language : 'cpp')
add_global_arguments('-mprefer-vector-width=512', language : 'cpp')
add_global_arguments('-mfma', language : 'cpp')
add_global_arguments('-fopt-info-vec-optimized', language : 'cpp')

# > floating point flags
add_global_arguments('-fno-trapping-math', language : 'cpp')
add_global_arguments('-fno-math-errno', language : 'cpp')

# > openmp
add_global_arguments('-fopenmp', language : 'cpp')
add_global_link_arguments('-fopenmp', language : 'cpp')

# > likwid
add_global_arguments('-DLIKWID_PERFMON', language : 'cpp')
add_global_arguments('-I/usr/local/include/', language : 'cpp')
add_global_link_arguments('-L/usr/local/lib/', language : 'cpp')
add_global_link_arguments('-llikwid', language : 'cpp')

e_linear_stencil = executable(
  'e_linear_stencil',
  'e_linear_stencil.cpp',
  install : true
)
e_linear_stencil_likwid = executable(
  'e_linear_stencil_likwid',
  'e_linear_stencil_likwid.cpp',
  install : true
)
e_linear_stencil_firsttouch = executable(
  'e_linear_stencil_firsttouch',
  'e_linear_stencil_firsttouch.cpp',
  install : true
)
e_linear_stencil_firsttouch_likwid = executable(
  'e_linear_stencil_firsttouch_likwid',
  'e_linear_stencil_firsttouch_likwid.cpp',
  install : true
)
