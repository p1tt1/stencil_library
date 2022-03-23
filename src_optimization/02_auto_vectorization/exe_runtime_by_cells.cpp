#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <omp.h>

using VALUE_TYPE = double;

constexpr std::size_t FACTOR_MIN = 10;
constexpr std::size_t FACTOR_MAX = 2000;
constexpr std::size_t FACTOR_STEP = 10;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RND_MAX = 100;

constexpr VALUE_TYPE EPSILON_VERIFY = 1e-7;

constexpr std::size_t RUNS = 1;

constexpr std::size_t NUMBER_OF_THREADS = 1;

std::ofstream g_logFile;

#include "c_linear_stencil.hpp"
#include "c_sparse_band_matrix.hpp"
#include "c_sparse_csr_matrix.hpp"


template <template<typename ValueType> class OperatorType, typename ValueType>
void routine(const std::size_t p_factor)
{
    double l_tStartRoutine = omp_get_wtime();
    std::cout << std::setprecision(4);

    std::size_t l_sCols = p_factor;
    std::size_t l_sRows = p_factor;
    std::size_t l_sLevels = p_factor;

    std::size_t l_sCells = l_sCols * l_sRows * l_sLevels;

    std::size_t l_iter;
    ValueType * l_x = new ValueType[l_sCells];
    ValueType * l_y = new ValueType[l_sCells];

    // srand(time(NULL));
    for (std::size_t i = 0; i < l_sCells; ++i)
    {
        // l_x[i] = rand() % RND_MAX;
        l_x[i] = i;
        // l_x[i] = 1;
    }

    OperatorType<ValueType> l_Op(
        l_sCols,
        l_sRows,
        l_sLevels,
        C,
        H,
        TAU
    );

    std::cout << "> run " << l_Op.IDENTIFER << std::endl;
    std::cout << "p_factor: " << p_factor << std::endl;
    std::cout << "l_sCells: " << (double) l_sCells << std::endl;

    double l_tStartApply = omp_get_wtime();
    for (std::size_t i = 0; i < RUNS; ++i)
    {
        l_Op.apply(l_x,l_y);
    }
    double l_tEndApply = omp_get_wtime();

    delete [] l_x;
    delete [] l_y;

    double l_tEndRoutine = omp_get_wtime();


    double l_tApply = l_tEndApply - l_tStartApply;
    double l_tRoutine = l_tEndRoutine - l_tStartRoutine;

    std::cout << "runtime Apply: " << l_tApply << std::endl;
    std::cout << "runtime Routine: " << l_tRoutine << std::endl;
    g_logFile   << std::setprecision(16)
                << "time_apply"      << ","
                << l_Op.IDENTIFER    << ","
                << l_sCells          << ","
                << p_factor          << ","
                << NUMBER_OF_THREADS << ","
                << l_tApply          << std::endl;
    g_logFile   << std::setprecision(16)
                << "time_routine"    << ","
                << l_Op.IDENTIFER    << ","
                << l_sCells          << ","
                << p_factor          << ","
                << NUMBER_OF_THREADS << ","
                << l_tRoutine        << std::endl;
}

int main()
{
    std::cout << std::setprecision(4);

    std::cout << "NUMBER_OF_THREADS: " << NUMBER_OF_THREADS << std::endl;

    g_logFile.open("./plot.csv", std::ofstream::trunc);
    g_logFile << "time_id,impl_id,n_cells,factor,n_threads,runtime" << std::endl;
    g_logFile.close();

    std::cout << "=============================" << std::endl;

    for (std::size_t i = FACTOR_MIN; i <= FACTOR_MAX; i += FACTOR_STEP)
    {
        //
        // WORKAROUND plot immediately
        // flush buffer had not worked
        //
        g_logFile.open("./plot.csv", std::ofstream::app);

        routine<CLinearStencil, VALUE_TYPE>(i);
        std::cout << "-----------------------------" << std::endl;

        routine<CSparseBandMatrix, VALUE_TYPE>(i);
        std::cout << "-----------------------------" << std::endl;

        routine<CSparseCSRMatrix, VALUE_TYPE>(i);
        std::cout << "-----------------------------" << std::endl;

        g_logFile.close();
        std::cout << "=============================" << std::endl;
    }

    return 0;
}
