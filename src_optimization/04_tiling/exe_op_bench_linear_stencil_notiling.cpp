
#include <cstddef>
#include <cassert>
#include <iostream>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_COLS =   65536;
constexpr std::size_t OBJ_ROWS =   256;
constexpr std::size_t OBJ_LEVELS = 256;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RUNS = 1;

#include "c_linear_stencil_notiling.hpp"

template <template<typename ValueType, typename VecType> class OperatorType, typename ValueType, typename VecType>
void routine(std::size_t p_objCols,
             std::size_t p_objRows,
             std::size_t p_objLevels
)
{
    std::size_t l_objCells = p_objCols * p_objRows * p_objLevels;

    std::cout << "objCols: " << p_objCols << std::endl;
    std::cout << "objRows: " << p_objRows << std::endl;
    std::cout << "objLevels: " << p_objLevels << std::endl;
    std::cout << "objCells: " << l_objCells << std::endl;
    std::cout << std::endl;

    ValueType * l_x = new ValueType[l_objCells];
    ValueType * l_y = new ValueType[l_objCells];

    for (std::size_t i = 0; i < l_objCells; ++i)
    {
        l_x[i] = i;
    }

    OperatorType<ValueType,VecType> l_Op(
        p_objCols,
        p_objRows,
        p_objLevels,
        C,
        H,
        TAU
    );

    for (std::size_t i = 0; i < RUNS; ++i)
    {
        l_Op.apply(l_x,l_y);
    }

    delete [] l_x;
    delete [] l_y;
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

    if(argc != 1 && argc != 4)
    {
        std::cout << "INVALID INPUT: argc must be 0 or 4" << std::endl;
        return 1;
    }

    std::size_t l_objCols;
    std::size_t l_objRows;
    std::size_t l_objLevels;

    if(argc == 1)
    {
        l_objCols = OBJ_COLS;
        l_objRows = OBJ_ROWS;
        l_objLevels = OBJ_LEVELS;
    }
    else if (argc == 4)
    {
        l_objCols = atoi(argv[1]);
        l_objRows = atoi(argv[2]);
        l_objLevels = atoi(argv[3]);
    }

    assert(l_objCols%VEC_TYPE::size() == 0);

    routine<CLinearStencilNotiling, VALUE_TYPE, VEC_TYPE>(
        l_objCols,
        l_objRows,
        l_objLevels
    );

    return 0;
}
