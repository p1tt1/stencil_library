
#include <cstddef>
#include <cassert>
#include <iostream>
#include <omp.h>
#include <cmath>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_COLS =   512;
constexpr std::size_t OBJ_ROWS =   512;
constexpr std::size_t OBJ_LEVELS = 512;

// constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr VALUE_TYPE EPSILON = 1e-15;
constexpr std::size_t ITER_MAX = 1000000;


#include "c_stencil_precalc.hpp"
#include "c_cg.hpp"

template <template<typename ValueType> class SolverType, template<typename ValueType, typename VecType> class OperatorType, typename ValueType, typename VecType>
void routine(std::size_t p_objCols,
             std::size_t p_objRows,
             std::size_t p_objLevels,
             VALUE_TYPE p_epsilon
)
{
    std::size_t l_objSize2d = p_objCols * p_objRows;
    std::size_t l_objCells = l_objSize2d * p_objLevels;

    SolverType<ValueType> l_solver;

    VALUE_TYPE * l_x0_raw = new VALUE_TYPE[l_objCells+2*l_objSize2d];
    VALUE_TYPE * l_x0 = &(l_x0_raw[l_objSize2d]);

    VALUE_TYPE * l_x1 = new VALUE_TYPE[l_objCells];
    VALUE_TYPE * l_b = new VALUE_TYPE[l_objCells];

    ValueType * l_c_raw = new ValueType[l_objCells+2*l_objSize2d];
    ValueType * l_c = &(l_c_raw[l_objSize2d]);

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
            // l_c[i] = l_objCells-i;
            // l_c[i] = 0.5;
            l_c[i] = 0.5+i*1e-8;
        }

        #pragma omp for
        for (std::size_t i = 0; i < l_objSize2d; ++i)
        {
            l_x0_raw[i] = 0;
            l_x0_raw[l_objCells+l_objSize2d+i] = 0;

            l_c_raw[i] = 0;
            l_c_raw[l_objCells+l_objSize2d+i] = 0;
        }
    }

    OperatorType<ValueType,VEC_TYPE> l_stencil(
        p_objCols,
        p_objRows,
        p_objLevels,
        l_c,
        H,
        TAU
    );

    double l_tStartApply = omp_get_wtime();
    l_iter = l_solver(
        l_objCells,
        l_stencil,
        l_x0,
        l_b,
        l_x1,
        p_epsilon,
        ITER_MAX,
        l_objSize2d
    );
    double l_tEndApply = omp_get_wtime();
    double l_tApply = l_tEndApply - l_tStartApply;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "IMPL_ID_IMPL," << l_stencil.IDENTIFER << std::endl;
    std::cout << "OBJ_COLS_IMPL," << p_objCols << std::endl;
    std::cout << "OBJ_ROWS_IMPL," << p_objRows << std::endl;
    std::cout << "OBJ_LEVELS_IMPL," << p_objLevels << std::endl;
    std::cout << "OBJ_CELLS_IMPL," << l_objCells << std::endl;
    std::cout << "STENCIL_ID_IMPL," << l_stencil.IDENTIFER << std::endl;
    std::cout << "SOLVER_ID_IMPL," << l_solver.IDENTIFER << std::endl;
    std::cout << "EPSILON_IMPL," << p_epsilon << std::endl;
    std::cout << "ITER_MAX_IMPL," << ITER_MAX << std::endl;
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
    VALUE_TYPE epsilon = EPSILON;

    if (argc >= 4)
    {
        l_objCols =   atoi(argv[1]);
        l_objRows =   atoi(argv[2]);
        l_objLevels = atoi(argv[3]);
    }
    if (argc == 5)
    {
        epsilon =   1.0*std::pow(10,-1* atoi(argv[4]));
    }

    double l_tStartRoutine = omp_get_wtime();
    routine<C_CG, CStencilPrecalc, VALUE_TYPE, VEC_TYPE>(
        l_objCols,
        l_objRows,
        l_objLevels,
        epsilon
    );
    double l_tEndRoutine = omp_get_wtime();
    double l_tRoutine = l_tEndRoutine - l_tStartRoutine;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "RUNTIME_OVERALL_IMPL," << l_tRoutine << std::endl;

    return 0;
}
