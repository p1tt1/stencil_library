
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>

#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_LEVELS_TILING = 2;
constexpr std::size_t OBJ_ROWS_TILING = 2;
constexpr std::size_t OBJ_COLS_TILING = 32;

constexpr std::size_t S_FACTOR =   5;

constexpr std::size_t S_CUBE_SIZE = (S_FACTOR * OBJ_COLS_TILING) + 2;

constexpr std::size_t OBJ_COLS =   S_CUBE_SIZE;
constexpr std::size_t OBJ_ROWS =   S_CUBE_SIZE;
constexpr std::size_t OBJ_LEVELS = S_CUBE_SIZE;
constexpr std::size_t OBJ_CELLS =  OBJ_ROWS * OBJ_COLS * OBJ_LEVELS;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RND_MAX = 100;

constexpr VALUE_TYPE EPSILON_VERIFY = 1e-7;

#include "c_linear_stencil_notiling.hpp"
#include "c_linear_stencil_tiling.hpp"

bool equal(VALUE_TYPE * p_v_0, VALUE_TYPE * p_v_1)
{
    for (std::size_t i=0;i<OBJ_LEVELS;++i)
    {
        for (std::size_t j=0;j<OBJ_ROWS;++j)
        {
            for (std::size_t k=0;k<OBJ_COLS;++k)
            {
                std::size_t l_pos = i*OBJ_COLS*OBJ_ROWS+j*OBJ_COLS+k;
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
                std::size_t l_pos = i*OBJ_COLS*OBJ_ROWS+j*OBJ_COLS+k;
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
    assert(OBJ_COLS_TILING%VEC_TYPE::size() == 0);
    assert(OBJ_COLS_TILING%OBJ_ROWS_TILING == 0);
    assert(OBJ_COLS_TILING%OBJ_LEVELS_TILING == 0);

    std::cout << std::endl;
    std::cout << "> init vectors" << std::endl;

    VALUE_TYPE *l_vX = new VALUE_TYPE[OBJ_CELLS];

    VALUE_TYPE *l_vY_linear_stencil_notiling = new VALUE_TYPE[OBJ_CELLS];
    VALUE_TYPE *l_vY_linear_stencil_tiling = new VALUE_TYPE[OBJ_CELLS];

    srand(time(NULL));
    for (std::size_t i = 0; i < OBJ_CELLS; ++i)
    {
        l_vX[i] = rand() % RND_MAX;
        // l_vX[i] = i;
        // l_vX[i] = 1;

        // l_vY_linear_stencil_notiling[i] = 0;
        // l_vY_linear_stencil_tiling[i] = 0;
    }

    std::cout << std::endl;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> c_linear_stencil_notiling.apply()" << std::endl;
    CLinearStencilNotiling<VALUE_TYPE,VEC_TYPE> l_linear_stencil_notiling(
        OBJ_COLS,
        OBJ_ROWS,
        OBJ_LEVELS,
        C,
        H,
        TAU
    );

    l_linear_stencil_notiling.apply(l_vX,l_vY_linear_stencil_notiling);

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> c_linear_stencil_tiling.apply()" << std::endl;
    CLinearStencilTiling<VALUE_TYPE,VEC_TYPE> l_linear_stencil_tiling(
        OBJ_COLS,
        OBJ_ROWS,
        OBJ_LEVELS,
        OBJ_COLS_TILING,
        OBJ_ROWS_TILING,
        OBJ_LEVELS_TILING,
        C,
        H,
        TAU
    );

    l_linear_stencil_tiling.apply(l_vX,l_vY_linear_stencil_tiling);

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> compare result vectors" << std::endl;

    std::cout << std::endl;
    std::cout << "> compare l_vY_linear_stencil_notiling with l_vY_linear_stencil_tiling" << std::endl;
    assert(equal(l_vY_linear_stencil_notiling, l_vY_linear_stencil_tiling) == true);

    std::cout << std::endl;
    std::cout << "> all result vectors are equal" << std::endl;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> delete memory" << std::endl;
    delete [] l_vX;
    delete [] l_vY_linear_stencil_notiling;
    delete [] l_vY_linear_stencil_tiling;
}
