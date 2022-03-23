
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <omp.h>

#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_CUBE_SIZE = 512;

constexpr std::size_t OBJ_COLS =   OBJ_CUBE_SIZE;
constexpr std::size_t OBJ_ROWS =   OBJ_CUBE_SIZE;
constexpr std::size_t OBJ_LEVELS = OBJ_CUBE_SIZE;

constexpr std::size_t OBJ_SIZE_2D = OBJ_ROWS * OBJ_COLS;
constexpr std::size_t OBJ_CELLS =  OBJ_SIZE_2D * OBJ_LEVELS;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RND_MAX = 100;

constexpr VALUE_TYPE EPSILON_VERIFY = 1e-7;
constexpr VALUE_TYPE EPSILON_SOLVER = 1e-15;
constexpr std::size_t ITER_MAX = 1000000;

constexpr std::size_t NUMBER_OF_THREADS = 128;

#include "c_stencil.hpp"
#include "c_cg_b7.hpp"
#include "c_cg_b5.hpp"
#include "c_cg_b4.hpp"

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

    omp_set_num_threads(NUMBER_OF_THREADS);

    std::cout << std::endl;
    std::cout << "> init components" << std::endl;

    CStencil<VALUE_TYPE,VEC_TYPE> l_stencil(
        OBJ_COLS,
        OBJ_ROWS,
        OBJ_LEVELS,
        C,
        H,
        TAU
    );

    VALUE_TYPE * l_x0_raw = new VALUE_TYPE[OBJ_CELLS+2*OBJ_SIZE_2D];
    VALUE_TYPE * l_x0 = &(l_x0_raw[OBJ_SIZE_2D]);

    VALUE_TYPE * l_b = new VALUE_TYPE[OBJ_CELLS];

    for (std::size_t i = 0; i < OBJ_CELLS; ++i)
    {
        l_x0[i] = 0;
        l_b[i] = i;
    }
    for (std::size_t i = 0; i < OBJ_SIZE_2D; ++i)
    {
        l_x0_raw[i] = 0;
        l_x0_raw[OBJ_CELLS+OBJ_SIZE_2D+i] = 0;
    }


    std::cout << std::endl;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> c_cg_b7::apply()" << std::endl;

    C_CG_B7<VALUE_TYPE> l_cg_b7;

    VALUE_TYPE * l_x1_cg_b7 = new VALUE_TYPE[OBJ_CELLS];
    for (std::size_t i = 0; i < OBJ_CELLS; ++i)
    {
        l_x1_cg_b7[i] = 0;
    }

    std::size_t l_iter_cg_b7 = l_cg_b7(
        OBJ_CELLS,
        l_stencil,
        l_x0,
        l_b,
        l_x1_cg_b7,
        EPSILON_SOLVER,
        ITER_MAX,
        OBJ_SIZE_2D
    );

    delete [] l_x1_cg_b7;

    // // -------------------------------------------------------------------------------------------------

    // std::cout << std::endl;
    // std::cout << "> c_cg_b5::apply()" << std::endl;
    // C_CG_B5<VALUE_TYPE> l_cg_b5;

    // VALUE_TYPE * l_x1_cg_b5 = new VALUE_TYPE[OBJ_CELLS];
    // for (std::size_t i = 0; i < OBJ_CELLS; ++i)
    // {
    //     l_x1_cg_b5[i] = 0;
    // }

    // std::size_t l_iter_cg_b5 = l_cg_b5(
    //     OBJ_CELLS,
    //     l_stencil,
    //     l_x0,
    //     l_b,
    //     l_x1_cg_b5,
    //     EPSILON_SOLVER,
    //     ITER_MAX,
    //     OBJ_SIZE_2D
    // );

    // delete [] l_x1_cg_b5;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> c_cg_b4::apply()" << std::endl;
    C_CG_B4<VALUE_TYPE> l_cg_b4;

    VALUE_TYPE * l_x1_cg_b4 = new VALUE_TYPE[OBJ_CELLS];
    for (std::size_t i = 0; i < OBJ_CELLS; ++i)
    {
        l_x1_cg_b4[i] = 0;
    }

    std::size_t l_iter_cg_b4 = l_cg_b4(
        OBJ_CELLS,
        l_stencil,
        l_x0,
        l_b,
        l_x1_cg_b4,
        EPSILON_SOLVER,
        ITER_MAX,
        OBJ_SIZE_2D
    );

    delete [] l_x1_cg_b4;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> compare iterations" << std::endl;

    std::cout << "l_iter_cg_b7: " << l_iter_cg_b7 << std::endl;
    // std::cout << "l_iter_cg_b5: " << l_iter_cg_b5 << std::endl;
    std::cout << "l_iter_cg_b4: " << l_iter_cg_b4 << std::endl;

    // assert(l_iter_cg_b7 == l_iter_cg_b5);
    assert(l_iter_cg_b7 == l_iter_cg_b4);

    std::cout << std::endl;
    std::cout << "> all iterations are equal" << std::endl;

    // -------------------------------------------------------------------------------------------------

    delete [] l_x0_raw;
    delete [] l_b;
}
