#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <omp.h>

#include <cstddef>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

// constexpr std::size_t OBJ_COLS_TILINGS_LENGTH = 2;
// constexpr std::size_t OBJ_COLS_TILINGS[OBJ_COLS_TILINGS_LENGTH] = {32768, 65536};
constexpr std::size_t OBJ_COLS_TILINGS_LENGTH = 17;
constexpr std::size_t OBJ_COLS_TILINGS[OBJ_COLS_TILINGS_LENGTH] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};

constexpr std::size_t OBJ_LEVELS_ROWS_TILINGS_LENGTH = 9;
constexpr std::size_t OBJ_LEVELS_ROWS_TILINGS[OBJ_LEVELS_ROWS_TILINGS_LENGTH] = {1, 2, 4, 8, 16, 32, 64, 128, 256};

constexpr std::size_t OBJ_FACTOR = 1;

// constexpr std::size_t OBJ_CUBE_SIZE = (OBJ_FACTOR * OBJ_COLS_TILINGS[OBJ_COLS_TILINGS_LENGTH-1]) + 2;
constexpr std::size_t OBJ_COLS = (OBJ_FACTOR * OBJ_COLS_TILINGS[OBJ_COLS_TILINGS_LENGTH-1]) + 2;
constexpr std::size_t OBJ_ROWS = (OBJ_FACTOR * OBJ_LEVELS_ROWS_TILINGS[OBJ_LEVELS_ROWS_TILINGS_LENGTH-1]) + 2;
constexpr std::size_t OBJ_LEVELS = (OBJ_FACTOR * OBJ_LEVELS_ROWS_TILINGS[OBJ_LEVELS_ROWS_TILINGS_LENGTH-1]) + 2;

constexpr std::size_t OBJ_CELLS = OBJ_COLS * OBJ_ROWS * OBJ_LEVELS;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RND_MAX = 100;

constexpr VALUE_TYPE EPSILON_VERIFY = 1e-7;

constexpr std::size_t RUNS = 1;

std::ofstream g_logFile;

std::string OpIdentiferNotiling;
double tApplyNoTiling = -1.0;

#include "c_linear_stencil_tiling.hpp"
#include "c_linear_stencil_notiling.hpp"

template <template<typename ValueType, typename VecType> class TilingOperatorType, typename ValueType, typename VecType>
void routine_tiling(const std::size_t p_tiling_levels_rows, const std::size_t p_tiling_cols)
{
    std::cout << "> run routine_tiling" << std::endl;
    std::cout << "p_tiling_levels_rows: " << p_tiling_levels_rows << std::endl;
    std::cout << "p_tiling_cols: " << p_tiling_cols << std::endl;
    std::cout << "OBJ_CELLS: " << (double) OBJ_CELLS << std::endl;

    assert(p_tiling_cols%VEC_TYPE::size() == 0);
    assert(OBJ_COLS%p_tiling_cols == 0);
    assert(OBJ_ROWS%p_tiling_levels_rows == 0);
    assert(OBJ_LEVELS%p_tiling_levels_rows == 0);

    std::cout << std::setprecision(4);

    std::size_t l_iter;
    ValueType * l_x = new ValueType[OBJ_CELLS];
    ValueType * l_y = new ValueType[OBJ_CELLS];

    // srand(time(NULL));
    for (std::size_t i = 0; i < OBJ_CELLS; ++i)
    {
        // l_x[i] = rand() % RND_MAX;
        l_x[i] = i;
        // l_x[i] = 1;
    }

    TilingOperatorType<ValueType,VecType> l_Op(
        OBJ_COLS,
        OBJ_ROWS,
        OBJ_LEVELS,
        p_tiling_cols,
        p_tiling_levels_rows,
        p_tiling_levels_rows,
        C,
        H,
        TAU
    );

    double l_tStartApply = omp_get_wtime();
    for (std::size_t i = 0; i < RUNS; ++i)
    {
        l_Op.apply(l_x,l_y);
    }
    double l_tEndApply = omp_get_wtime();
    double l_tApply = l_tEndApply - l_tStartApply;

    std::cout << "runtime Apply: " << l_tApply << std::endl;

    g_logFile   << std::setprecision(16)
                << "time_apply"         << ","
                << l_Op.IDENTIFER       << ","
                << OBJ_CELLS           << ","
                << OBJ_FACTOR           << ","
                << p_tiling_levels_rows << ","
                << p_tiling_cols        << ","
                << l_tApply             << std::endl;

    delete [] l_x;
    delete [] l_y;

}

