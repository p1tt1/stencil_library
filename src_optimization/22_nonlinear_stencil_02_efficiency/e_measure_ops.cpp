
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
#include <omp.h>
#include <iostream>
#include <iomanip>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_COLS =   2048;
constexpr std::size_t OBJ_ROWS =   2048;
constexpr std::size_t OBJ_LEVELS = 2048;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RUNS = 1;

constexpr std::size_t RND_MAX = 100;

constexpr VALUE_TYPE EPSILON_STENCIL = 1e-15;

#include "c_nonlinear_stencil.hpp"
#include "c_state_function_mul2.hpp"
#include "c_state_function_pow2.hpp"
#include "c_state_function_exp.hpp"
#include "c_state_function_pow4_3.hpp"
#include "c_state_function_costly_0.hpp"
#include "c_state_function_costly_1.hpp"
#include "c_state_function_costly_2.hpp"

void printVec(const VEC_TYPE & p_vec)
{
    for (int i = 0; i < p_vec.size(); ++i)
    {
        std::cout << std::setprecision(3);
        std::cout << p_vec[i] << " ";
    }
    std::cout << std::endl;
}

void printArray(const double * p_array, const size_t p_size)
{
    for (size_t i = 0; i < p_size; ++i)
    {
        std::cout << std::setprecision(3);
        std::cout << p_array[i] << " ";
    }
    std::cout << std::endl;
}

double randDouble()
{
    return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/RND_MAX));
}


int main(int argc, char *argv[])
{
    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;
    LIKWID_MARKER_REGISTER("Mul2");
    LIKWID_MARKER_REGISTER("Pow2");
    LIKWID_MARKER_REGISTER("Exp");
    LIKWID_MARKER_REGISTER("Pow4_3");
    LIKWID_MARKER_REGISTER("Costly_0");
    LIKWID_MARKER_REGISTER("Costly_1");
    LIKWID_MARKER_REGISTER("Costly_2");

    srand (static_cast <unsigned> (time(0)));

    // double l_values[4] = {0.2f, -1.0f, 0.5f, 2.0f};
    double l_values[4] = {randDouble(), randDouble(), randDouble(), randDouble()};
    std::cout << "l_values:" << std::endl;
    printArray(l_values, 4);
    std::cout << std::endl;

    VEC_TYPE l_vecMul2;
    VEC_TYPE l_vecPow2;
    VEC_TYPE l_vecExp;
    VEC_TYPE l_vecPow4_3;
    VEC_TYPE l_vecCostly_0;
    VEC_TYPE l_vecCostly_1;
    VEC_TYPE l_vecCostly_2;

    l_vecMul2.load(l_values);
    l_vecPow2.load(l_values);
    l_vecExp.load(l_values);
    l_vecPow4_3.load(l_values);
    l_vecCostly_0.load(l_values);
    l_vecCostly_1.load(l_values);
    l_vecCostly_2.load(l_values);

    LIKWID_MARKER_START("Mul2");
    CStateFunctionMul2<VEC_TYPE>::apply(l_vecMul2);
    LIKWID_MARKER_STOP("Mul2");
    std::cout << "Mul2:" << std::endl;
    printVec(l_vecMul2);
    std::cout << std::endl;

    LIKWID_MARKER_START("Pow2");
    CStateFunctionPow2<VEC_TYPE>::apply(l_vecPow2);
    LIKWID_MARKER_STOP("Pow2");
    std::cout << "Pow2:" << std::endl;
    printVec(l_vecPow2);
    std::cout << std::endl;

    LIKWID_MARKER_START("Exp");
    CStateFunctionExp<VEC_TYPE>::apply(l_vecExp);
    LIKWID_MARKER_STOP("Exp");
    std::cout << "Exp:" << std::endl;
    printVec(l_vecExp);
    std::cout << std::endl;

    LIKWID_MARKER_START("Pow4_3");
    CStateFunctionPow4_3<VEC_TYPE>::apply(l_vecPow4_3);
    LIKWID_MARKER_STOP("Pow4_3");
    std::cout << "Pow4_3:" << std::endl;
    printVec(l_vecPow4_3);
    std::cout << std::endl;

    LIKWID_MARKER_START("Costly_0");
    CStateFunctionCostly0<VEC_TYPE>::apply(l_vecCostly_0);
    LIKWID_MARKER_STOP("Costly_0");
    std::cout << "Costly_0:" << std::endl;
    printVec(l_vecCostly_0);
    std::cout << std::endl;

    LIKWID_MARKER_START("Costly_1");
    CStateFunctionCostly1<VEC_TYPE>::apply(l_vecCostly_1);
    LIKWID_MARKER_STOP("Costly_1");
    std::cout << "Costly_1:" << std::endl;
    printVec(l_vecCostly_1);
    std::cout << std::endl;

    LIKWID_MARKER_START("Costly_2");
    CStateFunctionCostly2<VEC_TYPE>::apply(l_vecCostly_2);
    LIKWID_MARKER_STOP("Costly_2");
    std::cout << "Costly_2:" << std::endl;
    printVec(l_vecCostly_2);
    std::cout << std::endl;

    LIKWID_MARKER_CLOSE;
    return 0;
}
