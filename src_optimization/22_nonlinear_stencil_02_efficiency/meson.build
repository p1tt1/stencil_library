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

add_global_arguments('-pthread', language : 'cpp')
add_global_link_arguments('-pthread', language : 'cpp')
add_global_link_arguments('-lm', language : 'cpp')

e_nonlinear_stencil_mul2 = executable(
  'e_nonlinear_stencil_mul2',
  'e_nonlinear_stencil_mul2.cpp',
  install : true
)
e_nonlinear_stencil_mul2_likwid = executable(
  'e_nonlinear_stencil_mul2_likwid',
  'e_nonlinear_stencil_mul2_likwid.cpp',
  install : true
)
e_nonlinear_stencil_precalc_mul2 = executable(
  'e_nonlinear_stencil_precalc_mul2',
  'e_nonlinear_stencil_precalc_mul2.cpp',
  install : true
)
e_nonlinear_stencil_precalc_mul2_likwid = executable(
  'e_nonlinear_stencil_precalc_mul2_likwid',
  'e_nonlinear_stencil_precalc_mul2_likwid.cpp',
  install : true
)

e_nonlinear_stencil_pow2 = executable(
  'e_nonlinear_stencil_pow2',
  'e_nonlinear_stencil_pow2.cpp',
  install : true
)
e_nonlinear_stencil_pow2_likwid = executable(
  'e_nonlinear_stencil_pow2_likwid',
  'e_nonlinear_stencil_pow2_likwid.cpp',
  install : true
)
e_nonlinear_stencil_precalc_pow2 = executable(
  'e_nonlinear_stencil_precalc_pow2',
  'e_nonlinear_stencil_precalc_pow2.cpp',
  install : true
)

e_nonlinear_stencil_exp = executable(
  'e_nonlinear_stencil_exp',
  'e_nonlinear_stencil_exp.cpp',
  install : true
)
e_nonlinear_stencil_exp_likwid = executable(
  'e_nonlinear_stencil_exp_likwid',
  'e_nonlinear_stencil_exp_likwid.cpp',
  install : true
)
e_nonlinear_stencil_precalc_exp = executable(
  'e_nonlinear_stencil_precalc_exp',
  'e_nonlinear_stencil_precalc_exp.cpp',
  install : true
)

e_nonlinear_stencil_pow4_3 = executable(
  'e_nonlinear_stencil_pow4_3',
  'e_nonlinear_stencil_pow4_3.cpp',
  install : true
)
e_nonlinear_stencil_pow4_3_likwid = executable(
  'e_nonlinear_stencil_pow4_3_likwid',
  'e_nonlinear_stencil_pow4_3_likwid.cpp',
  install : true
)
e_nonlinear_stencil_precalc_pow4_3 = executable(
  'e_nonlinear_stencil_precalc_pow4_3',
  'e_nonlinear_stencil_precalc_pow4_3.cpp',
  install : true
)

e_nonlinear_stencil_costly_0 = executable(
  'e_nonlinear_stencil_costly_0',
  'e_nonlinear_stencil_costly_0.cpp',
  install : true
)
e_nonlinear_stencil_costly_0_likwid = executable(
  'e_nonlinear_stencil_costly_0_likwid',
  'e_nonlinear_stencil_costly_0_likwid.cpp',
  install : true
)
e_nonlinear_stencil_precalc_costly_0 = executable(
  'e_nonlinear_stencil_precalc_costly_0',
  'e_nonlinear_stencil_precalc_costly_0.cpp',
  install : true
)

e_nonlinear_stencil_costly_1 = executable(
  'e_nonlinear_stencil_costly_1',
  'e_nonlinear_stencil_costly_1.cpp',
  install : true
)
e_nonlinear_stencil_costly_1_likwid = executable(
  'e_nonlinear_stencil_costly_1_likwid',
  'e_nonlinear_stencil_costly_1_likwid.cpp',
  install : true
)
e_nonlinear_stencil_precalc_costly_1 = executable(
  'e_nonlinear_stencil_precalc_costly_1',
  'e_nonlinear_stencil_precalc_costly_1.cpp',
  install : true
)

e_nonlinear_stencil_costly_2 = executable(
  'e_nonlinear_stencil_costly_2',
  'e_nonlinear_stencil_costly_2.cpp',
  install : true
)
e_nonlinear_stencil_costly_2_likwid = executable(
  'e_nonlinear_stencil_costly_2_likwid',
  'e_nonlinear_stencil_costly_2_likwid.cpp',
  install : true
)
e_nonlinear_stencil_precalc_costly_2 = executable(
  'e_nonlinear_stencil_precalc_costly_2',
  'e_nonlinear_stencil_precalc_costly_2.cpp',
  install : true
)

e_verify = executable(
  'e_verify',
  'e_verify.cpp',
  install : true
)
e_measure_ops = executable(
  'e_measure_ops',
  'e_measure_ops.cpp',
  install : true
)
