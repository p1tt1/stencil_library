project('01_compiler_optimized', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++17',
                     'optimization=3'])

# > optimize
add_global_arguments('-march=native', language : 'cpp')
add_global_arguments('-mtune=native', language : 'cpp')

# > strict aliasing
add_global_arguments('-fstrict-aliasing', language : 'cpp')

# > disable vectorization
add_global_arguments('-fno-tree-vectorize', language : 'cpp')
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
exe_runtime_by_cells = executable(
  'exe_runtime_by_cells',
  'exe_runtime_by_cells.cpp',
  install : true
)
exe_op_bench_linear_stencil = executable(
  'exe_op_bench_linear_stencil',
  'exe_op_bench_linear_stencil.cpp',
  install : true
)
exe_op_bench_sparse_band = executable(
  'exe_op_bench_sparse_band',
  'exe_op_bench_sparse_band.cpp',
  install : true
)
exe_op_bench_sparse_csr = executable(
  'exe_op_bench_sparse_csr',
  'exe_op_bench_sparse_csr.cpp',
  install : true
)

test('basic', exe_verify_operator)
test('basic', exe_runtime_by_cells)
test('basic', exe_op_bench_linear_stencil)
test('basic', exe_op_bench_sparse_band)
test('basic', exe_op_bench_sparse_csr)
