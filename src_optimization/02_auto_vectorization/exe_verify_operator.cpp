
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>

using VALUE_TYPE = double;

constexpr std::size_t S_COLS =   20;
constexpr std::size_t S_ROWS =   20;
constexpr std::size_t S_LEVELS = 20;
constexpr std::size_t S_CELLS = S_ROWS * S_COLS * S_LEVELS;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RND_MAX = 100;

constexpr VALUE_TYPE EPSILON_VERIFY = 1e-7;

#include "c_linear_stencil.hpp"
#include "c_sparse_band_matrix.hpp"
#include "c_sparse_csr_matrix.hpp"

bool equal(VALUE_TYPE * p_v_0, VALUE_TYPE * p_v_1)
{
    for (std::size_t i=0;i<S_LEVELS;++i)
    {
        for (std::size_t j=0;j<S_ROWS;++j)
        {
            for (std::size_t k=0;k<S_COLS;++k)
            {
                std::size_t l_pos = i*S_COLS*S_ROWS+j*S_COLS+k;
                if(std::abs(p_v_0[l_pos] - p_v_1[l_pos]) > EPSILON_VERIFY)
                {
                    std::cout << std::endl;
                    std::cout << "l_pos: " << l_pos << std::endl;
                    std::cout << "Level: " << i << " Col: " << j << " Row: " << k << std::endl;
                    std::cout << p_v_0[l_pos] << " : " << p_v_1[l_pos] << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}

void printStructure(VALUE_TYPE * pArray, std::size_t pCols, std::size_t p_Rows, std::size_t p_Levels)
{
    constexpr std::size_t PRINT_VALUE_GAP = 3;
    for (std::size_t i=0;i<p_Levels;++i)
    {
        std::cout << "Level " << i << std::endl;
        for (std::size_t j=0;j<p_Rows;++j)
        {
            for (std::size_t k=0;k<pCols;++k)
            {
                std::size_t l_pos = i*S_COLS*S_ROWS+j*S_COLS+k;
                std::cout << std::setprecision(3);
                std::cout << std::setw(PRINT_VALUE_GAP) << pArray[l_pos] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

int main()
{

    std::cout << std::endl;
    std::cout << "> init vectors" << std::endl;

    VALUE_TYPE *l_vX = new VALUE_TYPE[S_CELLS];

    VALUE_TYPE *l_vY_linear_stencil = new VALUE_TYPE[S_CELLS];
    VALUE_TYPE *l_vY_sparse_band_matrix = new VALUE_TYPE[S_CELLS];
    VALUE_TYPE *l_vY_sparse_csr_matrix = new VALUE_TYPE[S_CELLS];

    srand(time(NULL));
    for (std::size_t i = 0; i < S_CELLS; ++i)
    {
        l_vX[i] = rand() % RND_MAX;
        // l_vX[i] = i;
        // l_vX[i] = 1;

        // l_vY_linear_stencil[i] = 0;
        // l_vY_sparse_band_matrix[i] = 0;
    }

    std::cout << std::endl;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> c_linear_stencil.apply()" << std::endl;
    CLinearStencil<VALUE_TYPE> l_linear_stencil(
        S_COLS,
        S_ROWS,
        S_LEVELS,
        C,
        H,
        TAU
    );

    l_linear_stencil.apply(l_vX,l_vY_linear_stencil);

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> c_sparse_band_matrix.apply()" << std::endl;
    CSparseBandMatrix<VALUE_TYPE> l_sparse_band_matrix(
        S_COLS,
        S_ROWS,
        S_LEVELS,
        C,
        H,
        TAU
    );

    l_sparse_band_matrix.apply(l_vX,l_vY_sparse_band_matrix);

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> c_sparse_csr_matrix.apply()" << std::endl;
    CSparseCSRMatrix<VALUE_TYPE> l_sparse_csr_matrix(
        S_COLS,
        S_ROWS,
        S_LEVELS,
        C,
        H,
        TAU
    );

    // l_sparse_csr_matrix.print();

    l_sparse_csr_matrix.apply(l_vX,l_vY_sparse_csr_matrix);

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> compare result vectors" << std::endl;

    // std::cout << std::endl;
    // std::cout << "> compare l_vY_linear_stencil with l_vY_sparse_band_matrix" << std::endl;
    // assert(equal(l_vY_linear_stencil, l_vY_sparse_band_matrix) == true);

    std::cout << std::endl;
    std::cout << "> compare l_vY_linear_stencil with l_vY_sparse_csr_matrix" << std::endl;
    assert(equal(l_vY_linear_stencil, l_vY_sparse_csr_matrix) == true);

    std::cout << std::endl;
    std::cout << "> all result vectors are equal" << std::endl;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> delete memory" << std::endl;
    delete [] l_vX;
    delete [] l_vY_linear_stencil;
    delete [] l_vY_sparse_band_matrix;
    delete [] l_vY_sparse_csr_matrix;
}
