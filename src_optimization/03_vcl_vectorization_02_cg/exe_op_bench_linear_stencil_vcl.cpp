// This block enables to compile the code with and without the likwid header in place
#ifdef LIKWID_PERFMON
#include <likwid-marker.h>
#else
#define LIKWID_MARKER_INIT
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_SWITCH
#define LIKWID_MARKER_REGISTER(regionTag)
#define LIKWID_MARKER_START(regionTag)
#define LIKWID_MARKER_STOP(regionTag)
#define LIKWID_MARKER_CLOSE
#define LIKWID_MARKER_GET(regionTag, nevents, events, time, count)
#endif

#include <cassert>
#include <omp.h>

using VALUE_TYPE = double;

constexpr std::size_t MEMORY_ALIGNMENT = 4;
constexpr std::size_t VEC_SIZE = 4;

constexpr std::size_t S_COLS_TILING = 32;
constexpr std::size_t S_ROWS_TILING = 2;
constexpr std::size_t S_LEVELS_TILING = 2;

constexpr std::size_t FACTOR = 20;

constexpr std::size_t ITER_MAX = 100000;
// constexpr std::size_t ITER_MAX = 1;
constexpr VALUE_TYPE EPSILON = 1e-15;
constexpr std::size_t RUNS = 1;

constexpr VALUE_TYPE DIFF_COEFF = 0.5;

constexpr std::size_t NUMBER_OF_THREADS = 1;
// constexpr std::size_t NUMBER_OF_THREADS = 1;

#include "c_stencil_diff_3d_03_oc_ne_vcl.hpp"
#include "c_sol_cg_01_buffer.hpp"

template <template<typename ValueType> class SolverType, template<typename ValueType> class OperatorType, typename ValueType>
void routine()
{
    SolverType<ValueType> l_solver;

    std::size_t l_sCols = FACTOR * S_COLS_TILING;
    std::size_t l_sRows = FACTOR * S_COLS_TILING;
    std::size_t l_sLevels = FACTOR * S_COLS_TILING;
    // std::size_t l_sRows = FACTOR * S_ROWS_TILING;
    // std::size_t l_sLevels = FACTOR * S_LEVELS_TILING;

    std::size_t l_sSize2d = l_sCols * l_sRows;
    std::size_t l_sCells = l_sSize2d * l_sLevels;

    std::size_t l_iter;
    ValueType * l_vX_0_raw = new (std::align_val_t(MEMORY_ALIGNMENT)) ValueType[l_sCells+2*l_sSize2d];
    ValueType * l_vX_0 = &(l_vX_0_raw[l_sSize2d]);
    ValueType * l_vX_1 = new (std::align_val_t(MEMORY_ALIGNMENT)) ValueType[l_sCells];
    ValueType * l_vB_0 = new (std::align_val_t(MEMORY_ALIGNMENT)) ValueType[l_sCells];

    //
    // NOTE: first touch initialization
    //
    #pragma omp parallel for
    for (std::size_t i = 0; i < l_sCells; ++i)
    {
        l_vX_0[i] = 0;
        l_vX_1[i] = 0;

        l_vB_0[i] = i;
    }

    for (std::size_t i = 0; i < l_sSize2d; ++i)
    {
        l_vX_0_raw[i] = 0;
        l_vX_0_raw[l_sCells+l_sSize2d+i] = 0;
    }

    OperatorType<ValueType> l_Op(
        l_sCols,
        l_sRows,
        l_sLevels,
        S_COLS_TILING,
        S_ROWS_TILING,
        S_LEVELS_TILING,
        DIFF_COEFF
    );

    LIKWID_MARKER_START("Apply");
    for (std::size_t i = 0; i < RUNS; ++i)
    {
        l_iter = l_solver(l_sCells, l_Op, l_vX_0, l_vB_0, l_vX_1, EPSILON, ITER_MAX, l_sSize2d);
    }
    LIKWID_MARKER_STOP("Apply");

    delete [] l_vX_0_raw;
    delete [] l_vX_1;
    delete [] l_vB_0;
}

int main()
{
    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;
    LIKWID_MARKER_REGISTER("Apply");
    omp_set_num_threads(NUMBER_OF_THREADS);

    routine<C_Sol_CG_01_Buffer, CStencilDiff3D_03_Oc_Ne_Vcl, VALUE_TYPE>();

    LIKWID_MARKER_CLOSE;
    return 0;
}
