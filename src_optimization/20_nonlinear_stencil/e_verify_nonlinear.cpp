
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

constexpr VALUE_TYPE C = 0.5;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RND_MAX = 100;

constexpr VALUE_TYPE EPSILON_VERIFY = 1e-7;
constexpr VALUE_TYPE EPSILON_STENCIL = 1e-15;

constexpr std::size_t NUMBER_OF_THREADS = 64;

#include "c_nonlinear_stencil.hpp"
#include "c_nonlinear_stencil_01.hpp"
#include "s_state_function_none.hpp"
#include "s_state_function_mul2.hpp"
#include "c_state_function_none.hpp"
#include "c_state_function_mul2.hpp"

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

    omp_set_num_threads(NUMBER_OF_THREADS);

    std::cout << std::endl;
    std::cout << "> init vectors" << std::endl;

    VALUE_TYPE * l_x_raw = new VALUE_TYPE[OBJ_CELLS+2*OBJ_SIZE_2D];
    VALUE_TYPE *l_x = &(l_x_raw[OBJ_SIZE_2D]);

    VALUE_TYPE *l_y_nonlinear_00 = new VALUE_TYPE[OBJ_CELLS];
    VALUE_TYPE *l_y_nonlinear_01 = new VALUE_TYPE[OBJ_CELLS];

    VALUE_TYPE * l_c_raw = new VALUE_TYPE[OBJ_CELLS+2*OBJ_SIZE_2D];
    VALUE_TYPE *l_c = &(l_c_raw[OBJ_SIZE_2D]);


    srand(time(NULL));
    for (std::size_t i = 0; i < OBJ_CELLS; ++i)
    {
        // l_x[i] = rand() % RND_MAX;
        // l_x[i] = i;
        l_x[i] = 1;

        // l_y_current[i] = 0;
        // l_y_nonlinear_00[i] = 0;

        l_c[i] = C;
    }
    for (std::size_t i = 0; i < OBJ_SIZE_2D; ++i)
    {
        l_x_raw[i] = 0;
        l_x_raw[OBJ_CELLS+OBJ_SIZE_2D+i] = 0;

        l_c_raw[i] = 0;
        l_c_raw[OBJ_CELLS+OBJ_SIZE_2D+i] = 0;
    }

    std::cout << std::endl;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> nonlinear_00:apply()" << std::endl;

    // CNonlinearStencil<SSF_none,VALUE_TYPE,VEC_TYPE> l_nonlinear_00(
    CNonlinearStencil<SSF_mul2,VALUE_TYPE,VEC_TYPE> l_nonlinear_00(
        OBJ_COLS,
        OBJ_ROWS,
        OBJ_LEVELS,
        l_c,
        H,
        TAU,
        EPSILON_STENCIL
    );

    l_nonlinear_00.apply(l_x,l_y_nonlinear_00);

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> nonlinear_01:apply()" << std::endl;

    // CNonlinearStencil01<CStateFunctionNone,VALUE_TYPE,VEC_TYPE> l_nonlinear_01(
    CNonlinearStencil01<CStateFunctionMul2,VALUE_TYPE,VEC_TYPE> l_nonlinear_01(
        OBJ_COLS,
        OBJ_ROWS,
        OBJ_LEVELS,
        l_c,
        H,
        TAU,
        EPSILON_STENCIL
    );

    l_nonlinear_01.apply(l_x,l_y_nonlinear_01);

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> compare result vectors" << std::endl;

    std::cout << std::endl;
    std::cout << "> compare l_y_nonlinear_00 with l_y_nonlinear_01" << std::endl;
    assert(equal(l_y_nonlinear_00, l_y_nonlinear_01) == true);

    std::cout << std::endl;
    std::cout << "> all result vectors are equal" << std::endl;

    // -------------------------------------------------------------------------------------------------

    std::cout << std::endl;
    std::cout << "> delete memory" << std::endl;
    delete [] l_x_raw;
    delete [] l_y_nonlinear_00;
    delete [] l_y_nonlinear_01;
    delete [] l_c_raw;
}