template <template<typename ValueType, typename VecType> class NotilingOperatorType, typename ValueType, typename VecType>
void routine_notiling(const std::size_t p_tiling_levels_rows, const std::size_t p_tiling_cols)
{
    std::cout << "> run routine_notiling" << std::endl;
    std::cout << "p_tiling_levels_rows: " << p_tiling_levels_rows << std::endl;
    std::cout << "p_tiling_cols: " << p_tiling_cols << std::endl;
    std::cout << "OBJ_CELLS: " << (double) OBJ_CELLS << std::endl;

    if (tApplyNoTiling == -1.0)
    {
        std::cout << std::setprecision(4);

        std::size_t l_iter;
        ValueType * l_x = new ValueType[OBJ_CELLS];
        ValueType * l_y = new ValueType[OBJ_CELLS];

        // srand(time(NULL));
        for (std::size_t i = 0; i < OBJ_CELLS; ++i)
        {
            // l_x[i] = rand() % RND_MAX;
            l_x[i] = i;
            // l_x[i] = 1;
        }

        NotilingOperatorType<ValueType,VecType> l_Op(
            OBJ_COLS,
            OBJ_ROWS,
            OBJ_LEVELS,
            C,
            H,
            TAU
        );

        double l_tStartApply = omp_get_wtime();
        for (std::size_t i = 0; i < RUNS; ++i)
        {
            l_Op.apply(l_x,l_y);
        }
        double l_tEndApply = omp_get_wtime();

        OpIdentiferNotiling = l_Op.IDENTIFER;
        tApplyNoTiling = l_tEndApply - l_tStartApply;

        delete [] l_x;
        delete [] l_y;
    }

    std::cout << "runtime Apply: " << tApplyNoTiling << std::endl;

    g_logFile   << std::setprecision(16)
                << "time_apply"         << ","
                << OpIdentiferNotiling  << ","
                << OBJ_CELLS            << ","
                << OBJ_FACTOR           << ","
                << p_tiling_levels_rows << ","
                << p_tiling_cols        << ","
                << tApplyNoTiling       << std::endl;
}

int main()
{
    std::cout << std::setprecision(4);

    std::cout << "OBJ_FACTOR: " << OBJ_FACTOR << std::endl;

    g_logFile.open("./plot.csv", std::ofstream::trunc);
    g_logFile << "time_id,impl_id,n_cells,factor,n_tiling_levels_rows,n_tiling_cols,runtime" << std::endl;
    g_logFile.close();

    std::cout << "=============================" << std::endl;

    for (std::size_t j=0; j <OBJ_COLS_TILINGS_LENGTH; ++j)
    {
        //
        // WORKAROUND plot immediately
        // flush buffer had not worked
        //
        g_logFile.open("./plot.csv", std::ofstream::app);

        routine_notiling<CLinearStencilNotiling, VALUE_TYPE, VEC_TYPE>(0,OBJ_COLS_TILINGS[j]);

        std::cout << "=============================" << std::endl;

        g_logFile.close();
    }

    for (std::size_t i=0; i <OBJ_LEVELS_ROWS_TILINGS_LENGTH; ++i)
    {
        for (std::size_t j=0; j <OBJ_COLS_TILINGS_LENGTH; ++j)
        {
            //
            // WORKAROUND plot immediately
            // flush buffer had not worked
            //
            g_logFile.open("./plot.csv", std::ofstream::app);

            routine_tiling<CLinearStencilTiling, VALUE_TYPE, VEC_TYPE>(OBJ_LEVELS_ROWS_TILINGS[i],OBJ_COLS_TILINGS[j]);

            std::cout << "=============================" << std::endl;

            g_logFile.close();
        }
    }

    return 0;
}
