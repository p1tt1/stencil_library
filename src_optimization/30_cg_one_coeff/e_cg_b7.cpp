
#include <cstddef>
#include <cassert>
#include <iostream>
#include <omp.h>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_COLS =   512;
constexpr std::size_t OBJ_ROWS =   512;
constexpr std::size_t OBJ_LEVELS = 512;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr VALUE_TYPE EPSILON_SOLVER = 1e-15;
constexpr std::size_t ITER_MAX = 1000000;


#include "c_stencil.hpp"
#include "c_cg_b7.hpp"

template <template<typename ValueType> class SolverType, typename ValueType>
void routine(std::size_t p_objCols,
             std::size_t p_objRows,
             std::size_t p_objLevels,
             std::size_t p_iterMax
)
{
    std::size_t l_objSize2d = p_objCols * p_objRows;
    std::size_t l_objCells = l_objSize2d * p_objLevels;

    SolverType<ValueType> l_solver;

    CStencil<ValueType,VEC_TYPE> l_stencil(
        p_objCols,
        p_objRows,
        p_objLevels,
        C,
        H,
        TAU
    );

    VALUE_TYPE * l_x0_raw = new VALUE_TYPE[l_objCells+2*l_objSize2d];
    VALUE_TYPE * l_x0 = &(l_x0_raw[l_objSize2d]);
    VALUE_TYPE * l_x1 = new VALUE_TYPE[l_objCells];
    VALUE_TYPE * l_b = new VALUE_TYPE[l_objCells];

    std::size_t l_iter;

    //
    // NOTE: first touch initialization
    //
    #pragma omp parallel
    {
        #pragma omp for
        for (std::size_t i = 0; i < l_objCells; ++i)
        {
            l_x0[i] = 0;
            l_x1[i] = 0;
            l_b[i] = i;
        }

        #pragma omp for
        for (std::size_t i = 0; i < l_objSize2d; ++i)
        {
            l_x0_raw[i] = 0;
            l_x0_raw[l_objCells+l_objSize2d+i] = 0;
        }
    }

    double l_tStartApply = omp_get_wtime();
    l_iter = l_solver(
        l_objCells,
        l_stencil,
        l_x0,
        l_b,
        l_x1,
        EPSILON_SOLVER,
        p_iterMax,
        l_objSize2d
    );
    double l_tEndApply = omp_get_wtime();
    double l_tApply = l_tEndApply - l_tStartApply;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "OBJ_COLS_IMPL," << p_objCols << std::endl;
    std::cout << "OBJ_ROWS_IMPL," << p_objRows << std::endl;
    std::cout << "OBJ_LEVELS_IMPL," << p_objLevels << std::endl;
    std::cout << "OBJ_CELLS_IMPL," << l_objCells << std::endl;
    std::cout << "STENCIL_ID_IMPL," << l_stencil.IDENTIFER << std::endl;
    std::cout << "SOLVER_ID_IMPL," << l_solver.IDENTIFER << std::endl;
    std::cout << "EPSILON_SOLVER_IMPL," << EPSILON_SOLVER << std::endl;
    std::cout << "ITER_MAX_IMPL," << p_iterMax << std::endl;
    std::cout << "ITER_IMPL," << l_iter << std::endl;
    std::cout << "RUNTIME_APPLY_IMPL," << l_tApply << std::endl;

    delete [] l_x0_raw;
    delete [] l_x1;
    delete [] l_b;
}

int main(int argc, char *argv[])
{
    std::cout << "argc: " << argc << std::endl;
    std::cout << "argv: [";
    for (std::size_t i = 0; i < argc; ++i)
    {
        std::cout << argv[i];
        if (i < argc - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    if(argc != 1 && argc != 4 && argc != 5)
    {
        std::cout << "INVALID INPUT: argc must be 1 or 4 or 5" << std::endl;
        return 1;
    }

    std::size_t l_objCols = OBJ_COLS;
    std::size_t l_objRows = OBJ_ROWS;
    std::size_t l_objLevels = OBJ_LEVELS;
    std::size_t l_iterMax = ITER_MAX;

    if (argc >= 4)
    {
        l_objCols =   atoi(argv[1]);
        l_objRows =   atoi(argv[2]);
        l_objLevels = atoi(argv[3]);
    }
    if (argc == 5)
    {
        l_iterMax =   atoi(argv[4]);
    }

    double l_tStartRoutine = omp_get_wtime();
    routine<C_CG_B7, VALUE_TYPE>(
        l_objCols,
        l_objRows,
        l_objLevels,
        l_iterMax
    );
    double l_tEndRoutine = omp_get_wtime();
    double l_tRoutine = l_tEndRoutine - l_tStartRoutine;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "RUNTIME_ROUTINE_IMPL," << l_tRoutine << std::endl;

    return 0;
}
