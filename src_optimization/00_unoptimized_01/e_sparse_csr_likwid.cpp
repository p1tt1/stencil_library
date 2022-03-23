
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
#include <iostream>
#include <omp.h>

using VALUE_TYPE = double;

constexpr std::size_t OBJ_COLS =   2048;
constexpr std::size_t OBJ_ROWS =   2048;
constexpr std::size_t OBJ_LEVELS = 2048;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

#include "c_sparse_csr_matrix.hpp"

template <template<typename ValueType> class OperatorType, typename ValueType>
void routine(std::size_t p_objCols,
             std::size_t p_objRows,
             std::size_t p_objLevels
)
{

    std::size_t l_objCells = p_objCols * p_objRows * p_objLevels;

    ValueType * l_x = new ValueType[l_objCells];
    ValueType * l_y = new ValueType[l_objCells];

    for (std::size_t i = 0; i < l_objCells; ++i)
    {
        l_x[i] = i;
    }

    OperatorType<ValueType> l_Op(
        p_objCols,
        p_objRows,
        p_objLevels,
        C,
        H,
        TAU
    );

    double l_tStartApply = omp_get_wtime();
    LIKWID_MARKER_START("Apply");
    l_Op.apply(l_x,l_y);
    LIKWID_MARKER_STOP("Apply");
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
    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;
    LIKWID_MARKER_REGISTER("Apply");

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

    std::size_t l_objCols = OBJ_COLS;
    std::size_t l_objRows = OBJ_ROWS;
    std::size_t l_objLevels = OBJ_LEVELS;
    if (argc == 4)
    {
        l_objCols =   atoi(argv[1]);
        l_objRows =   atoi(argv[2]);
        l_objLevels = atoi(argv[3]);
    }

    double l_tStartRoutine = omp_get_wtime();
    routine<CSparseCSRMatrix, VALUE_TYPE>(
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

    LIKWID_MARKER_CLOSE;
    return 0;
}
