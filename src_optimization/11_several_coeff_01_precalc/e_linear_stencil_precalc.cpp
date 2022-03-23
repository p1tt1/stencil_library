
#include <cstddef>
#include <cassert>
#include <iostream>
#include <omp.h>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_COLS =   2048;
constexpr std::size_t OBJ_ROWS =   2048;
constexpr std::size_t OBJ_LEVELS = 2048;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RND_MAX = 100;
constexpr std::size_t RND_SEED = 1;

constexpr VALUE_TYPE EPSILON_STENCIL = 1e-15;

#include "c_linear_stencil_precalc.hpp"

template <template<typename ValueType, typename VecType> class OperatorType, typename ValueType, typename VecType>
void routine(std::size_t p_objCols,
             std::size_t p_objRows,
             std::size_t p_objLevels
)
{
    std::size_t l_objSize2d = p_objCols * p_objRows;
    std::size_t l_objCells = l_objSize2d * p_objLevels;

    ValueType * l_x_raw = new ValueType[l_objCells+2*l_objSize2d];
    ValueType * l_x = &(l_x_raw[l_objSize2d]);

    ValueType * l_y = new ValueType[l_objCells];

    ValueType * l_c_raw = new ValueType[l_objCells+2*l_objSize2d];
    ValueType * l_c = &(l_c_raw[l_objSize2d]);

    //
    // NOTE: first touch initialization
    //
    #pragma omp parallel
    {
        #pragma omp for
        for (std::size_t i = 0; i < l_objCells; ++i)
        {
            l_x[i] = i;
            // l_x[i] = rand() % RND_MAX;

            l_y[i] = 0;
            l_c[i] = C;
        }

        #pragma omp for
        for (std::size_t i = 0; i < l_objSize2d; ++i)
        {
            l_x_raw[i] = 0;
            l_x_raw[l_objCells+l_objSize2d+i] = 0;

            l_c_raw[i] = 0;
            l_c_raw[l_objCells+l_objSize2d+i] = 0;
        }
    }

    OperatorType<ValueType,VecType> l_Op(
        p_objCols,
        p_objRows,
        p_objLevels,
        l_c,
        H,
        TAU,
        EPSILON_STENCIL
    );

    double l_tStartApply = omp_get_wtime();
    #pragma omp parallel
    {
        l_Op.apply(l_x,l_y);
    }
    double l_tEndApply = omp_get_wtime();
    double l_tApply = l_tEndApply - l_tStartApply;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "OBJ_COLS_IMPL," << p_objCols << std::endl;
    std::cout << "OBJ_ROWS_IMPL," << p_objRows << std::endl;
    std::cout << "OBJ_LEVELS_IMPL," << p_objLevels << std::endl;
    std::cout << "OBJ_CELLS_IMPL," << l_objCells << std::endl;
    std::cout << "IMPL_ID_IMPL," << l_Op.IDENTIFER << std::endl;
    std::cout << "RUNTIME_APPLY_IMPL," << l_tApply << std::endl;

    delete [] l_x_raw;
    delete [] l_y;
    delete [] l_c_raw;
}

int main(int argc, char *argv[])
{
    srand(RND_SEED);
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

    if(argc != 1 && argc != 4)
    {
        std::cout << "INVALID INPUT: argc must be 1 or 4" << std::endl;
        return 1;
    }

    std::size_t l_objCols;
    std::size_t l_objRows;
    std::size_t l_objLevels;

    if (argc == 1)
    {
        l_objCols = OBJ_COLS;
        l_objRows = OBJ_ROWS;
        l_objLevels = OBJ_LEVELS;
    }
    if (argc == 4)
    {
        l_objCols =   atoi(argv[1]);
        l_objRows =   atoi(argv[2]);
        l_objLevels = atoi(argv[3]);
    }
    double l_tStartRoutine = omp_get_wtime();
    routine<CLinearStencilPrecalc, VALUE_TYPE, VEC_TYPE>(
        l_objCols,
        l_objRows,
        l_objLevels
    );
    double l_tEndRoutine = omp_get_wtime();
    double l_tRoutine = l_tEndRoutine - l_tStartRoutine;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "RUNTIME_OVERALL_IMPL," << l_tRoutine << std::endl;

    return 0;
}
