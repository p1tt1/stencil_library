
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

#include <cstddef>
#include <cassert>
#include <iostream>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_COLS_TILING = 8192;
constexpr std::size_t OBJ_ROWS_TILING = 8;
constexpr std::size_t OBJ_LEVELS_TILING = 8;

constexpr std::size_t OBJ_COLS =   65536;
constexpr std::size_t OBJ_ROWS =   256;
constexpr std::size_t OBJ_LEVELS = 256;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RUNS = 1;

#include "c_linear_stencil_tiling.hpp"

template <template<typename ValueType, typename VecType> class OperatorType, typename ValueType, typename VecType>
void routine(std::size_t p_objCols,
             std::size_t p_objRows,
             std::size_t p_objLevels,
             std::size_t p_objColsTiling,
             std::size_t p_objRowsTiling,
             std::size_t p_objLevelsTiling)
{
    std::size_t l_objCells = p_objCols * p_objRows * p_objLevels;

    std::cout << "objCols: " << p_objCols << std::endl;
    std::cout << "objRows: " << p_objRows << std::endl;
    std::cout << "objLevels: " << p_objLevels << std::endl;
    std::cout << "objColsTiling: " << p_objColsTiling << std::endl;
    std::cout << "objRowsTiling: " << p_objRowsTiling << std::endl;
    std::cout << "objLevelsTiling: " << p_objLevelsTiling << std::endl;
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
        p_objColsTiling,
        p_objRowsTiling,
        p_objLevelsTiling,
        C,
        H,
        TAU
    );

    LIKWID_MARKER_START("Apply");
    for (std::size_t i = 0; i < RUNS; ++i)
    {
        l_Op.apply(l_x,l_y);
    }
    LIKWID_MARKER_STOP("Apply");

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

    if(argc != 1 && argc != 7)
    {
        std::cout << "INVALID INPUT: argc must be 0 or 7" << std::endl;
        return 1;
    }

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;
    LIKWID_MARKER_REGISTER("Apply");

    std::size_t l_objCols;
    std::size_t l_objRows;
    std::size_t l_objLevels;

    std::size_t l_objColsTiling;
    std::size_t l_objRowsTiling;
    std::size_t l_objLevelsTiling;

    if(argc == 1)
    {
        l_objCols = OBJ_COLS;
        l_objRows = OBJ_ROWS;
        l_objLevels = OBJ_LEVELS;

        l_objColsTiling = OBJ_COLS_TILING;
        l_objRowsTiling = OBJ_ROWS_TILING;
        l_objLevelsTiling = OBJ_LEVELS_TILING;
    }
    else if (argc == 7)
    {
        l_objCols = atoi(argv[1]);
        l_objRows = atoi(argv[2]);
        l_objLevels = atoi(argv[3]);

        l_objColsTiling = atoi(argv[4]);
        l_objRowsTiling = atoi(argv[5]);
        l_objLevelsTiling = atoi(argv[6]);
    }

    assert(l_objColsTiling%VEC_TYPE::size() == 0);
    assert(l_objCols%l_objColsTiling == 0);
    assert(l_objRows%l_objRowsTiling == 0);
    assert(l_objLevels%l_objLevelsTiling == 0);

    //
    // NOTE: Edges
    //
    l_objCols += 2;
    l_objRows += 2;
    l_objLevels += 2;

    routine<CLinearStencilTiling, VALUE_TYPE, VEC_TYPE>(
        l_objCols,
        l_objRows,
        l_objLevels,
        l_objColsTiling,
        l_objRowsTiling,
        l_objLevelsTiling
    );

    LIKWID_MARKER_CLOSE;

    return 0;
}
