project('00_unoptimized', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++17'])

add_global_arguments('-O0', language : 'cpp')
add_global_arguments('-fno-tree-vectorize', language : 'cpp')

#
# > openmp
#
# needed for time measure
#
add_global_arguments('-fopenmp', language : 'cpp')
add_global_link_arguments('-fopenmp', language : 'cpp')

#
# > likwid
#
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

e_sparse_band = executable(
  'e_sparse_band',
  'e_sparse_band.cpp',
  install : true
)
e_sparse_band_likwid = executable(
  'e_sparse_band_likwid',
  'e_sparse_band_likwid.cpp',
  install : true
)

e_sparse_csr = executable(
  'e_sparse_csr',
  'e_sparse_csr.cpp',
  install : true
)
e_sparse_csr_likwid = executable(
  'e_sparse_csr_likwid',
  'e_sparse_csr_likwid.cpp',
  install : true
)
