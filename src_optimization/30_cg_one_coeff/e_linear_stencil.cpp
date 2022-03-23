
#include <cstddef>
#include <cassert>
#include <iostream>
#include <omp.h>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_CUBE_SIZE_INNER = 2048;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RND_MAX = 100;
constexpr std::size_t RND_SEED = 1;

#include "c_linear_stencil.hpp"

template <template<typename ValueType, typename VecType> class OperatorType, typename ValueType, typename VecType>
void routine(std::size_t p_objCols,
             std::size_t p_objRows,
             std::size_t p_objLevels
)
{
    std::size_t l_objCells = p_objCols * p_objRows * p_objLevels;

    ValueType * l_x = new ValueType[l_objCells];
    ValueType * l_y = new ValueType[l_objCells];

    //
    // NOTE: first touch initialization
    //
    #pragma omp parallel for
    for (std::size_t i = 0; i < l_objCells; ++i)
    {
        l_x[i] = i;
        // l_x[i] = rand() % RND_MAX;
        l_y[i] = 0;
    }

    OperatorType<ValueType,VecType> l_Op(
        p_objCols,
        p_objRows,
        p_objLevels,
        C,
        H,
        TAU
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

    delete [] l_x;
    delete [] l_y;
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

    if(argc != 1 && argc != 2)
    {
        std::cout << "INVALID INPUT: argc must be 1 or 2" << std::endl;
        return 1;
    }

    std::size_t l_objCubeSizeInner = OBJ_CUBE_SIZE_INNER;

    if (argc > 1)
    {
        l_objCubeSizeInner = atoi(argv[1]);
        assert(l_objCubeSizeInner%VEC_TYPE::size() == 0);
    }

    std::size_t l_objCubeSize = l_objCubeSizeInner + 2;

    std::size_t l_objCols =   l_objCubeSize;
    std::size_t l_objRows =   l_objCubeSize;
    std::size_t l_objLevels = l_objCubeSize;

    double l_tStartRoutine = omp_get_wtime();
    routine<CLinearStencil, VALUE_TYPE, VEC_TYPE>(
        l_objCols,
        l_objRows,
        l_objLevels
    );
    double l_tEndRoutine = omp_get_wtime();
    double l_tRoutine = l_tEndRoutine - l_tStartRoutine;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "RUNTIME_ROUTINE_IMPL," << l_tRoutine << std::endl;

    return 0;
}
