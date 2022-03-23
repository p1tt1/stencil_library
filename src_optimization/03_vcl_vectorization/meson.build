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
#
# needed for time measure
#
add_global_arguments('-fopenmp', language : 'cpp')
add_global_link_arguments('-fopenmp', language : 'cpp')

# > likwid
add_global_arguments('-DLIKWID_PERFMON', language : 'cpp')
add_global_arguments('-I/usr/local/include/', language : 'cpp')
add_global_link_arguments('-L/usr/local/lib/', language : 'cpp')
add_global_link_arguments('-llikwid', language : 'cpp')

exe_verify_operator = executable(
  'exe_verify_operator',
  'exe_verify_operator.cpp',
  install : true
)
exe_op_bench_linear_stencil = executable(
  'exe_op_bench_linear_stencil',
  'exe_op_bench_linear_stencil.cpp',
  install : true
)
exe_op_bench_linear_stencil_vcl = executable(
  'exe_op_bench_linear_stencil_vcl',
  'exe_op_bench_linear_stencil_vcl.cpp',
  install : true
)
exe_identify_simd_level = executable(
  'exe_identify_simd_level',
  'exe_identify_simd_level.cpp',
  'vcl/instrset_detect.cpp',
  install : true
)

test('basic', exe_verify_operator)
test('basic', exe_op_bench_linear_stencil)
test('basic', exe_op_bench_linear_stencil_vcl)
test('basic', exe_identify_simd_level)
